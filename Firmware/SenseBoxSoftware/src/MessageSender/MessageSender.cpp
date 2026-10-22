#include "MessageSender.h"
#include "../Timer/Timer.h"
#include "../Logger/Logger.h"
#include "../SenseBoxConfig/sensebox_config.h"

MessageSender::MessageSender():Task(1000){ }

void MessageSender::initialize(){
    time_sending_started = timer->get_current_time_ms();
    time_next_measurement_frame = time_sending_started + lora_sending_interval_sec * 1000;
    time_next_status_frame = time_sending_started + lora_status_interval_sec * 1000;
}

void MessageSender::append_task(Task* t){
    if (number_tasks_to_send < NUMBER_TASKS){
        tasks_to_send[number_tasks_to_send] = t;
        number_tasks_to_send += 1;
    }
}

void MessageSender::computeLoRaCredentials(){
    volatile uint32_t val1, val2, val3, val4;
    volatile uint32_t *ptr1 = (volatile uint32_t *)0x0080A00C; val1 = *ptr1;
    volatile uint32_t *ptr = (volatile uint32_t *)0x0080A040; val2 = *ptr;
    ptr++; val3 = *ptr;
    ptr++; val4 = *ptr;

    DEVEUI[0]= ((val1>>20)); DEVEUI[1]= ((val2>>14)); DEVEUI[2]= ((val1>>6)); DEVEUI[3]= ((val2)); 
    DEVEUI[4]= ((val3>>20)); DEVEUI[5]= ((val4>>14)); DEVEUI[6]= ((val3>>6)); DEVEUI[7]= ((val4)); 
    
    APPKEY[0]= ((val1>>24)); APPKEY[1]= ((val2>>16)); APPKEY[2]= ((val1>>8)); APPKEY[3]= ((val2));
    APPKEY[4]= ((val2>>24)); APPKEY[5]= ((val1>>16)); APPKEY[6]= ((val2>>8)); APPKEY[7]= ((val1));
    APPKEY[8]= ((val4>>24)); APPKEY[9]= ((val3>>16)); APPKEY[10]= ((val4>>8)); APPKEY[11]= ((val3));
    APPKEY[12]= ((val3>>24)); APPKEY[13]= ((val4>>16)); APPKEY[14]= ((val3>>8)); APPKEY[15]= ((val4));

    uint64_t time = timer->get_current_time_ms();
    char buf[41];
    DEBUG_HIGH_PRIO(time); DEBUG_HIGH_PRIO(": Dev-EUI of this box: ");
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", DEVEUI[7], DEVEUI[6], DEVEUI[5], DEVEUI[4], DEVEUI[3], DEVEUI[2], DEVEUI[1], DEVEUI[0]);
    DEBUGLN_HIGH_PRIO(buf);
    
    DEBUG_HIGH_PRIO(time); DEBUG_HIGH_PRIO(": App-Key of this box: ");
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", APPKEY[0], APPKEY[1], APPKEY[2], APPKEY[3], APPKEY[4], APPKEY[5], APPKEY[6], APPKEY[7],APPKEY[8], APPKEY[9], APPKEY[10], APPKEY[11], APPKEY[12], APPKEY[13], APPKEY[14], APPKEY[15]);
    DEBUGLN_HIGH_PRIO(buf);
}

void MessageSender::run(){
    uint64_t current_time = timer->get_current_time_ms();

    if (status == STATUS_DISCONNECTED){
        connect();        
    }
    else if (status == STATUS_CONNECTING && time_sending_started + lora_max_time_joining_sec * 1000 < current_time){
        notify_join_failed();
        connect();
    }
    else if (status == STATUS_CONNECTED) {        
        if (current_time > time_next_measurement_frame){    // Time to send a new measurement message
            LoraMessage msg;
            prepare_measurement_message(&msg);
            if (send(&msg, LORA_FPORT_MEASUREMENT) == 0){
                time_next_measurement_frame = current_time + (uint64_t)lora_sending_interval_sec * 1000ull;
            }
        }
        else if (current_time > time_next_status_frame){
            LoraMessage msg;
            prepare_status_message(&msg);
            if (send(&msg, LORA_FPORT_STATUS) == 0){        // Time to send a new status message
                time_next_status_frame = current_time + (uint64_t)LORA_STATUS_SENDING_INTERVAL_S * 1000ull;
                logger->reset_lora_joining_errors();
            }
        }
    }   
}

void MessageSender::connect(){    
    LMIC_reset();

    LoraMessage m;
    prepare_status_message(&m);
    send(&m, LORA_FPORT_STATUS);    
}
   
int MessageSender::check_lora_status(){
    return status;    
}

void MessageSender::prepare_status_message(LoraMessage *m){
    m->addUint16(FIRMWARE_VERSION);
    m->addUint16(lora_sending_interval_sec);
    m->addUint16(0);    // Sensor-Messintervall - auf 0 gesetzt, weil verschiedene Sensoren verschiedene Intervalle haben
    m->addUint16(lora_max_time_joining_sec);
    m->addUint16(lora_max_time_tx_sec);
    m->addUint16(0);    // UV-Limit für das Anstellen des Fans => momentan nicht aktiv
    m->addUint16(FIRMWARE_CONFIGURATION);
    m->addUint16(0);    // Fan während des Sendens ausstellen => aktuell immer false

    logger->add_to_lora_status_message(m);      // collect all the information which is contributed by the logger class
}

void MessageSender::prepare_measurement_message(LoraMessage *m){
    m->addUint16(FRAME_FORMAT_VERSION);

    // Sensormessdaten aller Sensoren in der Reihenfolge von number_tasks_to_send hinzufügen
    for (int i=0; i<number_tasks_to_send; i++){
        tasks_to_send[i]->add_to_lora_message(m);
    }

    // 4x uint16 mit Wert 0 für Lattitude + Longitude
    for (int i=0; i<4; i++)
        m->addUint16(0);

    // Flags:
    m->addUint8(0);
}

int MessageSender::send(LoraMessage *m, uint8_t fport){        // returns the error code of the enqueuement: 0 means success.
    int result = 0;
    if (LMIC.opmode & OP_TXRXPEND) {    // Check whether an old message is still pending => we cannot queue a new one yet.              
        DEBUG(timer->get_current_time_ms());DEBUGLN(F(": Senden einer Nachricht abgebrochen: Es wird schon gesendet (OP_TXRXPEND-Flag gesetzt)."));
        result = -1;
    } 
    else {        
        result = LMIC_setTxData2(fport, m->getBytes(), m->getLength(), 0);       // LMIC_setTxData2 kopiert die Daten aus dem Puffer von m in den LoRa-Sendepuffer => das LoraMessage-Objekt darf (und sollte, sonst Memory Leak!) gelöscht werden (z.B. automatisch durch Definition als lokale Variable in der aufrufenden Funktion)        
    }
    
    if (result == 0){       // Success
        if (status==STATUS_CONNECTED) status = STATUS_SENDING;
        else if (status==STATUS_DISCONNECTED) status = STATUS_CONNECTING;
        this->time_sending_started = timer->get_current_time_ms();
        DEBUG_HIGH_PRIO(timer->get_current_time_ms());DEBUG_HIGH_PRIO(F(": LoRaWAN message queued for sending, fport = ")); DEBUG_HIGH_PRIO(fport); DEBUGLN_HIGH_PRIO("...");
    }
    else {      // Error queueing the message
        DEBUG_HIGH_PRIO(timer->get_current_time_ms()); DEBUG_HIGH_PRIO(": Error queueing message: "); DEBUGLN_HIGH_PRIO(result);
    }
    return result;
}


void MessageSender::notify_joined(){
    // Disable link check validation (automatically enabled during join, but not supported by TTN at this time).
    LMIC_setLinkCheckMode(0);       // from standard sensebox code
    DEBUG_HIGH_PRIO(timer->get_current_time_ms()); DEBUGLN_HIGH_PRIO(": LoRaWAN successfully joined.");
    status = STATUS_CONNECTED;
    join_number_trys = 0;
}


void MessageSender::notify_sent(){
    DEBUG_HIGH_PRIO(timer->get_current_time_ms()); DEBUGLN_HIGH_PRIO(": ... sent!");
    if (status==STATUS_SENDING || status == STATUS_CONNECTING){
        status = STATUS_CONNECTED;
        join_number_trys = 0;
    }
}

void MessageSender::notify_join_failed(){
    DEBUG_HIGH_PRIO(timer->get_current_time_ms()); DEBUGLN_HIGH_PRIO(": LoRaWAN join failed!");
    status = STATUS_DISCONNECTED;  
    join_number_trys += 1;
    logger->report_lora_joining_error();
    #ifndef DEBUG_DISABLE_LORA_RESET_REASON
        if (join_number_trys >= LORA_NUMBER_JOIN_TRYS_UNTIL_RESET) watchdog->request_reset(WD_RESET_REASON_LORA_CANNOT_SEND);
    #endif
}

void MessageSender::set_watchdog(WatchDog* wd){
    this->watchdog = wd;
}
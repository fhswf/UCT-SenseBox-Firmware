#include "Logger.h"
#include "../../global_defines.h"

/* Define Flash Storage Areas: */
FlashStorage(flash_page_1, FlashData_t);
FlashStorage(flash_page_2, FlashData_t);
FlashStorage(flash_page_3, FlashData_t);
FlashStorage(flash_page_4, FlashData_t);
FlashStorage(flash_page_5, FlashData_t);
FlashStorage(flash_page_6, FlashData_t);
FlashStorage(flash_page_7, FlashData_t);
FlashStorage(flash_page_8, FlashData_t);
FlashStorage(flash_page_9, FlashData_t);
FlashStorage(flash_page_10, FlashData_t);


FlashStorageClass<FlashData_t>* Logger::storage[LOGGER_NUMBER_PAGES] = {&flash_page_1, &flash_page_2, &flash_page_3, &flash_page_4, &flash_page_5, 
                                                                        &flash_page_6, &flash_page_7, &flash_page_8, &flash_page_9, &flash_page_10};

Logger* Logger::the_logger = NULL;

Logger* Logger::get_logger(){
    return the_logger;
}

Logger::Logger():Task(LOGGER_TASK_CALL_INTERVAL_MS){
    if (Logger::the_logger == NULL){    // If this is the singleton instance of Logger, store the reference to it        
        Logger::the_logger = this;
        logger = this;              // also, store the "this" reference in the logger pointer inherited by the base class Task - elsewise, this would be a NULL pointer!
    }
}

void Logger::initialize(){
    // retrieve the singleton timer object:
    timer = Timer::get_timer();     // retrieve a pointer to the singleton timer object    

    // If this ist the singleton instance of the logger, read back the RAM Mirror from flash:
    if (this == Logger::the_logger){
        FlashData_t intermediate_buffer;
        ram_buffer.time_of_storage = 0;
        for (uint8_t i=0; i<LOGGER_NUMBER_PAGES; i++){
            storage[i]->read(&intermediate_buffer);
            if (intermediate_buffer.time_of_storage > ram_buffer.time_of_storage){
                memcpy(&ram_buffer, &intermediate_buffer, sizeof(intermediate_buffer));
                this->last_page_written = i;    // Store the information that this is the latest page written, so that the page write cycle begins at this page (instead of always 1)
            }                                   //  => elsewise, after a reset the device will always with page 1, potentially causing a lot of write cycles to be consumed on page 1!
        }

        ResetHistoryItem_t *reset_info = get_last_reset_history_item();
        DEBUG_HIGH_PRIO(timer->get_current_time_ms());
        DEBUG_HIGH_PRIO(": Initializing Logger. Last entry has log ID ");
        DEBUG_HIGH_PRIO(ram_buffer.log_id);
        DEBUG_HIGH_PRIO(" and time stamp ");
        DEBUGLN_HIGH_PRIO(ram_buffer.time_of_storage);
        DEBUG_HIGH_PRIO(timer->get_current_time_ms());
        DEBUG_HIGH_PRIO(": Last logged reset was at time stamp ");
        DEBUG_HIGH_PRIO(reset_info->time_of_reset);        
        DEBUG_HIGH_PRIO("; reset reason was ");
        DEBUGLN_HIGH_PRIO(reset_info->reason);

        DEBUG_HIGH_PRIO(timer->get_current_time_ms());
        DEBUG_HIGH_PRIO(": Initializing system time from logged data to ");
        DEBUGLN_HIGH_PRIO(ram_buffer.time_of_storage);

        timer->initialize_with(ram_buffer.time_of_storage);
    }

    // Configure logger task to be executed in LOGGER_WRITE_INTERVAL_MS milliseconds
    time_next_buffer_write_ms = timer->get_current_time_ms() + LOGGER_WRITE_INTERVAL_MS;
}


void Logger::run(){
    uint64_t time = timer->get_current_time_ms();
    if (time > time_next_buffer_write_ms){
        write_buffer_into_flash();
        time_next_buffer_write_ms = time + LOGGER_WRITE_INTERVAL_MS;
    }

}

void Logger::write_buffer_into_flash(){
    last_page_written = (last_page_written + 1) % LOGGER_NUMBER_PAGES;    // Identify next flash page to write
    write_buffer_into_flash_page(last_page_written);
}


void Logger::write_buffer_into_flash_page(int page_no){
    ram_buffer.log_id += 1;
    ram_buffer.time_of_storage = timer->get_current_time_ms();
    storage[page_no]->write(ram_buffer);
    DEBUG_HIGH_PRIO(ram_buffer.time_of_storage);
    DEBUG_HIGH_PRIO(": Written logger data with id ");
    DEBUG_HIGH_PRIO(ram_buffer.log_id);
    DEBUG_HIGH_PRIO(" to flash page ");
    DEBUGLN_HIGH_PRIO(page_no);
}

void Logger::read_flash_into_buffer(){    
    read_flash_page_into_buffer(last_page_written);
}


void Logger::read_flash_page_into_buffer(int page_no){
    storage[page_no]->read(&ram_buffer);
}

uint16_t Logger::get_index_of_last_reset(){
    uint64_t newest_time = 0;   // min value for 64 Bit uint
    uint16_t index_newest = 0;
    for (uint16_t i=0; i<LOGGER_NO_RESET_HISTORY_ITEMS; i++)
        if (ram_buffer.reset_history[i].time_of_reset >= newest_time){       // >= means: when some resets have the same time (especially: 0), choose the last such event
            newest_time = ram_buffer.reset_history[i].time_of_reset;
            index_newest = i;
        }
    return index_newest;
}

uint16_t Logger::get_index_of_oldest_reset(){
    uint64_t oldest_time = 0xFFFFFFFFFFFFFFFFull;       // Max value for 64 Bit int
    uint16_t index_oldest = 0;
    for (uint16_t i=0; i<LOGGER_NO_RESET_HISTORY_ITEMS; i++)
        if (ram_buffer.reset_history[i].time_of_reset < oldest_time){       // < means: when some resets have the same time (especially: 0), choose the first such event
            oldest_time = ram_buffer.reset_history[i].time_of_reset;
            index_oldest = i;
        }
    return index_oldest;
}

ResetHistoryItem_t* Logger::get_last_reset_history_item(){
    uint16_t index_newest = get_index_of_last_reset();
    return &ram_buffer.reset_history[index_newest];
}

ResetHistoryItem_t* Logger::get_first_reset_history_item(){
    uint16_t index_oldest = get_index_of_oldest_reset();
    return &ram_buffer.reset_history[index_oldest];
}

void Logger::add_reset(uint8_t reason, uint64_t time){
    // identify oldest reset in buffer; if some are same (for example 0), choose the first such index:
    uint16_t index_oldest = get_index_of_oldest_reset();

    // overwrite it:
    ram_buffer.reset_history[index_oldest].time_of_reset = time;
    ram_buffer.reset_history[index_oldest].reason = reason;
}

void Logger::report_lora_joining_error(){
    ram_buffer.number_lora_joinig_errors++;
}

void Logger::reset_lora_joining_errors(){
    ram_buffer.number_lora_joinig_errors = 0;
}

FlashData_t* Logger::get_ram_buffer(){
    return &ram_buffer;
}

void Logger::add_to_lora_status_message(LoraMessage *msg){    
    msg->addUint8(ram_buffer.log_id % 256);                 // Niederwertiges Byte
    msg->addUint16((uint16_t)(ram_buffer.log_id >> 8));     // Obere zwei Bytes in Big Endian

    uint32_t current_time_h = (uint32_t)(timer->get_current_time_ms()/3600000ull);
    msg->addUint8(current_time_h % 256);                 // Niederwertiges Byte
    msg->addUint16((uint16_t)(current_time_h >> 8));     // Obere zwei Bytes in Big Endian
    
    // Reset History: Order the reset history items ascending by time of occurence, starting with the first one:
    int16_t last_entry = get_index_of_last_reset();
    for (int16_t i=0; i<5; i++){         // only append the 5 newest results
        int16_t number_entry = (last_entry - i) % LOGGER_NO_RESET_HISTORY_ITEMS;  // The id of the i-th history entry to store
        while (number_entry<0) number_entry += LOGGER_NO_RESET_HISTORY_ITEMS;     // C++ modulo result can be negative => Add LOGGER_NO_RESET_HISTORY_ITEMS until the result becomes positive
        DEBUG(timer->get_current_time_ms()); DEBUG(": Writing log to msg, entry "); DEBUG(i+1); DEBUG(" has log index "); DEBUGLN(number_entry);
        
        msg->addUint8(ram_buffer.reset_history[number_entry].reason);        
        uint32_t reset_time_h = (uint32_t)(ram_buffer.reset_history[number_entry].time_of_reset/3600000ull);
        msg->addUint8(reset_time_h % 256);                // Niederwertiges Byte
        msg->addUint16((uint16_t)(reset_time_h >> 8));    // Obere zwei Bytes in Big Endian
    }

    msg->addUint16(ram_buffer.number_lora_joinig_errors);
}
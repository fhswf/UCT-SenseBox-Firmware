#include "WatchDog.h"
#include <lmic.h>


void WatchDog::setup(int tolerance_time_ms, Timer* timer){
    this->timer = timer;
    uint64_t time = timer->get_current_time_ms();
    uint8_t reset_cause = hw_watchdog->resetCause();

    DEBUG_HIGH_PRIO(time);
    DEBUG_HIGH_PRIO(": Initializing Watchdog... last hw watchdog reset cause was ");
    DEBUGLN_HIGH_PRIO(reset_cause);

    this->logger = Logger::get_logger();    
    logger->add_reset(reset_cause, time);       // Log the reset together with its cause ...

    // ... and immediately write it to flash, but only, if we are not in rapidly repeating resets (i.e., oldest reset in log is older than an hour)!
    ResetHistoryItem_t *oldest_reset = logger->get_first_reset_history_item();
    if (oldest_reset->time_of_reset - time > 3600000ull || oldest_reset->time_of_reset==0)
        logger->write_buffer_into_flash();      
    
    #ifndef WD_RESET_DISABLED
    this->tolerance_time_ms = hw_watchdog->enable(tolerance_time_ms);       // retrieve the actual HW WD reset time that has been programmed to the hardware
    DEBUG("WatchDog initialized with tolerance time (in ms) ");
    DEBUGLN(this->tolerance_time_ms);
    #endif
}

void WatchDog::set_supervised_tasks(Task *((*tasks)[NUMBER_TASKS])){
    this->supervised_tasks = tasks;
}

void WatchDog::request_reset(int reset_reason){
    this->reset_requested = true;
    DEBUG_HIGH_PRIO(timer->get_current_time_ms());
    DEBUG_HIGH_PRIO(": Watchdog reset requested, reset reason id is: ");
    DEBUGLN_HIGH_PRIO(reset_reason);
}

void WatchDog::notify_milestone(int milestone_id){
    uint64_t current_time =  timer->get_current_time_ms();        

    // Check that all tasks have been executed during the last WD_TOLERANCE_TIME_MS milliseconds
    for (int i=0; i<NUMBER_TASKS; i++){
        Task* t = (*(this->supervised_tasks))[i];
        if (t->get_time_last_run() + t->get_interval() + WD_TOLERANCE_TIME_MS < current_time){        // Task is overdue for at least WD_TOLERANCE_TIME_MS ms!
            this->request_reset(WD_RESET_REASON_TASK_BASE + t->get_id());
            DEBUG_HIGH_PRIO("Current time is ");
            DEBUG_HIGH_PRIO(current_time);
            DEBUG_HIGH_PRIO("; last task execution was ");
            DEBUGLN_HIGH_PRIO(t->get_time_last_run());
        }
    }

    // Check whether the WD is notified at the same current time as before; if so, count the number of notifications at this point of time
    if (current_time == this->last_time_notified){
        this->number_notifications_while_timer_not_changed += 1;
        if (this->number_notifications_while_timer_not_changed > WD_MAX_NOTIFICATIONS_AT_SAME_TIMER_VALUE)
            this->request_reset(WD_RESET_REASON_TIMER);
    }
    else {
        this->last_time_notified = current_time;
        this->number_notifications_while_timer_not_changed = 0;
    }
    
    // Reset the WD timer, if no system reset has been requested:
    if (!this->reset_requested){                
        last_milestone = milestone_id;
        
        hw_watchdog->reset();                   // Trigger HW Watchdog in order to reset its timer
    }
}
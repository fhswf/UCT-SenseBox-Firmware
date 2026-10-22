#include "task.h"
#include "../../global_defines.h"
#include "../Logger/Logger.h"

int Task::id_counter = 1;   // First Task ID

Task::Task(uint64_t interval){
    this->timer = Timer::get_timer();
    this->logger = Logger::get_logger();
    this->interval_ms = interval;
    this->time_next_run_ms = timer->get_current_time_ms();
    this->id = Task::id_counter++;  // get next ID and give it to the task
}

int Task::get_id(){return this->id;}

uint64_t Task::get_time_last_run(){return this->time_last_run_ms;}

void Task::set_timer(Timer* timer){
    this->timer = timer;
}

void Task::check_and_perform_next_run(){
    /* Checks whether the run() method needs to be called since the 
       time for the next run of the task has been reached, performs 
       this call and updates the time for the next run */
    uint64_t time = timer->get_current_time_ms();
    if (time >= time_next_run_ms){        
        this->time_last_run_ms = time;
        this->run();

        if (time-time_next_run_ms < 2*interval_ms/2){    // normally, we want to simply add the interval to the last time to run so that the calls are not shifted over time ...
            time_next_run_ms += interval_ms;            
        }
        else
            time_next_run_ms = time + interval_ms;      // ...  but if the time for the next run has been delayed by more than the interval, we accept the shift, so that it cannot be rapidly executed.
    }
}

void Task::run(){
    // virtual, please override in derived class
    DEBUG("Warning: Executing empty task at time ");
    DEBUGLN(timer->get_current_time_ms());
}

void Task::add_to_lora_message(LoraMessage *msg){
    // virtual, please override in derived class
}

uint64_t Task::get_interval(){return interval_ms;}
uint64_t Task::get_time_text_run(){return time_next_run_ms;}

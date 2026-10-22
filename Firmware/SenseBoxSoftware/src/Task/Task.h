/*
Class Task

Base Class for every task
Derived tasks specify an interval and overwrite the run() method, which will 
be called in exactly this interval if check_and_perform_next_run() is called in 
loop().

*/

#pragma once

#include "../Timer/Timer.h"
#include <Arduino.h>
#include <lmic.h>
#include <LoraMessage.h>

class Logger;                           // Forward declaration of logger, to avoid cyclical inclusion

class Task
{
    static int id_counter;
    int id;
    uint64_t interval_ms;
    uint64_t time_next_run_ms;
    uint64_t time_last_run_ms;          // For watchdog supervision    
protected:
    Timer *timer = NULL;
    Logger *logger = NULL;
public:
    Task(uint64_t interval_ms);
    uint64_t get_interval();
    uint64_t get_time_text_run();
    void check_and_perform_next_run();
    virtual void run();
    virtual void add_to_lora_message(LoraMessage *msg);
    void set_timer(Timer *timer);
    uint64_t get_time_last_run();
    int get_id();
};
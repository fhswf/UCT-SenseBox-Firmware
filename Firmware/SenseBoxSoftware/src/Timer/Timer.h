/*
Timer class

Wrapper for the lmic time functions, since these
work with 32 bit ints, which overflow after a couple
of hours.
*/

#pragma once

#include <Arduino.h>

class Timer
{    
    uint64_t current_time_ms;       // the current time stamp as can be read out by (uint64_t)osticks2ms(os_getTime())
    uint64_t overflow_time_ms;      // the "base" time to add to current_time_ms to get the actual system time, which is not reset to 0 every ~9 hours. 
                                    //   - will increase by the former current_time_ms whenever the os system time resets to 0
    static Timer *the_timer;

    #if DEBUG_LEVEL >= 1
        uint64_t next_time_debug_entry = 0;
    #endif

public:
    Timer();    
    uint64_t get_current_time_ms();
    static Timer* get_timer();

private:
    void initialize_with(uint64_t initial_time);

    friend class Logger;        // only Logger is a friend class of timer and can initialize the timer by calling initialize_with
};
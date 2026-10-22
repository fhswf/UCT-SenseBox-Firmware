#include "../../global_defines.h"
#include "timer.h"
#include <lmic.h>

Timer* Timer::the_timer = NULL;     // Singleton: There is only one timer object (respectively, this is a pointer to the first one created)

Timer::Timer(){
    if (Timer::the_timer == NULL) Timer::the_timer = this;
    this->current_time_ms = 0ULL;
    this->overflow_time_ms = 0ULL;
    #if DEBUG_LEVEL >= 1
        next_time_debug_entry = 1800000ull;
    #endif
}

void Timer::initialize_with(uint64_t initial_time){    
    // set the overflow time in an way that the total time (current + overflow) equals the initial_time passed to this function.
    this->get_current_time_ms();        // Update this->current_time_ms
    this->overflow_time_ms = max((signed)initial_time - (signed)current_time_ms, 0ll);

    DEBUG_HIGH_PRIO(this->current_time_ms+this->overflow_time_ms);
    DEBUG_HIGH_PRIO(": Current time value is "); DEBUG_HIGH_PRIO(this->current_time_ms);
    DEBUG_HIGH_PRIO("; accumulated overflow time is "); DEBUGLN_HIGH_PRIO(this->overflow_time_ms);

    #if DEBUG_LEVEL >= 1
        next_time_debug_entry = initial_time + 1800000ull;
    #endif
}

uint64_t Timer::get_current_time_ms(){
    // get current system time:        
    uint64_t time = ((uint64_t)hal_ticks() * 1000ull) / (uint64_t)OSTICKS_PER_SEC;

    if (time < this->current_time_ms){          // new time is less than the last time determined! => there has been an overflow
        DEBUG_HIGH_PRIO(this->current_time_ms+this->overflow_time_ms);
        DEBUG_HIGH_PRIO(": Timer overflow in os_getTime()! Current OS time is "); DEBUG_HIGH_PRIO(time);
        DEBUG_HIGH_PRIO("; last time was "); DEBUG_HIGH_PRIO(this->current_time_ms);
        DEBUG_HIGH_PRIO("; accumulated overflow time is ");DEBUGLN_HIGH_PRIO(this->overflow_time_ms);
        this->overflow_time_ms += this->current_time_ms + 1ULL;    // the new overflow time is the last known time + (at least) one ms for reaching (as an overflow) 0 again
    }
    this->current_time_ms = time;               // Store new sys time only after updating the overflow time.

    #if DEBUG_LEVEL >= 1
    uint64_t now = this->current_time_ms + this->overflow_time_ms;
    if (now >= next_time_debug_entry){
        DEBUG_HIGH_PRIO(now);
        DEBUGLN_HIGH_PRIO(": Alive!");
        next_time_debug_entry += 1800000ull;
    }
    #endif

    return this->current_time_ms + this->overflow_time_ms;
}


Timer* Timer::get_timer(){
    return Timer::the_timer;
}
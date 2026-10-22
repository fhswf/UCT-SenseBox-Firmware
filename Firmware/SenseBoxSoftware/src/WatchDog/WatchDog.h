#pragma once

#include <Adafruit_SleepyDog.h>
#include "../Timer/Timer.h"
#include "../Logger/Logger.h"
#include "../Task/Task.h"
#include "../../global_defines.h"

class WatchDog
{
    bool reset_requested = false;
    int tolerance_time_ms = 0;
    Task *((*supervised_tasks)[NUMBER_TASKS]) = NULL;
    uint64_t last_time_notified = 0;
    int number_notifications_while_timer_not_changed = 0;   // Used to detect timer HW lockup: If more than WD_MAX_NOTIFICATIONS_AT_SAME_TIMER_VALUE notifications 
                                                            //  - happen at the same time, the software conjectures that the timer has locked up and will cause a reset to heal this.
    int last_milestone = 0;

    WatchdogType *hw_watchdog = &Watchdog;    // Pointer to the HW Watchdog object from the Adafruit SleepyDog class
    Timer *timer = NULL;
    Logger *logger = NULL;

public:
    void setup(int tolerance_time_ms, Timer* timer);
    void request_reset(int reset_reason);
    void notify_milestone(int milestone_id);
    void set_supervised_tasks(Task *((*tasks)[NUMBER_TASKS]));
};
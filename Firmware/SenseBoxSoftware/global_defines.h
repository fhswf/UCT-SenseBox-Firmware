#pragma once

#define DEBUG_LEVEL 1           // 0 = nothing, 1 = only high prio notifications, 2 = all notifications
#define DEBUG_DISABLE_LORA_RESET_REASON   // Disables LoRa based reset for debug purposes

#define FIRMWARE_VERSION 505
#define FIRMWARE_CONFIGURATION 0
#define LORA_FPORT_MEASUREMENT 20
#define LORA_FPORT_STATUS 22
#define LORA_SENDING_INTERVAL_S 600             // every 10 minutes
#define LORA_STATUS_SENDING_INTERVAL_S 86400    // once a day
#define LORA_JOIN_TIMEOUT_TIME_S 180            // a bit less than 3 minutes, since in this way a second and third try in a 10 min sending interval is possible
#define LORA_SENDING_TIMEOUT_TIME_S 180         // a bit less than 3 minutes
#define LORA_NUMBER_JOIN_TRYS_UNTIL_RESET 50

#define NUMBER_TASKS 8

#define WD_TOLERANCE_TIME_MS 10000ULL                   // Time after which the WD triggers a reset when a task is overdue
#define WD_MAX_NOTIFICATIONS_AT_SAME_TIMER_VALUE 25000  // Number of calls of the WD's notify function at the same timestamp (in ms), before a reset is triggered

// Watchdog Milestone IDs
#define WD_MILESTONE_NONE 0
#define WD_MILESTONE_ENTRY_LOOP 10
#define WD_MILESTONE_AFTER_OS_RUNLOOP 20
#define WD_MILESTONE_AFTER_TASK_BASE 30           // Base number for task milestones (reached after finishing the task), the task with id i has milestone ID (WD_MILESTONE_TASK_BASE + i)
#define WD_MILESTONE_EXIT_LOOP 50

// Watchdog Reset Reasons
#define WD_RESET_REASON_NONE 0
#define WD_RESET_REASON_LORA_CANNOT_SEND 10
#define WD_RESET_REASON_TIMER 20
#define WD_RESET_REASON_TASK_BASE 100

#if DEBUG_LEVEL >= 1
    #define MS_PER_SEC 1000         // Can be used to speed up measurement intervals für debugging by setting lower numbers
    #define DEBUG_HIGH_PRIO(str) Serial.print(str)
    #define DEBUGLN_HIGH_PRIO(str) Serial.println(str)
    #if DEBUG_LEVEL >= 2
        #define DEBUG(str) Serial.print(str)
        #define DEBUGLN(str) Serial.println(str)
        #define WD_RESET_DISABLED       // Define this to disable Watchdog resets for debug reasons
    #else
        #define DEBUG(str) 
        #define DEBUGLN(str) 
    #endif
#else
    #define MS_PER_SEC 1000   // 1 sec is 1 sec for measurement rhythm control, standard
    #define DEBUG(str)
    #define DEBUGLN(str)
    #define DEBUG_HIGH_PRIO(str)
    #define DEBUGLN_HIGH_PRIO(str)
#endif
/*
sensebox_config.h

Stores senseBox Settings.
*/


#pragma once

#include <lmic.h>
#include <hal/hal.h>

extern uint8_t DEVEUI[8]; 
extern const uint8_t APPEUI[8];
extern uint8_t APPKEY[16];

extern const lmic_pinmap lmic_pins;

extern void os_getDevEui (u1_t* buf);
extern void os_getArtEui (u1_t* buf);
extern void os_getDevKey (u1_t* buf);
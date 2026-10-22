#include <Arduino.h>
#include "sensebox_config.h"

uint8_t DEVEUI[8] = {0,0,0,0,0,0,0,0};
const uint8_t APPEUI[8] = {0,0,0,0,0,0,0,0};
uint8_t APPKEY[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void os_getDevEui (u1_t* buf) {memcpy_P(buf, DEVEUI, 8);}
void os_getArtEui (u1_t* buf) {memcpy_P(buf, APPEUI, 8);}
void os_getDevKey (u1_t* buf) {memcpy_P(buf, APPKEY, 16);}

// Pin Mapping:

const lmic_pinmap lmic_pins = {
  .nss = PIN_XB1_CS,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LMIC_UNUSED_PIN,
  .dio = {PIN_XB1_INT, PIN_XB1_INT, LMIC_UNUSED_PIN},
};
#include <Arduino.h>
#include <EEPROM.h>

#include "control.h"
#include "loadcell.h"
#include "config.h"
#include "eeprom.h"

#define PRINT_INTERVAL  1000

void control_setup(void)
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, 0);
}

void control_set_relay(void)
{
    digitalWrite(RELAY_PIN, 1);
}

void control_reset_relay(void)
{
    digitalWrite(RELAY_PIN, 0);
}

void control_loop(void)
{
    static unsigned int t = millis(); 
    if (loadcell_get_weight() >= eeprom_setpoint_get()) {
         if (millis() > t + PRINT_INTERVAL) { 
            Serial.println("Weight setpoint exceeded.");
            t = millis();
        }
        control_set_relay();
    }
}


#include <Arduino.h>
#include <EEPROM.h>

#include "control.h"
#include "loadcell.h"
#include "config.h"

#define RELAY_PIN   0 // D3

#define WEIGHT_LIMIT_MIN    5.0f
#define WEIGHT_LIMIT_MAX    30.0f

#define PRINT_INTERVAL  1000

static float g_weight_setpoint = 18.0f;

void control_setup(void)
{
    float weight;
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, 0);
#if defined(ESP8266) || defined(ESP32)
    EEPROM.begin(EEP_SIZE);
#endif
    EEPROM.get(EEP_SETPOINT_ADDR, weight);
    if (isnan(weight) || weight > WEIGHT_LIMIT_MAX || weight < WEIGHT_LIMIT_MIN) {
        Serial.print("Invalid value for weight in EEPROM: ");
        Serial.print(weight);
        Serial.print(". Using default value.");
        Serial.println(g_weight_setpoint);
    } else {
        Serial.print("Restored weight setpoint from EEPROM: ");
        Serial.println(weight);
        g_weight_setpoint = weight;
    }
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
    if (loadcell_get_weight() >= g_weight_setpoint) {
         if (millis() > t + PRINT_INTERVAL) { 
            Serial.println("Weight setpoint exceeded.");
            t = millis();
        }
        control_set_relay();
    }
}

void control_set_setpoint(float weight)
{
    if (weight < WEIGHT_LIMIT_MIN || weight > WEIGHT_LIMIT_MAX) {
        Serial.print("Invalid weight: ");
        Serial.println(weight);
    } else {
        g_weight_setpoint = weight;
        Serial.print("New weight setpoint: ");
        Serial.println(weight);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.begin(EEP_SIZE);
#endif
        EEPROM.put(EEP_SETPOINT_ADDR, weight);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
    }
}

float control_get_setpoint(void)
{
   return g_weight_setpoint;
}

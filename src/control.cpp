#include <Arduino.h>
#include <EEPROM.h>

#include "control.h"
#include "loadcell.h"
#include "config.h"
#include "eeprom.h"

#define PRINT_INTERVAL  1000

static unsigned int g_timer_start = 0;
static unsigned int g_timer_stop = 0;

enum timer_state_e {
        WAITING,
        RUNNING,
        STOPPED
    };

static enum timer_state_e g_tstate = WAITING;

void control_setup(void)
{
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, 0);
}

bool control_get_relay(void)
{
    return (digitalRead(RELAY_PIN) == HIGH);
}

void control_set_relay(void)
{
    digitalWrite(RELAY_PIN, 1);
}

void control_reset_relay(void)
{
    digitalWrite(RELAY_PIN, 0);
}

unsigned int control_get_elapsed_time(void)
{
#ifdef DEBUG
    Serial.print("t_state = ");
    Serial.println(g_tstate);
    Serial.print("g_timer_start = ");
    Serial.println(g_timer_start);
    Serial.print("g_timer_stop = ");
    Serial.println(g_timer_stop);

#endif
    if (g_tstate == RUNNING)
        return millis() - g_timer_start;
    else if (g_tstate == STOPPED)
        return g_timer_stop - g_timer_start;
    else
        return 0;
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

    switch (g_tstate) {
    case WAITING:
        if (loadcell_get_weight() >= eeprom_timer_threshold_get()) {
           g_timer_start = millis();
           g_tstate = RUNNING;
        }
        break;

    case RUNNING:
        if (loadcell_get_weight() >= eeprom_setpoint_get()) {
            g_timer_stop = millis();
            g_tstate = STOPPED;
        }

    case STOPPED:
        if (loadcell_get_weight() <= eeprom_timer_threshold_get()) {
            g_tstate = WAITING;
        }
        break;        
    }
}


#include <Arduino.h>
#include <EEPROM.h>
#include <CRC32.h>

#include "config.h"

struct parameter_cache {
  float weight_setpoint;
  float calibration_factor;
  float timer_threshold;
};

static struct parameter_cache g_parameter_cache = { 0 };

static void eeprom_update_checksum(void)
{
    int i;
    uint32_t checksum;
    CRC32 crc;

    for (i = 0; i < (EEP_SIZE - EEP_CRC32_SIZE); ++i) {
        crc.update(EEPROM.read(i));
    }

    checksum = crc.finalize();
    noInterrupts();
    EEPROM.put(EEP_CRC32_ADDR, checksum);
#if defined(ESP8266) || defined(ESP32)
    EEPROM.commit();
#endif
    interrupts();
    Serial.printf("New CRC: %04X\n", checksum);
}

void eeprom_setpoint_set(float s)
{
    if (s < WEIGHT_LIMIT_MIN || s > WEIGHT_LIMIT_MAX || isnan(s)) {
        Serial.print("Invalid weight: ");
        Serial.println(s);
        return;
    }

    if (s != g_parameter_cache.weight_setpoint) {
        noInterrupts();
        EEPROM.put(EEP_SETPOINT_ADDR, s);
        g_parameter_cache.weight_setpoint = s;
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
        interrupts();
        eeprom_update_checksum();
    }
}

void eeprom_calfactor_set(float c)
{
  if (c != g_parameter_cache.calibration_factor) {
      noInterrupts();
      EEPROM.put(EEP_CALIBRATION_VALUE_ADDR, c);
      g_parameter_cache.calibration_factor = c;
#if defined(ESP8266) || defined(ESP32)
      EEPROM.commit();
#endif
      interrupts();
      eeprom_update_checksum();
  }
}

void eeprom_timer_threshold_set(float t)
{
  if (t != g_parameter_cache.timer_threshold) {
      noInterrupts();
      EEPROM.put(EEP_TIMER_THRESHOLD_ADDR, t);
      g_parameter_cache.timer_threshold = t;
#if defined(ESP8266) || defined(ESP32)
      EEPROM.commit();
#endif
      interrupts();
      eeprom_update_checksum();
  }
}

float eeprom_timer_threshold_get(void)
{
    return g_parameter_cache.timer_threshold;
}

float eeprom_setpoint_get(void)
{
    return g_parameter_cache.weight_setpoint;
}

float eeprom_calfactor_get(void)
{
    return g_parameter_cache.calibration_factor;
}

static void reset_eeprom(void)
{
    Serial.println("EEPROM checksum invalid, restoring default values.");
    noInterrupts();
    EEPROM.put(EEP_SETPOINT_ADDR, DEFAULT_SETPOINT);
    EEPROM.put(EEP_CALIBRATION_VALUE_ADDR, DEFAULT_CALIBRATION_VALUE);
    EEPROM.put(EEP_TIMER_THRESHOLD_ADDR, DEFAULT_TIMER_THRESHOLD);
#if defined(ESP8266) || defined(ESP32)
    EEPROM.commit();
#endif
    interrupts();
    eeprom_update_checksum();
}

void eeprom_setup(void)
{
    int i;
    uint32_t checksum_calc, checksum_stored;
    float tmp;
    CRC32 crc;
#if defined(ESP8266) || defined(ESP32)
    EEPROM.begin(EEP_SIZE);
#endif

    for (i = 0; i < (EEP_SIZE - EEP_CRC32_SIZE); ++i) {
        crc.update(EEPROM.read(i));
    }

    checksum_calc = crc.finalize();
    EEPROM.get(EEP_CRC32_ADDR, checksum_stored);

    if (checksum_calc != checksum_stored) {
        reset_eeprom();
    }

    /* Update the parameter cache from EEPROM */
    EEPROM.get(EEP_CALIBRATION_VALUE_ADDR, tmp);
    if (isnan(tmp)) {
        noInterrupts();
        EEPROM.put(EEP_CALIBRATION_VALUE_ADDR, DEFAULT_CALIBRATION_VALUE);
        interrupts();
        tmp = DEFAULT_CALIBRATION_VALUE;
    } 
    g_parameter_cache.calibration_factor = tmp;

    EEPROM.get(EEP_SETPOINT_ADDR, tmp);
    if (isnan(tmp)) {
        noInterrupts();
        EEPROM.put(EEP_SETPOINT_ADDR, DEFAULT_SETPOINT);
        interrupts();
        tmp = DEFAULT_SETPOINT;
    } 
    g_parameter_cache.weight_setpoint = tmp;

    EEPROM.get(EEP_TIMER_THRESHOLD_ADDR, tmp);
    if (isnan(tmp)) {
        noInterrupts();
        EEPROM.put(EEP_TIMER_THRESHOLD_ADDR, DEFAULT_TIMER_THRESHOLD);
        interrupts();
        tmp = DEFAULT_TIMER_THRESHOLD;
    } 
    g_parameter_cache.timer_threshold = tmp;

    eeprom_update_checksum();

    Serial.println("EEPROM starting values:");
    Serial.print("Calibration factor: ");
    Serial.println(g_parameter_cache.calibration_factor);
    Serial.print("Weight setpoint: ");
    Serial.println(g_parameter_cache.weight_setpoint);
    Serial.print("Timer threshold weight: ");
    Serial.println(g_parameter_cache.timer_threshold);
}


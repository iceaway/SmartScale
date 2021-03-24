#include <Arduino.h>
#include <EEPROM.h>
#include <CRC32.h>

#include "config.h"

struct parameter_cache {
  float weight_setpoint;
  float calibration_factor;
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

float eeprom_setpoint_get(void)
{
    return g_parameter_cache.weight_setpoint;
}

float eeprom_calfactor_get(void)
{
    return g_parameter_cache.calibration_factor;
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
        Serial.println("EEPROM checksum invalid, restoring default values.");
        noInterrupts();
        EEPROM.put(EEP_SETPOINT_ADDR, DEFAULT_SETPOINT);
        EEPROM.put(EEP_CALIBRATION_VALUE_ADDR, DEFAULT_CALIBRATION_VALUE);
#if defined(ESP8266) || defined(ESP32)
        EEPROM.commit();
#endif
        interrupts();
        eeprom_update_checksum();
    }

    /* Update the parameter cache from EEPROM */
    EEPROM.get(EEP_CALIBRATION_VALUE_ADDR, tmp);
    g_parameter_cache.calibration_factor = tmp;
    EEPROM.get(EEP_SETPOINT_ADDR, tmp);
    g_parameter_cache.weight_setpoint = tmp;
}


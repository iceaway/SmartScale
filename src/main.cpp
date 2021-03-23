#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include <CRC32.h>
#include "webserver.h"
#include "loadcell.h"
#include "control.h"
#include "config.h"

/* 
 * TODO:
 * - Improve EEPROM handling
 * - SoftAP mode to configure WLAN
 * - Update target weight corrupts EEPROM?
 * - Calibrate from web page
 * - Make webpage look nicer
 */

static bool check_eeprom(void)
{
    int i;
    uint32_t checksum_calc, checksum_stored;
    CRC32 crc;
    EEPROM.begin(EEP_SIZE);

    for (i = 0; i < (EEP_SIZE - EEP_CRC32_SIZE); ++i) {
        crc.update(EEPROM.read(i));
    }

    checksum_calc = crc.finalize();
    EEPROM.get(EEP_CRC32_ADDR, checksum_stored);
    Serial.printf("Calculated CRC: %04X\n", checksum_calc);
    Serial.println(checksum_calc);
    Serial.printf("Stored CRC: %04X\n", checksum_stored);
    Serial.println(checksum_stored);

    return true;
}

void setup(void)
{
    Serial.begin(9600);
    delay(500);
    Serial.println();
    Serial.println("Starting...");
    /* Check EEPROM validity */
    check_eeprom();
    Serial.println("Setting up wifi and webserver...");
    webserver_setup();
    Serial.println("Setting up load cell...");
    loadcell_setup();
    Serial.println("Setting up Control loop..");
    control_setup();
    Serial.println("Setting up filesystem...");
    if (!LittleFS.begin()) {
        Serial.println("Failed to setup LittleFS!");
    }
    Serial.println("Setup complete!");
}

void loop(void)
{
    loadcell_loop();
    control_loop();
}

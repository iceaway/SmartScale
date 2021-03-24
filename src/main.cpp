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
#include "eeprom.h"

/* 
 * TODO:
 * - Calibrate from web page
 * - Make webpage look nicer
 * - mDNS / DNS server support
 */

void setup(void)
{
    Serial.begin(9600);
    delay(500);
    Serial.println();
    Serial.println("Starting...");
    /* Check EEPROM validity */
    Serial.println("Checking EEPROM...");
    eeprom_setup();
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

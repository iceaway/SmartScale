#include <FS.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <LittleFS.h>

#define WEBSERVER_H

#ifdef ESP32
    #include <WiFi.h>
    #include <AsyncTCP.h>
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
#endif
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>

#include "loadcell.h"
#include "control.h"
#include "eeprom.h"
#include "config.h"

static AsyncWebServer server(HTTP_PORT);

String processor(const String& var)
{
    if (var == "STARTWEIGHT") {
        String weight = String(eeprom_setpoint_get());
        return weight;
    }
    return String();
}

static void not_found(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "Not found");
}

static void get_root(AsyncWebServerRequest *request)
{
    request->send(LittleFS, "/index.html", "text/html", false, processor);
}

static void get_weight(AsyncWebServerRequest *request)
{
    String value;
    value = String(loadcell_get_weight());
    request->send(200, "text/plain", value);
}

static void tare(AsyncWebServerRequest *request)
{
    loadcell_tare();
    request->send(200, "text/plain", "Taring...");
}

static void tare_status(AsyncWebServerRequest *request)
{
    String message = loadcell_tare_status() ? "Done" : "Taring...";
    request->send(200, "text/plain", message);
}

static void reset_relay(AsyncWebServerRequest *request)
{
    control_reset_relay();
    request->send(200, "text/plain", "");
    Serial.println("Reset relay");
}

static void set_weight_setpoint(AsyncWebServerRequest *request)
{
    String message;
    if (request->hasParam("value")) {
        message = request->getParam("value")->value();
        eeprom_setpoint_set(message.toFloat());
    } else {
        message = "No message sent";
    }
    
    request->send(200, "text/plain", "");
}

void webserver_setup()
{
    WiFiManager wifi;

    wifi.setTimeout(180);
    if (!wifi.autoConnect("SmartScale", "SmartScale!")) {
      Serial.println("Failed to connect");
      delay(3000);
      ESP.reset();
      delay(5000);
    }

    Serial.println("Successfully conneced to the configured WiFi");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, get_root);

    // Send a GET request to <IP>/get?message=<message>
    server.on("/weight", HTTP_GET, get_weight);

    // Send a GET request to <IP>/get?message=<message>
    server.on("/tare", HTTP_GET, tare);

    // Send a GET request to <IP>/get?message=<message>
    server.on("/tare_status", HTTP_GET, tare_status);

    // Send a GET request to <IP>/get?message=<message>
    server.on("/reset_relay", HTTP_GET, reset_relay);

    // Send a GET request to <IP>/get?message=<message>
    server.on("/set_weight_setpoint", HTTP_GET, set_weight_setpoint);

    // Not found error
    server.onNotFound(not_found);

    server.begin();
}


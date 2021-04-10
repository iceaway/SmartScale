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
        String weight = String(eeprom_setpoint_get(), 1);
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

static void get_stylesheet(AsyncWebServerRequest *request)
{
    request->send(LittleFS, "/ss.css", "text/css");
}

static void get_weight(AsyncWebServerRequest *request)
{
    String value;
    value = String(loadcell_get_weight());
    request->send(200, "text/plain", value);
}

static void get_data(AsyncWebServerRequest *request)
{
    String value;
    value = String(loadcell_get_weight(), 1);
    if (control_get_relay())
        value += ";1;";
    else
        value += ";0;";
    value += String(control_get_elapsed_time());
#ifdef DEBUG
    Serial.println("Get data: " + value);
#endif
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
#ifdef DEBUG
    Serial.println("Reset relay");
#endif
}

static void toggle_relay(AsyncWebServerRequest *request)
{
    if (control_get_relay())
        control_reset_relay();
    else
        control_set_relay();
#ifdef DEBUG
    Serial.println("Toggle relay");
#endif
    request->send(200, "text/plain", "");
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

    server.on("/ss.css", HTTP_GET, get_stylesheet);
    server.on("/get_data", HTTP_GET, get_data);
    server.on("/toggle_relay", HTTP_GET, toggle_relay);
    server.on("/weight", HTTP_GET, get_weight);
    server.on("/tare", HTTP_GET, tare);
    server.on("/tare_status", HTTP_GET, tare_status);
    server.on("/reset_relay", HTTP_GET, reset_relay);
    server.on("/set_weight_setpoint", HTTP_GET, set_weight_setpoint);

    // Not found error
    server.onNotFound(not_found);

    server.begin();
}


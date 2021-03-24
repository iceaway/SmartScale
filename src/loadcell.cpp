#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include <WiFiManager.h>

#include "config.h"
#include "control.h"
#include "eeprom.h"

//HX711 constructor:
static HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);

static volatile boolean g_new_data_ready = false;
static float g_last_weight = 0.0f;
static bool g_update_data = false;

static void change_saved_cal_factor();
static void calibrate();

//interrupt routine:
ICACHE_RAM_ATTR void data_ready_isr()
{
    /* 
     * Only signal the loop function to update the data in the ISR.
     * Previously the data was read from the ADC too, way too much to
     * do in an ISR.
     */
    g_update_data = true;
}

void loadcell_setup(void)
{
    float calibration_value = 696.0f;

    calibration_value = eeprom_calfactor_get();

    LoadCell.begin();
    LoadCell.setSamplesInUse(8);
    unsigned long stabilizingtime = 2000;
    boolean tare = true;
    LoadCell.start(stabilizingtime, tare);
    if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
        Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
        while (1)
            ;
    } else {
        LoadCell.setCalFactor(calibration_value); // set calibration value (float)
        Serial.println("Startup is complete");
    }

    attachInterrupt(digitalPinToInterrupt(HX711_DOUT), data_ready_isr, FALLING);    
}

void loadcell_tare(void)
{
    LoadCell.tareNoDelay();
}

bool loadcell_tare_status(void)
{
    return LoadCell.getTareStatus();
}

static void set_weight()
{
    bool resume = false;
    float w;

    Serial.print("Enter new weight setpoint: ");
    while (resume == false) {
        if (Serial.available() > 0) {
            w = Serial.parseFloat();
            if (w != 0) {
                Serial.print("New weight setpoint is: ");
                Serial.println(w);
                resume = true;
            }
        }
    }

    resume = false;
    Serial.print("Store new eight setpoint in EEPROM? [y/N] ");
    while (resume == false) {
        if (Serial.available() > 0) {
            char c = Serial.read();
            if (c == 'y') {
                eeprom_setpoint_set(w);
                w = eeprom_setpoint_get();
                Serial.println();
                Serial.print("Stored value is: ");
                Serial.println(w);
            } else {
                Serial.println();
                Serial.println("New value not stored in EEPROM");
            }
            resume = true;
        }
    }
}

float loadcell_get_weight(void)
{
    return g_last_weight;
}

void loadcell_loop(void)
{
    const int serial_print_interval = 1000; //increase value to slow down serial print activity
    static unsigned int t = millis(); 
    static bool print_weight = true;
    bool new_data_ready = false;

    if (g_update_data) {
        if (LoadCell.update())
            new_data_ready = true;
        g_update_data = false;
    }

    // get smoothed value from the dataset:
    if (new_data_ready) {
        float f = LoadCell.getData();
        g_last_weight = f;
        
        if (print_weight && (millis() > (t + serial_print_interval))) { 
            Serial.print("Measured weight: ");
            Serial.println(f);
            //Serial.print("  ");
            //Serial.println(millis() - t);
            t = millis();
        }        
    }

    // receive command from serial terminal, send 't' to initiate tare operation:
    if (Serial.available() > 0) {
        char inByte = Serial.read();
        int i = 0;

        switch (inByte) {
        case 'h':
            Serial.println("Available commands:");
            Serial.println("t    - tare");
            Serial.println("r    - calibrate");
            Serial.println("c    - change calibration factor");
            Serial.println("d    - dump EEPROM contents");
            Serial.println("w    - set weight setpoint");
            Serial.println("z    - reset wifi settings");
            Serial.println("p    - enable/disable weight output");
            break;

        case 'p':
          print_weight = !print_weight;
          break;

        case 't':
            LoadCell.tareNoDelay();
            break;

        case 'r':
            calibrate();
            break;

        case 'c':
            change_saved_cal_factor();
            break;

        case 'd':
            EEPROM.begin(EEP_SIZE);
            Serial.println("EEPROM dump:");
            for (i = 0; i < 512; ++i) {
              uint8_t b = EEPROM.read(i);
              if (!i || ((i % 16) == 0))
                Serial.printf("\n%03X: ", i);
              Serial.printf("%02X ", b);

            }
            Serial.println();
            break;

        case 'w':
            set_weight();
            break;

        case 'z':
            WiFiManager wifi;
            noInterrupts();
            wifi.resetSettings();
            ESP.reset();
            break;
        }
            
    }
}

static void calibrate()
{
    Serial.println("***");
    Serial.println("Start calibration:");
    Serial.println("Place the load cell an a level stable surface.");
    Serial.println("Remove any load applied to the load cell.");
    Serial.println("Send 't' from serial monitor to set the tare offset.");

    boolean _resume = false;
    while (_resume == false) {
        LoadCell.update();
        if (Serial.available() > 0) {
            if (Serial.available() > 0) {
                char inByte = Serial.read();
                if (inByte == 't')
                    LoadCell.tareNoDelay();
            }
        }
        if (LoadCell.getTareStatus() == true) {
            Serial.println("Tare complete");
            _resume = true;
        }
    }

    Serial.println("Now, place your known mass on the loadcell.");
    Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

    float known_mass = 0;
    _resume = false;
    while (_resume == false) {
        LoadCell.update();
        if (Serial.available() > 0) {
            known_mass = Serial.parseFloat();
            if (known_mass != 0) {
                Serial.print("Known mass is: ");
                Serial.println(known_mass);
                _resume = true;
            }
        }
    }

    Serial.println("Refresh dataset");
    LoadCell.refreshDataSet(); //refresh the dataset to be sure that the known mass is measured correct
    float new_cal_value = LoadCell.getNewCalibration(known_mass); //get the new calibration value
    Serial.print("New calibration value has been set to: ");
    Serial.print(new_cal_value);
    Serial.println(", use this as calibration value (calFactor) in your project sketch.");
    Serial.print("Save this value to EEPROM adress ");
    Serial.print(EEP_CALIBRATION_VALUE_ADDR);
    Serial.println("? y/n");

    _resume = false;
    while (_resume == false) {
        if (Serial.available() > 0) {
            char c = Serial.read();
            if (c == 'y') {
                eeprom_calfactor_set(new_cal_value);
                new_cal_value = eeprom_calfactor_get();
                Serial.print("Value ");
                Serial.print(new_cal_value);
                Serial.print(" saved to EEPROM address: ");
                Serial.println(EEP_CALIBRATION_VALUE_ADDR);
                _resume = true;
            } else if (c == 'n') {
                Serial.println("Value not saved to EEPROM");
                _resume = true;
            }
        }
    }

    Serial.println("End calibration");
    Serial.println("***");
    Serial.println("To re-calibrate, send 'r' from serial monitor.");
    Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
    Serial.println("***");
}

static void change_saved_cal_factor()
{
    float old_cal_value = LoadCell.getCalFactor();
    float new_cal_value;
    boolean _resume = false;

    Serial.println("***");
    Serial.print("Current value is: ");
    Serial.println(old_cal_value);
    Serial.println("Now, send the new value from serial monitor, i.e. 696.0");
    
    while (_resume == false) {
        if (Serial.available() > 0) {
            new_cal_value = Serial.parseFloat();
            if (new_cal_value != 0) {
                Serial.print("New calibration value is: ");
                Serial.println(new_cal_value);
                LoadCell.setCalFactor(new_cal_value);
                _resume = true;
            }
        }
    }
    _resume = false;
    Serial.print("Save this value to EEPROM adress ");
    Serial.print(EEP_CALIBRATION_VALUE_ADDR);
    Serial.println("? y/n");
    while (_resume == false) {
        if (Serial.available() > 0) {
            char inByte = Serial.read();
            if (inByte == 'y') {
                eeprom_calfactor_set(new_cal_value);
                new_cal_value = eeprom_calfactor_get();
                Serial.print("Value ");
                Serial.print(new_cal_value);
                Serial.print(" saved to EEPROM address: ");
                Serial.println(EEP_CALIBRATION_VALUE_ADDR);
                _resume = true;
            } else if (inByte == 'n') {
                Serial.println("Value not saved to EEPROM");
                _resume = true;
            }
        }
    }
    Serial.println("End change calibration value");
    Serial.println("***");
}

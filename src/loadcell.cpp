/*
   Settling time (number of samples) and data filtering can be adjusted in the config.h file
   For calibration and storing the calibration value in eeprom, see example file "Calibration.ino"

   The update() function checks for new data and starts the next conversion. In order to acheive maximum effective
   sample rate, update() should be called at least as often as the HX711 sample rate; >10Hz@10SPS, >80Hz@80SPS.

   This example shows how call the update() function from an ISR with interrupt on the dout pin.
   Try this if you experince longer settling time due to time consuming code in the loop(),
   i.e. if you are refreshing an graphical LCD, etc.
   The pin used for dout must be external interrupt capable.
*/

#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
#include "config.h"
#include "control.h"

#define HX711_DOUT  5//mcu > HX711 dout pin, must be external interrupt capable!
#define HX711_SCK   4 //mcu > HX711 sck pin

//HX711 constructor:
static HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);


static volatile boolean g_new_data_ready = false;
static float g_last_weight = 0.0f;

static void change_saved_cal_factor();
static void calibrate();

//interrupt routine:
ICACHE_RAM_ATTR void data_ready_isr()
{
    if (LoadCell.update())
    {
        g_new_data_ready = 1;
    }
}

void loadcell_setup(void)
{
    float calibration_value = 696.0f;
    float f;
#if defined(ESP8266) || defined(ESP32)
    EEPROM.begin(EEP_SIZE);
#endif
    EEPROM.get(EEP_CALIBRATION_VALUE_ADDR, f); // uncomment this if you want to fetch the value from eeprom
    /* Simple sanity check */
    if (!isnan(f)) 
        calibration_value = f;

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
                control_set_setpoint(w);
                w = control_get_setpoint();
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

    // check for new data/start next conversion:
    //if (LoadCell.update()) 
    //    g_new_data_ready = true;

    // get smoothed value from the dataset:
    if (g_new_data_ready) {
        if (millis() > t + serial_print_interval) { 
            float f = LoadCell.getData();
            g_new_data_ready = false;
            Serial.print("Load_cell output val: ");
            Serial.println(f);
            //Serial.print("  ");
            //Serial.println(millis() - t);
            t = millis();
            g_last_weight = f;
        }        
    }

    // receive command from serial terminal, send 't' to initiate tare operation:
    if (Serial.available() > 0) {
        char inByte = Serial.read();
        int i = 0;

        switch (inByte) {
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

    // We need to disable interrupts during the calibration routine for some reason
    detachInterrupt(digitalPinToInterrupt(HX711_DOUT));

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
    Serial.println("getNewCalib");
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
#if defined(ESP8266) || defined(ESP32)
                EEPROM.begin(EEP_SIZE);
#endif
                EEPROM.put(EEP_CALIBRATION_VALUE_ADDR, new_cal_value);
#if defined(ESP8266) || defined(ESP32)
                EEPROM.commit();
#endif
                EEPROM.get(EEP_CALIBRATION_VALUE_ADDR, new_cal_value);
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

    attachInterrupt(digitalPinToInterrupt(HX711_DOUT), data_ready_isr, FALLING);
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
#if defined(ESP8266) || defined(ESP32)
                EEPROM.begin(EEP_SIZE);
#endif
                EEPROM.put(EEP_CALIBRATION_VALUE_ADDR, new_cal_value);
#if defined(ESP8266) || defined(ESP32)
                EEPROM.commit();
#endif
                EEPROM.get(EEP_CALIBRATION_VALUE_ADDR, new_cal_value);
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

#ifndef Config_h
#define Config_h

// Pins to use for HX711
#define HX711_DOUT  5//mcu > HX711 dout pin, must be external interrupt capable!
#define HX711_SCK   4 //mcu > HX711 sck pin

// Default values
#define DEFAULT_SETPOINT                18.0f
#define DEFAULT_CALIBRATION_VALUE       696.0f

// EEPROM memory map
#define EEP_SIZE                        512
#define EEP_CALIBRATION_VALUE_ADDR      0
#define EEP_CALIBRATION_VALUE_SIZE      4
#define EEP_SETPOINT_ADDR               ((EEP_CALIBRATION_VALUE_ADDR) + (EEP_CALIBRATION_VALUE_SIZE))
#define EEP_SETPOINT_SIZE               4
#define EEP_CRC32_ADDR                  ((EEP_SIZE) - 4)
#define EEP_CRC32_SIZE                  4

#define WEIGHT_LIMIT_MIN    5.0f
#define WEIGHT_LIMIT_MAX    30.0f

#endif

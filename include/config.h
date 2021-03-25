#ifndef Config_h
#define Config_h

#define HTTP_PORT   80

/* Pins to use for the hardware connections */
#define HX711_DOUT  5 /* D1 */
#define HX711_SCK   4 /* D2 */
#define RELAY_PIN   0 /* D3 */

/* Default values */
#define DEFAULT_SETPOINT                18.0f
#define DEFAULT_CALIBRATION_VALUE       696.0f

/* EEPROM memory map */
#define EEP_SIZE                        512
#define EEP_CALIBRATION_VALUE_ADDR      0
#define EEP_CALIBRATION_VALUE_SIZE      4
#define EEP_SETPOINT_ADDR               ((EEP_CALIBRATION_VALUE_ADDR) + (EEP_CALIBRATION_VALUE_SIZE))
#define EEP_SETPOINT_SIZE               4
#define EEP_CRC32_ADDR                  ((EEP_SIZE) - 4)
#define EEP_CRC32_SIZE                  4

/* Limit the weight setpoint */
#define WEIGHT_LIMIT_MIN    5.0f
#define WEIGHT_LIMIT_MAX    30.0f

#endif

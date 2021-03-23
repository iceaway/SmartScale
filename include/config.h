#ifndef Config_h
#define Config_h

// Default values
#define DEFAULT_SETPOINT                18.0f
#define DEFAULT_CALIBRATION_VALUE       696.0f
#define DEFAULT_SSID                    ""
#define DEFAULT_WPA_PASSPHRASE          ""

// EEPROM memory map
#define EEP_SIZE                        512
#define EEP_CALIBRATION_VALUE_ADDR      0
#define EEP_CALIBRATION_VALUE_SIZE      4
#define EEP_SETPOINT_ADDR               ((EEP_CALIBRATION_VALUE_ADDR) + (EEP_CALIBRATION_VALUE_SIZE))
#define EEP_SETPOINT_SIZE               4
#define EEP_SSID_ADDR                   ((EEP_SETPOINT_ADDR) + (EEP_SETPOINT_SIZE))
#define EEP_SSID_SIZE                   32 /* Including terminating nul byte */
#define EEP_WPA_PASSPHRASE_ADDR         ((EEP_SSID_ADDR) + (EEP_SSID_SIZE))
#define EEP_WPA_PASSPHRASE_SIZE         64 /* Including terminating nul byte */
#define EEP_CRC32_ADDR                  ((EEP_SIZE) - 4)
#define EEP_CRC32_SIZE                  4


#endif
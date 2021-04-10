#ifndef Eeprom_h
#define Eeprom_h

void eeprom_setpoint_set(float s);
float eeprom_setpoint_get(void);

void eeprom_calfactor_set(float c);
float eeprom_calfactor_get(void);

void eeprom_timer_threshold_set(float t);
float eeprom_timer_threshold_get(void);

void eeprom_setup(void);

#endif

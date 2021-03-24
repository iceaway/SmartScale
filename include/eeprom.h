#ifndef Eeprom_h
#define Eeprom_h

void eeprom_setpoint_set(float s);
void eeprom_calfactor_set(float c);
float eeprom_setpoint_get(void);
float eeprom_calfactor_get(void);
void eeprom_setup(void);

#endif

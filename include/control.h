#ifndef Control_h
#define Control_h

void control_loop(void);
void control_setup(void);
void control_set_setpoint(float setpoint);
void control_set_relay(void);
void control_reset_relay(void);
float control_get_setpoint(void);


#endif
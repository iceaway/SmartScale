#ifndef LoadCell_h
#define LoadCell_h

void loadcell_setup(void);
void loadcell_loop(void);
float loadcell_get_weight(void);
void loadcell_tare(void);
bool loadcell_tare_status(void);

#endif
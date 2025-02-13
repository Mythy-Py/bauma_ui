#ifndef _gscope_H_
#define _gscope_H_

#include "Arduino_BMI270_BMM150.h"
#include "math.h"

void calibrate();
void read_sensor_data();
void calc_a(float i); 
void get_position(void);
#endif

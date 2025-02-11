#ifndef _gscope_H_
#define _gscope_H_

#include "Arduino_BMI270_BMM150.h"
#include "math.h"

struct positions
{
  int x;
  int y;
  int z;
};


void calibrate();
void read_sensor_data();
void calc_a(float i); 
#endif

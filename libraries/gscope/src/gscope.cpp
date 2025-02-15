#include "gscope.h"

#if defined(ARDUINO_GIGA)
  BoschSensorClass imu(Wire1);
#elif defined(ARDUINO_NICLA_VISION)
  BoschSensorClass imu(Wire);
#else
  BoschSensorClass imu(Wire);
#endif

float gx, gy, gz;
float gyrX, gyrY, gyrZ;
float accX, accY, accZ;
float freq;
float gyrXoffs, gyrYoffs, gyrZoffs;
int x,y,z;

void gscope_init(void)
{
  imu.begin();
  freq = imu.gyroscopeSampleRate();
  calibrate();
}

void calibrate() {
  for (int i = 0; i < 100; ++i) {
    calc_a(0.08);
  }
  gyrXoffs = gx;
  gyrYoffs = gy;
  gyrZoffs = gz;
}

void read_sensor_data(void) {
  if (imu.accelerationAvailable()) {
    imu.readAcceleration(accX, accY, accZ);
  }
  if (imu.gyroscopeAvailable()) {
    imu.readGyroscope(gyrX, gyrY, gyrZ);
  }
}

void calc_a(float i) {

  read_sensor_data();
  constexpr float x = 180/ M_PI;
  float ay = atan2(accX, sqrt(pow(accY, 2) + pow(accZ, 2))) * x;
  float ax = atan2(accY, sqrt(pow(accX, 2) + pow(accZ, 2))) * x;
  float az = atan2(accZ, sqrt(pow(accX, 2) + pow(accY, 2))) * x;

  gx = gx + gyrX / freq;
  gy = gy - gyrY / freq;
  gz = gz + gyrZ / freq;

  float i2 = 1 - i;
  gx = gx * i + ax * i2;
  gy = gy * i + ay * i2;
  gz = gz * i + az * i2;
}

void get_position(void)
{
  calc_a(0.5);
  x = round(gx - gyrXoffs);
  y = round(gy - gyrYoffs);
  z = round(gz - gyrZoffs);
}

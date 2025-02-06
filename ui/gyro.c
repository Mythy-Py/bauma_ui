#include "Arduino_BMI270_BMM150.h"
#include "math.h"

BoschSensorClass imu(Wire1);


float gx, gy, gz;
float gyrX, gyrY, gyrZ;
float accX, accY, accZ;
float freq;
float gyrXoffs, gyrYoffs, gyrZoffs;


void calibrate() {

  float xSum = 0, ySum = 0, zSum = 0;
  float x = 0, y = 0, z = 0;
  int num = 100;
  for (int i = 0; i < num; ++i) {
    calc_a(0.08);
  }

  gyrXoffs = gx;
  gyrYoffs = gy;
  gyrZoffs = gz;
  Serial.println("gx,gy,gz after Calibration");
  Serial.print(gx);
  Serial.print(" , ");
  Serial.print(gy);
  Serial.print(" , ");
  Serial.println(gz);
}

void read_sensor_data() {
  if (imu.accelerationAvailable()) {
    imu.readAcceleration(accX, accY, accZ);
  }
  if (imu.gyroscopeAvailable()) {
    imu.readGyroscope(gyrX, gyrY, gyrZ);
  }
}

void calc_a(float i) {

  read_sensor_data();
  float ax, ay, az;
  ay = atan2(accX, sqrt(pow(accY, 2) + pow(accZ, 2))) * 180 / M_PI;
  ax = atan2(accY, sqrt(pow(accX, 2) + pow(accZ, 2))) * 180 / M_PI;
  az = atan2(accZ, sqrt(pow(accX, 2) + pow(accY, 2))) * 180 / M_PI;

  gx = gx + gyrX / freq;
  gy = gy - gyrY / freq;
  gz = gz + gyrZ / freq;

  float i2 = 1 - i;
  gx = gx * i + ax * i2;
  gy = gy * i + ay * i2;
  gz = gz * i + az * i2;
}

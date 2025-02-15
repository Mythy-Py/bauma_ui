
#include <ArduinoBLE.h>
#include "Nicla_System.h" 
#include "Arduino_BHY2.h"
#include <math.h>




BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

SensorXYZ accel(SENSOR_ID_ACC);
SensorXYZ gyro(SENSOR_ID_GYRO);

int16_t gx, gy, gz;
int16_t gyrX, gyrY, gyrZ;
int16_t accX, accY, accZ;
float freq;
int16_t gyrXoffs, gyrYoffs, gyrZoffs;
int x,y,z;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  nicla::begin();               // initialise library
  nicla::leds.begin();          // Start I2C connection
  BHY2.begin();
  accel.begin();
  gyro.begin();

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Peripheral - LED control");
  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");


}

void loop() {
  static auto printTime = millis();
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();
  nicla::leds.setColor(red);


  // if a central is connected to peripheral:
  if (central) {
    nicla::leds.setColor(blue);     
    
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          nicla::leds.setColor(green);        // will turn the LED on
        } else {                              // a 0 value
          nicla::leds.setColor(off);            // will turn the LED off
        }
      }
      if (millis() - printTime >= 50) {
      printTime = millis();
      getSensorData();
      }
    }

    // when the central disconnects, print it out:
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void getSensorData(void)
{
  //read_sensor_data();
  // Accelerometer values
  /*
  Serial.print("acc_X:");
  Serial.print(accX);
  Serial.print(",");
  Serial.print("acc_Y:");
  Serial.print(accY);
  Serial.print(",");
  Serial.print("acc_Z:");
  Serial.print(accZ);
  Serial.print(",");

  // Gyroscope values
  Serial.print("gyro_X:");
  Serial.print(gyrX);
  Serial.print(",");
  Serial.print("gyro_Y:");
  Serial.print(gyrY);
  Serial.print(",");
  Serial.print("gyro_Z:");
  Serial.println(gyrZ);
  */
  get_position();
  Serial.print("X:");
  Serial.print(x);
  Serial.print(",");
  Serial.print("Y:");
  Serial.print(y);
  Serial.print(",");
  Serial.print("Z:");
  Serial.println(z);
}

void gscope_init(void)
{
  freq = gyro.getConfiguration().sample_rate;
  calibrate();
}

void calibrate() {
  calc_a(0.08);

  gyrXoffs = gx;
  gyrYoffs = gy;
  gyrZoffs = gz;
}

void read_sensor_data(void) {
  BHY2.update();

  accX = accel.x();
  accY = accel.y();
  accZ = accel.z();
  gyrX = gyro.x();
  gyrY = gyro.y();
  gyrZ = gyro.z();
}

void calc_a(float i) {

  read_sensor_data();
  constexpr float div = 180/ M_PI;
  float ay = atan2(accX, sqrt(pow(accY, 2) + pow(accZ, 2))) * div;
  float ax = atan2(accY, sqrt(pow(accX, 2) + pow(accZ, 2))) * div;
  float az = atan2(accZ, sqrt(pow(accX, 2) + pow(accY, 2))) * div;


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


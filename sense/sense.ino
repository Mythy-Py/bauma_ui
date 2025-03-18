#include "Nicla_System.h"
#include "Arduino_BHY2.h"
#include <ArduinoBLE.h>
#include <math.h>




BLEService ledService("19b10000-e8f2-537e-4f6c-d104768a1214");  // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19b10001-e8f2-537e-4f6c-d104768a1214", BLERead | BLEWrite);
BLEIntCharacteristic X_Value("19B10001-E8F2-537E-4F6C-D104768A1215", BLERead);
BLEIntCharacteristic Y_Value("19B10001-E8F2-537E-4F6C-D104768A1216", BLERead);
BLEIntCharacteristic Z_Value("19B10001-E8F2-537E-4F6C-D104768A1217", BLERead);


SensorXYZ accel(SENSOR_ID_ACC);
SensorXYZ gyro(SENSOR_ID_GYRO);

int16_t gx, gy, gz;
int16_t gyrX, gyrY, gyrZ;
int16_t accX, accY, accZ;
float freq = 50;
int16_t gyrXoffs, gyrYoffs, gyrZoffs;
int x, y, z;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  nicla::begin();       // initialise library
  nicla::leds.begin();  // Start I2C connection

  // initialize the Bluetooth® Low Energy hardware
  BLE.begin();

  Serial.println("Bluetooth® Low Energy Peripheral - LED control");
  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);
  BLE.setDeviceName("Deez");
  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  ledService.addCharacteristic(X_Value);
  ledService.addCharacteristic(Y_Value);
  ledService.addCharacteristic(Z_Value);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");

  gscope_init();
}


RGBColors color = red;
BLEDevice central;
void loop() {
  // listen for Bluetooth® Low Energy peripherals to connect:
  color = color == red ? white : red;

  nicla::leds.setColor(color);
  Serial.println("- Discovering central device...");

  central = BLE.central();
  delay(500);
  getSensorData();
  // if a central is connected to peripheral:
    nicla::leds.setColor(blue);

    // print the central's MAC address:
    if (central) {
      Serial.println("* Connected to central device!");
      Serial.print("* Device MAC address: ");
      Serial.println(central.address());
      Serial.println(" ");
      // while the central is still connected to peripheral:
      while (central.connected()) {
        getSensorData();
        X_Value.setValue(x);
        Y_Value.setValue(y);
        Z_Value.setValue(z);
        // if the remote device wrote to the characteristic,
        // use the value to control the LED:
        if (switchCharacteristic.written()) {
          if (switchCharacteristic.value()) {  // any value other than 0
            Serial.println("On");
            nicla::leds.setColor(green);       // will turn the LED on
          } else {                             // a 0 value
            Serial.println("Off");
            nicla::leds.setColor(off);         // will turn the LED off
          }
        }
        //if (millis() - printTime >= 50) {
        //printTime = millis();
        //getSensorData();
        //}
      }
      // when the central disconnects, print it out:
      Serial.print("Disconnected from central: ");
      Serial.println(central.address());
    }
}

void getSensorData(void) {
  read_sensor_data();
  get_position();
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
  Serial.print("X:");
  Serial.print(x);
  Serial.print(",");
  Serial.print("Y:");
  Serial.print(y);
  Serial.print(",");
  Serial.print("Z:");
  Serial.println(z);
  */
}

void gscope_init(void) {
  //freq = gyro.getConfiguration().sample_rate;
  BHY2.begin(NICLA_I2C);
  accel.begin(freq);
  gyro.begin(freq);
  calibrate();
}

void calibrate() {
  for (int i = 0; i < 200; i++) {
    calc_a(0.5);
  }

  gyrXoffs = gx;
  gyrYoffs = gy;
  gyrZoffs = gz;
  Serial.print("GX Offset: ");
  Serial.println(gyrXoffs);
  Serial.print("GY Offset: ");
  Serial.println(gyrYoffs);
  Serial.print("GZ Offset: ");
  Serial.println(gyrZoffs);
  Serial.println("Done");
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
  constexpr float div = 180 / M_PI;
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

void get_position(void) {
  calc_a(0.2);
  x = round(gx - gyrXoffs);
  y = round(gy - gyrYoffs);
  z = round(gz - gyrZoffs);
}
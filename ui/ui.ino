#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"
#include "lvgl.h"
#include <ui.h>
#include "gscope.h"
#include <ArduinoBLE.h>

extern BoschSensorClass imu;
Arduino_H7_Video Display(800, 480, GigaDisplayShield);  //( 800, 480, GigaDisplayShield );
Arduino_GigaDisplayTouch TouchDetector;

extern lv_obj_t* ui_Xstr;
extern lv_obj_t* ui_Ystr;
extern lv_obj_t* ui_Zstr;
extern lv_obj_t* ui_Dot;
extern lv_obj_t* ui_LabelCalibrate;

extern float gx, gy, gz;
extern float freq;
extern float gyrXoffs, gyrYoffs, gyrZoffs;


//new_files
const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

BLEDevice peripheral;
//end of new files

void setup() {
  Display.begin();
  Serial.begin(9600);
  Serial.println("Started");
  TouchDetector.begin();
  if (imu.begin() == 0)
    Serial.println("Failed to init IMU!");
  freq = imu.gyroscopeSampleRate();
  calibrate();

  ui_init();
  lv_obj_add_event_cb(ui_LabelCalibrate, ui_event_LabelCalibrate, LV_EVENT_ALL, NULL);
}

void loop() {
  lv_timer_handler();
  calc_a(0.5);
  update_vals();
  delay(5);
}

void Serial_print() {
  Serial.print("gx,gy,gz : ");
  Serial.print(gx);
  Serial.print(" , ");
  Serial.print(gy);
  Serial.print(" , ");
  Serial.println(gz);
}

void round_vals(int* x, int* y, int* z)
{
  
  //Overflows
  if (*x > 90)
    *x = -1 * (*x - 90);
  else if (*x < -90)
    *x = -1 * (*x + 90);

  if (*y > 90)
    *y = -1 * (*y - 90);
  else if (*y < -90)
    *y = -1 * (*y + 90);


  if (*z > 90)
    *z = -1 * (*z - 90);
  else if (*z < -90)
    *z = -1 * (*z + 90);
}

void update_vals() {
  int x = round(gx - gyrXoffs);
  int y = round(gy - gyrYoffs);
  int z = round(gz - gyrZoffs);

  lv_label_set_text(ui_Xstr, String("X: " + String(x)).c_str());
  lv_label_set_text(ui_Ystr, String("Y: " + String(y)).c_str());
  lv_label_set_text(ui_Zstr, String("Z: " + String(z)).c_str());

  //Overflows
  round_vals(&x,&y,&z);
  
  constexpr float p_val = 100/90;
  if (x == 0)
    lv_obj_set_x(ui_Dot, lv_pct(0));
  else
    lv_obj_set_x(ui_Dot, lv_pct(round(p_val * x)));

  if (y == 0)
    lv_obj_set_y(ui_Dot, lv_pct(0));
  else
    lv_obj_set_y(ui_Dot, lv_pct(-1 * (round(p_val * y))));
}


// Testfiles
void initBLE(void) {
   if (!BLE.begin()) {
      Serial.println("* Starting Bluetooth® Low Energy module failed!");
      while(1){};
     }
    BLE.setLocalName("Arduino R1 (Central)"); 
    BLE.advertise();

    Serial.println("Arduino R1 BLE (Central Device) started");
    //connectToPeripheral();
}


void connectToPeripheral(){
  
  Serial.println("- Discovering peripheral device...");
  

  unsigned long forceQuit = millis() + 3000;
  while (!peripheral && forceQuit > millis());
  {
    BLE.scanForUuid(deviceServiceUuid);
    peripheral = BLE.available();
  }

  if (peripheral) {
    Serial.println("* Peripheral device found!");
    Serial.print("* Device MAC address: ");
    Serial.println(peripheral.address());
    Serial.print("* Device name: ");
    Serial.println(peripheral.localName());
    Serial.print("* Advertised service UUID: ");
    Serial.println(peripheral.advertisedServiceUuid());
    Serial.println(" ");
    BLE.stopScan();
    controlPeripheral(peripheral);
  }
  else
    Serial.println("* Peripheral device NOT found!");
  
}

void controlPeripheral(BLEDevice peripheral) {
  Serial.println("- Connecting to peripheral device...");

  if (peripheral.connect()) {
    Serial.println("* Connected to peripheral device!");
    Serial.println(" ");
  } else {
    Serial.println("* Connection to peripheral device failed!");
    Serial.println(" ");
    return;
  }

  Serial.println("- Discovering peripheral device attributes...");
  if (peripheral.discoverAttributes()) {
    Serial.println("* Peripheral device attributes discovered!");
    Serial.println(" ");
  } else {
    Serial.println("* Peripheral device attributes discovery failed!");
    Serial.println(" ");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic gestureCharacteristic = peripheral.characteristic(deviceServiceCharacteristicUuid);
    
  if (!gestureCharacteristic) {
    Serial.println("* Peripheral device does not have gesture_type characteristic!");
    peripheral.disconnect();
    return;
  } else if (!gestureCharacteristic.canWrite()) {
    Serial.println("* Peripheral does not have a writable gesture_type characteristic!");
    peripheral.disconnect();
    return;
  }
  
  if (peripheral.connected()) {
      Serial.print("* Writing value to gesture_type characteristic: ");
      Serial.println("* Writing value to gesture_type characteristic done!");
      Serial.println(" ");
  }
  Serial.println("- Peripheral device disconnected!");
}

void ui_event_LabelCalibrate(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        calibrate();
    }
}
// end of Testfiles
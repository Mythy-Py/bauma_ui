#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"
#include "lvgl.h"
#include <ui.h>
#include "gscope.h"
#include <ArduinoBLE.h>

//extern BoschSensorClass imu;
Arduino_H7_Video Display(800, 480, GigaDisplayShield);  //( 800, 480, GigaDisplayShield );
Arduino_GigaDisplayTouch TouchDetector;

extern lv_obj_t* ui_Xstr;
extern lv_obj_t* ui_Ystr;
extern lv_obj_t* ui_Zstr;
extern lv_obj_t* ui_Dot;
extern lv_obj_t* ui_LabelCalibrate;


extern int x,y,z;
int16_t xx,yy,zz;


BLEDevice peripheral;
BLECharacteristic ledCharacteristic;
BLECharacteristic X_val;
BLECharacteristic Y_val;
BLECharacteristic Z_val;
//BLEIntCharacteristic Y_val;
//BLEIntCharacteristic Z_val;

void setup() {
  Display.begin();
  Serial.begin(9600);
  Serial.println("Started");
  TouchDetector.begin();

  gscope_init();

  BLE.begin();  
  Serial.println("Bluetooth® Low Energy Central - LED control");
  ui_init();
  
}

void loop() {
  lv_timer_handler();
  if(!peripheral.connected())
    connect(peripheral);

  get_position();
  update_vals();
  controlLed(peripheral);
  
}

void ui_event_Calibrate(lv_event_t * e) {
        calibrate();
}

void overflows_vals(int* x, int* y, int* z){
  
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

  lv_label_set_text(ui_Xstr, String("X: " + String(xx)).c_str());
  lv_label_set_text(ui_Ystr, String("Y: " + String(yy)).c_str());
  lv_label_set_text(ui_Zstr, String("Z: " + String(zz)).c_str());

  //Overflows
  overflows_vals(&x,&y,&z);
  
  constexpr float p_val = 100/90;
  if (xx == 0)
    lv_obj_set_x(ui_Dot, lv_pct(0));
  else
    lv_obj_set_x(ui_Dot, lv_pct(round(p_val * xx)));

  if (yy == 0)
    lv_obj_set_y(ui_Dot, lv_pct(0));
  else
    lv_obj_set_y(ui_Dot, lv_pct(-1 * (round(p_val * yy))));
}

void Serial_print() {
  Serial.print("x,y,z : ");
  Serial.print(x);
  Serial.print(" , ");
  Serial.print(y);
  Serial.print(" , ");
  Serial.println(z);
}

void connect(BLEDevice& peripheral){
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
  Serial.println("* Start Scanning");
  delay(50);
  
  peripheral = BLE.available();
  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();
    BLE.stopScan();

    if (peripheral.localName() != "LED") {
      return;
    }


    
  // connect to the peripheral
    int tries = 0;
    while(!peripheral.connect() && tries++ < 5 )
    {
        Serial.println("Failed to connect!");
        delay(50);
        Serial.println("Retrying!");
    }

    if (tries == 5)
    {
      Serial.println("Abborted after 5 tries !");
      peripheral.disconnect();
    }
    else
        Serial.println("Connected!");
    



  // discover peripheral attributes
    tries = 0; 
    Serial.println("Discovering attributes ...");
  

    while (!peripheral.discoverAttributes() && tries++ < 10) {
      delay(50);
    }

    if (tries == 5) 
    {
      Serial.println("Attribute discovery failed!");
      peripheral.disconnect();
      BLE.stopScan();
      return;
    }
    else
      Serial.println("Attributes discovered");


    ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");
    if (!ledCharacteristic) {
      Serial.println("Peripheral does not have LED characteristic!");
      peripheral.disconnect();
      return;
    } else if (!ledCharacteristic.canWrite()) {
      Serial.println("Peripheral does not have a writable LED characteristic!");
      peripheral.disconnect();
      return;
    }
    
    X_val = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1215");
    if (!X_val) {
      Serial.println("Peripheral does not have X_val characteristic!");
      peripheral.disconnect();
      return;
    } else if (!X_val.canRead()) {
      Serial.println("Peripheral does not have a readable X_val characteristic!");
      peripheral.disconnect();
      return;
    }
    
    Y_val = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1216");
    if (!Y_val) {
      Serial.println("Peripheral does not have Y_val characteristic!");
      peripheral.disconnect();
      return;
    } else if (!Y_val.canRead()) {
      Serial.println("Peripheral does not have a readable Y_val characteristic!");
      peripheral.disconnect();
      return;
    }

    Z_val = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1217");
    if (!Z_val) {
      Serial.println("Peripheral does not have Z_val characteristic!");
      peripheral.disconnect();
      return;
    } else if (!Z_val.canRead()) {
      Serial.println("Peripheral does not have a readable Z_val characteristic!");
      peripheral.disconnect();
      return;
    }
      Serial.println("Device connected!");
    

  }
  else { 
      // peripheral disconnected, start scanning again
      Serial.println("Device not Found, trying again...");
      // stop scanning
      BLE.stopScan();

  }
}

void controlLed(BLEDevice& peripheral) {

  if (peripheral.connected()) {
    // while the peripheral is connected
    // write 0x01 to turn the LED on

    if (!ledCharacteristic.canWrite()) {
    peripheral.disconnect();
    }

    byte state; 
    ledCharacteristic.readValue(state);
    if(!state)
    ledCharacteristic.writeValue((byte)0x01);
    else
    ledCharacteristic.writeValue((byte)0x00);
    // write 0x00 to turn the LED off
    
      X_val.readValue(xx);
      Y_val.readValue(yy);
      Z_val.readValue(zz);
  }

}
#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"
#include "lvgl.h"
#include <ui.h>
#include "gscope.h"

extern BoschSensorClass imu;
Arduino_H7_Video Display(800, 480, GigaDisplayShield);  //( 800, 480, GigaDisplayShield );
Arduino_GigaDisplayTouch TouchDetector;

extern lv_obj_t* ui_Xstr;
extern lv_obj_t* ui_Ystr;
extern lv_obj_t* ui_Zstr;
extern lv_obj_t* ui_Dot;

extern float gx, gy, gz;
extern float freq;
extern float gyrXoffs, gyrYoffs, gyrZoffs;

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
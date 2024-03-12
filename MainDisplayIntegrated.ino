#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI:
#define OLED_DC    12
#define OLED_CS    5
#define OLED_RESET 13
#define OLED_MOSI  3
#define OLED_CLK   2

// Button pins
#define BUTTON_RED_PIN 9
#define BUTTON_YELLOW_PIN 14
#define BUTTON_BLUE_PIN 15
#define BUTTON_GREEN_PIN 16

#define DEBOUNCE_DELAY 100             // ms between accepted button events
#define PUSH_HOLD_DELAY 1000           // ms to hold button before repeating actions
#define PUSH_HOLD_REPEAT 50            // ms between repeated actions when holding button
#define BNO055_SAMPLERATE_DELAY_MS 100 // ms between IMU samples

// Button class for debouncing and push-hold
class Button {
public:
  int PIN;
  unsigned long last_change;
  int state;
  unsigned long last_trigger;

  Button(int pin) {
    this->PIN = pin;
    this->last_change = 0;
    this->state = 0;
    this->last_trigger = 0;
  }

  boolean shouldTrigger(){
    int newState = digitalRead((PIN));
    boolean will_trigger = false;
    int change_diff = millis() - last_change;
    int trigger_diff = millis() - last_trigger;
    if(state == HIGH && newState == LOW && trigger_diff > DEBOUNCE_DELAY){
      will_trigger = true;
    }else if(newState == LOW && state == LOW && change_diff > PUSH_HOLD_DELAY && trigger_diff > PUSH_HOLD_REPEAT){
      will_trigger = true;
    }

    if(newState != state){
      last_change = millis();
      state = newState;
    }
    if(will_trigger){
      last_trigger = millis();
    }
    return will_trigger;
  }
};

class Vec3d {
public:
  double x, y, z;
  Vec3d() : x(0), y(0), z(0) {}


  Vec3d(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vec3d add(Vec3d other) {
    return Vec3d(x + other.x, y + other.y, z + other.z);
  }

  Vec3d subtract(Vec3d other) {
    return Vec3d(x - other.x, y - other.y, z - other.z);
  }

  Vec3d multiply(double scalar) {
    double newX = x * scalar;
    double newY = y * scalar;
    double newZ = z * scalar;

    return Vec3d(newX, newY, newZ);
  }

  Vec3d cross(Vec3d other) {
    double newX = y * other.z - z * other.y;
    double newY = z * other.x - x * other.z;
    double newZ = x * other.y - y * other.x;

    return Vec3d(newX, newY, newZ);
  }

  double dot(Vec3d other) {
    return x * other.x + y * other.y + z * other.z;
  }

  Vec3d normalize() {
    double magnitude = sqrt(x * x + y * y + z * z);

    // Check for division by zero
    if (magnitude == 0) {
      // Handle the error as needed
    }

    double newX = x / magnitude;
    double newY = y / magnitude;
    double newZ = z / magnitude;

    return Vec3d(newX, newY, newZ);
  }

  double length() {
    return sqrt(x * x + y * y + z * z);
  }

  double lengthSquared() {
    return x * x + y * y + z * z;
  }

  Vec3d project(Vec3d other) {
    double dotProduct = this->dot(other);
    double uMagnitudeSquared = other.lengthSquared();

    // Check for division by zero
    if (uMagnitudeSquared == 0) {
      // Handle the error as needed
    }

    double scalar = dotProduct / uMagnitudeSquared;

    return other.multiply(scalar);
  }

  Vec3d applyEWMA(double alpha, Vec3d ewma){
    return Vec3d(x*alpha + (1-alpha)*ewma.x,y*alpha + (1-alpha)*ewma.y,z*alpha + (1-alpha)*ewma.z);
  }

  String toString() {
    return "(" + String(x) + ", " + String(y) + ", " + String(z) + ")";
  }

  bool operator!=(const Vec3d &other) const {
    return x != other.x || y != other.y || z != other.z;
  }
};

const unsigned char epd_bitmap_display_map [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7d, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7d, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7d, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x5d, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x55, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x75, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x7d, 
  0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x45, 
  0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x01, 
  0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x7d, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x45, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x75, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x7d, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x7d, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x05, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x7d, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x05, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0xa0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x41, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 
  0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x25, 
  0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x11, 
  0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x09, 
  0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x25, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x39, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x6d, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x45, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7d, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
// 'number_map_0', 5x3px
const unsigned char epd_bitmap_number_map_0 [] PROGMEM = {
  0xf0, 0x88, 0xf8
};
// 'number_map_1', 5x3px
const unsigned char epd_bitmap_number_map_1 [] PROGMEM = {
  0x90, 0xf8, 0x80
};
// 'number_map_2', 5x3px
const unsigned char epd_bitmap_number_map_2 [] PROGMEM = {
  0xe8, 0xa8, 0x98
};
// 'number_map_3', 5x3px
const unsigned char epd_bitmap_number_map_3 [] PROGMEM = {
  0x88, 0xa8, 0xf8
};
// 'number_map_4', 5x3px
const unsigned char epd_bitmap_number_map_4 [] PROGMEM = {
  0x38, 0x20, 0xf8
};
// 'number_map_5', 5x3px
const unsigned char epd_bitmap_number_map_5 [] PROGMEM = {
  0xb8, 0xa8, 0xc8
};
// 'number_map_6', 5x3px
const unsigned char epd_bitmap_number_map_6 [] PROGMEM = {
  0xf8, 0xa8, 0xe8
};
// 'number_map_7', 5x3px
const unsigned char epd_bitmap_number_map_7 [] PROGMEM = {
  0x08, 0x28, 0xf8
};
// 'number_map_8', 5x3px
const unsigned char epd_bitmap_number_map_8 [] PROGMEM = {
  0xf8, 0xa8, 0xf8
};
// 'number_map_9', 5x3px
const unsigned char epd_bitmap_number_map_9 [] PROGMEM = {
  0xb8, 0xa8, 0xf8
};

// 'target_marker', 7x7px
const unsigned char epd_bitmap_target_marker [] PROGMEM = {
  0x10, 0x38, 0x44, 0xd6, 0x44, 0x38, 0x10
};
// 'center_crosshair', 9x9px
const unsigned char epd_bitmap_center_crosshair [] PROGMEM = {
  0x08, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0xbe, 0x80, 0x08, 0x00, 0x08, 0x00, 0x00, 0x00, 
  0x08, 0x00
};
// 'left_arrows', 19x9px
const unsigned char epd_bitmap_left_arrows [] PROGMEM = {
  0x00, 0xe0, 0x00, 0x03, 0xf8, 0x00, 0x0f, 0x1e, 0x00, 0x3c, 0x07, 0x80, 0xf0, 0xe1, 0xe0, 0x03, 
  0xf8, 0x00, 0x0f, 0x1e, 0x00, 0x3c, 0x07, 0x80, 0xf0, 0x01, 0xe0
};
// 'right_arrows', 19x9px
const unsigned char epd_bitmap_right_arrows [] PROGMEM = {
  0xf0, 0x01, 0xe0, 0x3c, 0x07, 0x80, 0x0f, 0x1e, 0x00, 0x03, 0xf8, 0x00, 0xf0, 0xe1, 0xe0, 0x3c, 
  0x07, 0x80, 0x0f, 0x1e, 0x00, 0x03, 0xf8, 0x00, 0x00, 0xe0, 0x00
};
// 'up_arrows', 12x9px
const unsigned char epd_bitmap_up_arrows [] PROGMEM = {
  0xc3, 0x00, 0x61, 0x80, 0x30, 0xc0, 0x18, 0x60, 0x0c, 0x30, 0x18, 0x60, 0x30, 0xc0, 0x61, 0x80, 
  0xc3, 0x00
};
// 'down_arrows', 12x9px
const unsigned char epd_bitmap_down_arrows [] PROGMEM = {
  0x0c, 0x30, 0x18, 0x60, 0x30, 0xc0, 0x61, 0x80, 0xc3, 0x00, 0x61, 0x80, 0x30, 0xc0, 0x18, 0x60, 
  0x0c, 0x30
};
// 'roll_indicator_left', 3x11px
const unsigned char epd_bitmap_roll_indicator_left [] PROGMEM = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60, 0xc0
};
// 'roll_indicator_right', 3x11px
const unsigned char epd_bitmap_roll_indicator_right [] PROGMEM = {
  0xc0, 0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};
// 'counterclockwise_arrows', 25x25px
const unsigned char epd_bitmap_counterclockwise_arrows [] PROGMEM = {
  0x00, 0x3e, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x07, 0xc0, 0x3f, 0x00, 0x0e, 0x00, 0x3e, 0x00, 
  0x1c, 0x00, 0x3c, 0x00, 0x38, 0x00, 0x3e, 0x00, 0x30, 0x00, 0x37, 0x00, 0x60, 0x00, 0x23, 0x00, 
  0x60, 0x00, 0x03, 0x00, 0x60, 0x00, 0x03, 0x00, 0xc0, 0x00, 0x01, 0x80, 0xc0, 0x00, 0x01, 0x80, 
  0xc0, 0x00, 0x01, 0x80, 0xc0, 0x00, 0x01, 0x80, 0xc0, 0x00, 0x01, 0x80, 0x60, 0x00, 0x03, 0x00, 
  0x60, 0x00, 0x03, 0x00, 0x62, 0x00, 0x03, 0x00, 0x36, 0x00, 0x07, 0x00, 0x3e, 0x00, 0x0e, 0x00, 
  0x1e, 0x00, 0x1c, 0x00, 0x3e, 0x00, 0x38, 0x00, 0x7e, 0x01, 0xf0, 0x00, 0x00, 0x3f, 0xe0, 0x00, 
  0x00, 0x3e, 0x00, 0x00
};
// 'clockwise_arrows', 25x25px
const unsigned char epd_bitmap_clockwise_arrows [] PROGMEM = {
  0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x7e, 0x01, 0xf0, 0x00, 0x3e, 0x00, 0x38, 0x00, 
  0x1e, 0x00, 0x1c, 0x00, 0x3e, 0x00, 0x0e, 0x00, 0x36, 0x00, 0x07, 0x00, 0x62, 0x00, 0x03, 0x00, 
  0x60, 0x00, 0x03, 0x00, 0x60, 0x00, 0x03, 0x00, 0xc0, 0x00, 0x01, 0x80, 0xc0, 0x00, 0x01, 0x80, 
  0xc0, 0x00, 0x01, 0x80, 0xc0, 0x00, 0x01, 0x80, 0xc0, 0x00, 0x01, 0x80, 0x60, 0x00, 0x03, 0x00, 
  0x60, 0x00, 0x03, 0x00, 0x60, 0x00, 0x23, 0x00, 0x30, 0x00, 0x37, 0x00, 0x38, 0x00, 0x3e, 0x00, 
  0x1c, 0x00, 0x3c, 0x00, 0x0e, 0x00, 0x3e, 0x00, 0x07, 0xc0, 0x3f, 0x00, 0x03, 0xfe, 0x00, 0x00, 
  0x00, 0x3e, 0x00, 0x00
};

// 'number_map_big_0', 6x10px
const unsigned char epd_bitmap_number_map_big_0 [] PROGMEM = {
  0x78, 0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x78
};
// 'number_map_big_1', 6x10px
const unsigned char epd_bitmap_number_map_big_1 [] PROGMEM = {
  0x30, 0x70, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0xfc
};
// 'number_map_big_2', 6x10px
const unsigned char epd_bitmap_number_map_big_2 [] PROGMEM = {
  0x78, 0xfc, 0xcc, 0x0c, 0x7c, 0xf8, 0xc0, 0xc0, 0xfc, 0xfc
};
// 'number_map_big_3', 6x10px
const unsigned char epd_bitmap_number_map_big_3 [] PROGMEM = {
  0x78, 0xfc, 0xcc, 0x0c, 0x3c, 0x3c, 0x0c, 0xcc, 0xfc, 0x78
};
// 'number_map_big_4', 6x10px
const unsigned char epd_bitmap_number_map_big_4 [] PROGMEM = {
  0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x7c, 0x0c, 0x0c, 0x0c, 0x0c
};
// 'number_map_big_5', 6x10px
const unsigned char epd_bitmap_number_map_big_5 [] PROGMEM = {
  0xfc, 0xfc, 0xc0, 0xc0, 0xf8, 0x7c, 0x0c, 0x0c, 0xfc, 0xf8
};
// 'number_map_big_6', 6x10px
const unsigned char epd_bitmap_number_map_big_6 [] PROGMEM = {
  0x78, 0xfc, 0xcc, 0xc0, 0xf8, 0xfc, 0xcc, 0xcc, 0xfc, 0x78
};
// 'number_map_big_7', 6x10px
const unsigned char epd_bitmap_number_map_big_7 [] PROGMEM = {
  0xfc, 0xfc, 0x0c, 0x0c, 0x1c, 0x18, 0x38, 0x30, 0x70, 0x60
};
// 'number_map_big_8', 6x10px
const unsigned char epd_bitmap_number_map_big_8 [] PROGMEM = {
  0x78, 0xfc, 0xcc, 0xcc, 0x78, 0x78, 0xcc, 0xcc, 0xfc, 0x78
};
// 'number_map_big_9', 6x10px
const unsigned char epd_bitmap_number_map_big_9 [] PROGMEM = {
  0x78, 0xfc, 0xcc, 0xcc, 0xfc, 0x7c, 0x0c, 0xcc, 0xfc, 0x78
};
// 'display_menu', 128x64px
const unsigned char epd_bitmap_display_menu [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x97, 0xfe, 0x9f, 0x88, 0x78, 0x79, 0xf7, 0xe1, 0xe0, 0x43, 0x33, 0xcf, 0x80, 0x00, 0x00, 0x01, 
  0x97, 0xfe, 0x9f, 0x9c, 0x7c, 0xf9, 0xf7, 0xe1, 0xf0, 0xe3, 0x37, 0xcf, 0x80, 0x00, 0x00, 0x01, 
  0x96, 0x06, 0x86, 0x1c, 0x6e, 0xc1, 0x81, 0x81, 0xb8, 0xe3, 0xb6, 0x0c, 0x30, 0x00, 0x00, 0x01, 
  0x96, 0x06, 0x86, 0x36, 0x66, 0xc1, 0x81, 0x81, 0x99, 0xb3, 0xf6, 0x0c, 0x30, 0x00, 0x00, 0x01, 
  0x96, 0x66, 0x86, 0x36, 0x6e, 0xd9, 0xe1, 0x81, 0xb9, 0xb3, 0xf6, 0xcf, 0x00, 0x00, 0x00, 0x01, 
  0x96, 0x66, 0x86, 0x3e, 0x7c, 0xdd, 0xe1, 0x81, 0xf1, 0xf3, 0x76, 0xef, 0x00, 0x00, 0x00, 0x01, 
  0x96, 0x06, 0x86, 0x3e, 0x78, 0xcd, 0x81, 0x81, 0xe1, 0xf3, 0x36, 0x6c, 0x30, 0x00, 0x00, 0x01, 
  0x96, 0x06, 0x86, 0x77, 0x6c, 0xcd, 0x81, 0x81, 0xb3, 0xbb, 0x36, 0x6c, 0x30, 0x00, 0x00, 0x01, 
  0x97, 0xfe, 0x86, 0x63, 0x6e, 0xfd, 0xf1, 0x81, 0xbb, 0x1b, 0x37, 0xef, 0x80, 0x00, 0x00, 0x01, 
  0x97, 0xfe, 0x86, 0x63, 0x66, 0x79, 0xf1, 0x81, 0x9b, 0x1b, 0x33, 0xcf, 0x80, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x4e, 0x82, 0x1e, 0x3c, 0x3c, 0x82, 0x0f, 0xbe, 0x7d, 0xf7, 0x80, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x86, 0x87, 0x1f, 0x3e, 0x7e, 0xc6, 0x1f, 0xbf, 0x7d, 0xf7, 0xc0, 0x00, 0x00, 0x00, 0x01, 
  0x91, 0x0a, 0x87, 0x1b, 0xb7, 0x66, 0xc6, 0x18, 0x33, 0x61, 0x86, 0xec, 0x00, 0x00, 0x00, 0x01, 
  0x92, 0x10, 0x8d, 0x99, 0xb3, 0x66, 0xc6, 0x18, 0x33, 0x61, 0x86, 0x6c, 0x00, 0x00, 0x00, 0x01, 
  0x94, 0x20, 0x8d, 0x9b, 0xb7, 0x66, 0xd6, 0x1f, 0x3f, 0x79, 0xe6, 0x60, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x42, 0x8f, 0x9f, 0x3e, 0x66, 0xfe, 0x0f, 0xbe, 0x79, 0xe6, 0x60, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x84, 0x8f, 0x9e, 0x3c, 0x66, 0xfe, 0x01, 0xb0, 0x61, 0x86, 0x6c, 0x00, 0x00, 0x00, 0x01, 
  0x93, 0x08, 0x9d, 0xdb, 0x36, 0x66, 0xee, 0x01, 0xb0, 0x61, 0x86, 0xec, 0x00, 0x00, 0x00, 0x01, 
  0x97, 0x10, 0x98, 0xdb, 0xb7, 0x7e, 0xc6, 0x1f, 0xb0, 0x7d, 0xf7, 0xc0, 0x00, 0x00, 0x00, 0x01, 
  0x92, 0x20, 0x98, 0xd9, 0xb3, 0x3c, 0x44, 0x1f, 0x30, 0x7d, 0xf7, 0x80, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x92, 0x82, 0x99, 0xb3, 0x7e, 0xfc, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x95, 0x44, 0x99, 0xb3, 0x7e, 0xfd, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x95, 0x48, 0x99, 0xbb, 0x18, 0x31, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x94, 0x50, 0x99, 0xbf, 0x18, 0x31, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x99, 0xbf, 0x18, 0x31, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x99, 0xb7, 0x18, 0x30, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x6e, 0x99, 0xb3, 0x18, 0x30, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x91, 0x44, 0x99, 0xb3, 0x18, 0x30, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x92, 0x64, 0x9f, 0xb3, 0x7e, 0x31, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x94, 0x44, 0x8f, 0x33, 0x7e, 0x31, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x90, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x8f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
// 'menu_selector', 124x18px
const unsigned char epd_bitmap_menu_selector [] PROGMEM = {
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 
  0xc0, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
  0xc0, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0
};
// 'meters', 40x10px
const unsigned char epd_bitmap_meters [] PROGMEM = {
  0xc6, 0xfb, 0xf7, 0xde, 0x1f, 0xc6, 0xfb, 0xf7, 0xdf, 0x3f, 0xee, 0xc0, 0xc6, 0x1b, 0xb0, 0xfe, 
  0xc0, 0xc6, 0x19, 0xb0, 0xfe, 0xf0, 0xc7, 0x9b, 0xbe, 0xd6, 0xf0, 0xc7, 0x9f, 0x1f, 0xc6, 0xc0, 
  0xc6, 0x1e, 0x03, 0xc6, 0xc0, 0xc6, 0x1b, 0x03, 0xc6, 0xf8, 0xc7, 0xdb, 0xbf, 0xc6, 0xf8, 0xc7, 
  0xd9, 0xbe
};
// 'feet', 24x10px
const unsigned char epd_bitmap_feet [] PROGMEM = {
  0xfb, 0xef, 0xbf, 0xfb, 0xef, 0xbf, 0xc3, 0x0c, 0x0c, 0xc3, 0x0c, 0x0c, 0xf3, 0xcf, 0x0c, 0xf3, 
  0xcf, 0x0c, 0xc3, 0x0c, 0x0c, 0xc3, 0x0c, 0x0c, 0xc3, 0xef, 0x8c, 0xc3, 0xef, 0x8c
};

// Array of all bitmaps for convenience.
const int epd_bitmap_allArray_LEN = 35;
const unsigned char* epd_bitmap_allArray[epd_bitmap_allArray_LEN] = {
  epd_bitmap_number_map_0, // 0
  epd_bitmap_number_map_1, // 1
  epd_bitmap_number_map_2, // 2
  epd_bitmap_number_map_3, // 3
  epd_bitmap_number_map_4, // 4
  epd_bitmap_number_map_5, // 5 
  epd_bitmap_number_map_6, // 6
  epd_bitmap_number_map_7, // 7
  epd_bitmap_number_map_8, // 8
  epd_bitmap_number_map_9, // 9 
  epd_bitmap_display_map, // 10
  epd_bitmap_target_marker, // 11
  epd_bitmap_center_crosshair, // 12
  epd_bitmap_left_arrows, // 13
  epd_bitmap_right_arrows, // 14
  epd_bitmap_up_arrows, // 15
  epd_bitmap_down_arrows, // 16
  epd_bitmap_roll_indicator_left, // 17
  epd_bitmap_roll_indicator_right, // 18
  epd_bitmap_counterclockwise_arrows, // 19
  epd_bitmap_clockwise_arrows, // 20
  epd_bitmap_number_map_big_0, // 21
  epd_bitmap_number_map_big_1, // 22
  epd_bitmap_number_map_big_2, // 23
  epd_bitmap_number_map_big_3, // 24
  epd_bitmap_number_map_big_4, // 25
  epd_bitmap_number_map_big_5, // 26
  epd_bitmap_number_map_big_6, // 27
  epd_bitmap_number_map_big_7, // 28
  epd_bitmap_number_map_big_8, // 29
  epd_bitmap_number_map_big_9, // 30
  epd_bitmap_display_menu, // 31
  epd_bitmap_menu_selector, // 32
  epd_bitmap_meters, // 33
  epd_bitmap_feet // 34
};

// Global Variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS); // initialize display class
Adafruit_BNO055 bno = Adafruit_BNO055(55,0x28, &Wire);                                                    // initialize IMU 1 class
Adafruit_BNO055 bno2 = Adafruit_BNO055(56,0x29, &Wire);                                                   // initialize IMU 2 class

int mode = 0; // 0 - menu, 1 - target
Button red_button(BUTTON_RED_PIN);
Button green_button(BUTTON_GREEN_PIN);
Button yellow_button(BUTTON_YELLOW_PIN);
Button blue_button(BUTTON_BLUE_PIN);
double alpha = 0.5; // ewma coefficient
unsigned long last_sensor_check;
Vec3d ewma, ewma2, sensor_mean = {0,0,0};

// Menu Variables
int selected_param = 0; // 0 - distance, 1 - speed, 2 - units
int raw_distance = 18;
int raw_speed = 75; // units / second
int units = 0; // 0 - meters, 1 - feet

// Target Variables
int battery = 99;
int target_x = 32;
int target_y = 16;

void setup() {
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.begin(115200); // set communication speed for the serial monitor
  while (!Serial) delay(10);  // wait for serial port to open!

  if (!bno.begin() || !bno2.begin()){
    Serial.print("Oops, both BNO055s haven't been detected. ... Check your wiring or I2C ADDR!");
    while (1);
  }  

  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_YELLOW_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);

  // Clear the buffer.
  display.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:

  read_buttons();

  if(mode == 0){
    render_menu();
  }else{
    if (millis()-last_sensor_check > BNO055_SAMPLERATE_DELAY_MS){    
      display.clearDisplay();           
      Vec3d sensor_data = read_sensors();   

      Serial.print("1- (");
      Serial.print(sensor_data.x);
      Serial.print(",");
      Serial.print(sensor_data.y);
      Serial.print(",");
      Serial.print(sensor_data.z);
      Serial.println(")");

      render_target(sensor_data);
    }
  }
}

void render_menu(){
  display.clearDisplay();

  int range_1 = (int) raw_distance / 100;
  int range_2 = (int) (raw_distance/10) % 10;
  int range_3 = (int) raw_distance % 10;

  int speed_1 = (int) raw_speed / 100;
  int speed_2 = (int) (raw_speed/10) % 10;
  int speed_3 = (int) raw_speed % 10;

  // Main Background
  display.drawBitmap(0, 0, epd_bitmap_allArray[31], 128, 64, 1); 

  // Selector Bar
  display.drawBitmap(2, 2+selected_param*21, epd_bitmap_allArray[32], 124, 18, 1); 

  // Range Readout
  display.drawBitmap(102, 6, epd_bitmap_allArray[21+range_1], 6, 10, 1); 
  display.drawBitmap(109, 6, epd_bitmap_allArray[21+range_2], 6, 10, 1); 
  display.drawBitmap(116, 6, epd_bitmap_allArray[21+range_3], 6, 10, 1); 

  // Speed Readout
  display.drawBitmap(98, 26, epd_bitmap_allArray[21+speed_1], 6, 10, 1); 
  display.drawBitmap(105, 26, epd_bitmap_allArray[21+speed_2], 6, 10, 1); 
  display.drawBitmap(112, 26, epd_bitmap_allArray[21+speed_3], 6, 10, 1); 

  // Units Display
  if(units == 0){
    display.drawBitmap(59, 48, epd_bitmap_allArray[33], 40, 10, 1); 
  }else{
    display.drawBitmap(59, 48, epd_bitmap_allArray[34], 24, 10, 1); 
  }
  // Load to the display
  display.display();
}

Vec3d read_sensors(){
  last_sensor_check = millis();
  uint8_t sys, gyro, accel, mag = 0;
  uint8_t sys2, gyro2, accel2, mag2 = 0;
  Vec3d s0_1, s0_2 = {0,0,0};
  uint32_t address = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);
  bno2.getCalibration(&sys2, &gyro2, &accel2, &mag2);

  sensors_event_t event, event2;
  bno.getEvent(&event);
  bno2.getEvent(&event2);
  
  // using orientation for this example and can do the same for multiple sensors
  s0_1 = {360 - (double)event.orientation.x, (double)event.orientation.y, (double)event.orientation.z};
  s0_2 = {360 - (double)event2.orientation.x, (double)event2.orientation.y, (double)event2.orientation.z};
  
  ewma = s0_1.applyEWMA(alpha, ewma);
  ewma2 = s0_2.applyEWMA(alpha, ewma2);
  sensor_mean = ewma.add(ewma2).multiply(0.5);

  // showing writing to the max address and loopback for a partition of the EEPROM:
  if(address < 100*2*sizeof(uint8_t)){
  // Serial.print(F("Calibration Sensor 1: "));
  // Serial.print(sys, DEC);
  // Serial.println(F(""));
  // Serial.print(F("Calibration Sensor 2: "));
  // Serial.print(sys2, DEC);
  // Serial.println(F(""));
  byte calibrations[2] = {sys, sys2};                                // create variable to hold the data value sent
  address += sizeof(calibrations); 
  }
  else {
    address = 0;
  }

  return Vec3d(sensor_mean.x,sensor_mean.y,sensor_mean.z);
}

void read_buttons(){
  if(red_button.shouldTrigger()){
    mode = (mode+1) % 2;
  }
  if(mode == 0){
    if(blue_button.shouldTrigger()){
      if(selected_param == 0 && raw_distance > 1){
        raw_distance--;
      }else if(selected_param == 1 && raw_speed > 1){
        raw_speed--;
      }else if(selected_param == 2){
        units = (units+1) % 2;
      }
    }
    if(green_button.shouldTrigger()){
      if(selected_param == 0 && raw_distance < 998){
        raw_distance++;
      }else if(selected_param == 1 && raw_speed < 998){
        raw_speed++;
      }else if(selected_param == 2){
        units = (units+1) % 2;
      }
    }
  if(yellow_button.shouldTrigger()){
      selected_param = (selected_param+1) % 3;
    }
  }
}

void render_target(Vec3d orient){
  double roll = orient.x;
  double pitch = orient.y;
  double yaw = orient.z;

  double abs_pitch = abs(pitch);
  int pitch_1 = (int) abs_pitch / 10;
  int pitch_2 = (int) abs_pitch % 10;
  int pitch_3 = (int) (abs_pitch*10) % 10;
  int pitch_4 = (int) (abs_pitch*100) % 10;

  int yaw_1 = (int) yaw / 100;
  int yaw_2 = (int) (yaw/10) % 10;
  int yaw_3 = (int) yaw % 10;
  int yaw_4 = (int) (yaw*10) % 10;
  int yaw_5 = (int) (yaw*100) % 10;

  int battery_1 = battery / 10;
  int battery_2 = battery % 10;

  int target_drift_x = random(5)-2;
  int target_drift_y = random(5)-2;
  if(target_x+target_drift_x > 110 || target_x+target_drift_x < 10){
    target_drift_x *= -1;
  }

  if(target_y+target_drift_y > 63 || target_y+target_drift_y < 1){
    target_drift_y *= -1;
  }

  target_x = (int)(yaw * (45.0/180.0) + 60);
  target_y = (int)(pitch * (30.0/180.0) + 62);

  boolean left_arrows = false;
  boolean right_arrows = false;
  boolean up_arrows = false;
  boolean down_arrows = false;

  int crosshair_x = 59;
  int crosshair_y = 28;

  // Main Background
  display.drawBitmap(0, 0, epd_bitmap_allArray[10], 128, 64, 1);

  // Pitch Indicator
  if(pitch_1 >= 0 && pitch_1 <= 9){
    display.drawBitmap(113, 26, epd_bitmap_allArray[pitch_1], 5, 3, 1);
  }

  if(pitch_2 >= 0 && pitch_2 <= 9){
    display.drawBitmap(113, 30, epd_bitmap_allArray[pitch_2], 5, 3, 1);
  }
  if(pitch_3 >= 0 && pitch_3 <= 9){
    display.drawBitmap(113, 36, epd_bitmap_allArray[pitch_3], 5, 3, 1);
  }
  if(pitch_4 >= 0 && pitch_4 <= 9){
    display.drawBitmap(113, 40, epd_bitmap_allArray[pitch_4], 5, 3, 1);
  }
  
  // Pitch Sign
  if(!pitch > 0){
    display.drawPixel(114,23,1);
    display.drawPixel(116,23,1);
  }
  // Yaw Indicator
  display.drawBitmap(2, 22, epd_bitmap_allArray[yaw_1], 5, 3, 1);
  display.drawBitmap(2, 26, epd_bitmap_allArray[yaw_2], 5, 3, 1);
  display.drawBitmap(2, 30, epd_bitmap_allArray[yaw_3], 5, 3, 1);
  display.drawBitmap(2, 36, epd_bitmap_allArray[yaw_4], 5, 3, 1);
  display.drawBitmap(2, 40, epd_bitmap_allArray[yaw_5], 5, 3, 1);
  // Roll Indicator
  if(roll < -25){
    display.drawBitmap(51, 20, epd_bitmap_allArray[20], 25, 25, 1);
  }else if(roll > 25){
    display.drawBitmap(51, 20, epd_bitmap_allArray[19], 25, 25, 1);
  }else{
    int offset = (int) roll;
    offset = 0;
    display.drawBitmap(61-offset, 42, epd_bitmap_allArray[18], 3, 11, 1);
    display.drawBitmap(61+offset, 12, epd_bitmap_allArray[17], 3, 11, 1);
  }

  // Battery Percentage
  display.drawBitmap(121, 37, epd_bitmap_allArray[battery_1], 5, 3, 1);
  display.drawBitmap(121, 41, epd_bitmap_allArray[battery_2], 5, 3, 1);
  // Battery Fill
  for(int i = 0; i < battery_1; i++){
    display.drawPixel(122,59-i,1);
    display.drawPixel(123,59-i,1);
    display.drawPixel(124,59-i,1);
  }

  // Arrows
  if(down_arrows){  
    display.drawBitmap(3, 3, epd_bitmap_allArray[16], 12, 9, 1);
    display.drawBitmap(3, 52, epd_bitmap_allArray[16], 12, 9, 1);
  }
  if(up_arrows){  
    display.drawBitmap(105, 3, epd_bitmap_allArray[15], 12, 9, 1);
    display.drawBitmap(105, 52, epd_bitmap_allArray[15], 12, 9, 1);
  }
  if(left_arrows){  
    display.drawBitmap(54, 3, epd_bitmap_allArray[13], 19, 9, 1);
  }
  if(right_arrows){
    display.drawBitmap(54, 52, epd_bitmap_allArray[14], 19, 9, 1);
  }

  // Crosshair
  display.drawBitmap(crosshair_x,crosshair_y, epd_bitmap_allArray[12], 9, 9, 1);

  // Target Marker
  display.drawBitmap(target_x-3,target_y-3, epd_bitmap_allArray[11], 7, 7, 1);

  // Load to the display
  display.display();
  
  battery--;
  if(battery < 0){
    battery = 99;
  }
}

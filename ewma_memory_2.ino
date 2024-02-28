#include <EEPROMsimple.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.

   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3-5V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/
#define CSPIN 10       // Default Chip Select Line for Uno (change as needed)

EEPROMsimple EEPROM;       // initialize an instance of this class

/* Set the delay between fresh samples */
uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;

// we can play with alpha more to see if there's a better value
double alpha = 0.5; 
// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55,0x28, &Wire);
Adafruit_BNO055 bno2 = Adafruit_BNO055(55,0x29, &Wire);

// x,y,z coords of an event
struct coord {
    double x;
    double y;
    double z;
};

void setup(void)
{
  Serial.begin(9600);

  while (!Serial) delay(10);  // wait for serial port to open!

  /* Initialise the sensor */
  if (!bno.begin() || !bno2.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Oops, both BNO055s haven't been detected. ... Check your wiring or I2C ADDR!");
    while (1);
  }
  uint32_t address = 0;                       // create a 32 bit variable to hold the address (uint32_t=long)
  byte value;                                 // create variable to hold the data value read
  byte data;                                  // create variable to hold the data value sent                       // set communication speed for the serial monitor
  SPI.begin();                                // start communicating with the memory chip  
  delay(1000);
  coord ewma = {0, 0, 0};
  coord ewma2 = {0, 0, 0};
  coord s0_1 = {0, 0, 0};
  coord s0_2 = {0, 0, 0};
}

void loop(void)
{
  
  uint8_t sys, gyro, accel, mag = 0;
  uint8_t sys2, gyro2, accel2, mag2 = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);
  bno2.getCalibration(&sys2, &gyro2, &accel2, &mag2);

  sensors_event_t event, event2;
  bno.getEvent(&event);
  bno2.getEvent(&event2);
  
  // using orientation for this example and can do the same for multiple sensors
  s0_1 = {360 - (double)event.orientation.x, (double)event.orientation.y, (double)event.orientation.z};
  s0_2 = {360 - (double)event2.orientation.x, (double)event2.orientation.y, (double)event2.orientation.z};
  
  ewma = applyEWMA(s0_1, alpha, ewma);
  ewma2 = applyEWMA(s0_1, alpha, ewma2);
  
  Serial.print(F("Orientation 1: "));
  Serial.print(ewma.x);
  Serial.print(F(", "));
  Serial.print(ewma.y);
  Serial.print(F(", "));
  Serial.print(ewma.z);
  Serial.println(F(""));


  Serial.print(F("Orientation 2: "));
  Serial.print(ewma2.x);
  Serial.print(F(", "));
  Serial.print(ewma2.y);
  Serial.print(F(", "));
  Serial.print(ewma2.z);
  Serial.println(F(""));


  // showing writing to the max address and loopback for a partition of the EEPROM:
  if(address < 100*sizeof(2* uint_8)){
  Serial.print(F("Calibration Sensor 1: "));
  Serial.print(sys, DEC);
  Serial.println(F(""));
  Serial.print(F("Calibration Sensor 2: "));
  Serial.print(sys2, DEC);
  Serial.println(F(""));
  byte calibrations[2] = {sys, sys2};                                // create variable to hold the data value sent
  EEPROM.WriteByteArray(address, calibrations, sizeof(calibrations));
  address += sizeof(calibrations); 
  }
  else {
    address = 0
  }
  delay(BNO055_SAMPLERATE_DELAY_MS);

}

struct coord applyEWMA(struct coord s0, double alpha, struct coord ewma){
  struct coord s1;
  s1.x = s0.x*alpha + (1-alpha)*ewma.x;
  s1.y = s0.y*alpha + (1-alpha)*ewma.y;
  s1.z = s0.z*alpha + (1-alpha)*ewma.z;
  return s1;
}
struct coord returnEvent(sensors_event_t* event) {
  double x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem
  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION) {
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_ROTATION_VECTOR) {
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_GRAVITY) {
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  // setting coordinate values of an event
  struct coord xyz;
  xyz.x = x;
  xyz.y = y;
  xyz.z = z;
  return xyz;
}

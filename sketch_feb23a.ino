#include <SPI.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_PCD8544.h>

//Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);

float ARef = 1.1;

void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   analogReference(EXTERNAL);
}

void loop() {
  // put your main code here, to run repeatedly:
   
  float raw = analogRead(A0);;
  float voltage = raw*(3.3/1023);
  //float voltage = x*ARef*1.1;
  Serial.print(" ");
  Serial.print(voltage);
  Serial.print(" V");
  //display.clearDisplay();
 
}

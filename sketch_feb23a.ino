void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}
// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  float out = voltage * 1.303;
  // print out the value you read:
  Serial.println(out);
  delay(1000);
}

void displayBatteryStatus(float volt_out) {
   if (volt_out >= 3.9) {
    // Battery voltage is between 3.9V and 4.2V - Display 3 rectangles
    Serial.println("[■■■] Battery Full");
  } else if (volt_out >= 3.5 && volt_out < 3.9) {
    // Battery voltage is between 3.5V and 3.9V - Display 2 rectangles
    Serial.println("[■■ ] Battery Medium");
  } else if (volt_out >= 3.4 && volt_out < 3.5) {
    // Battery voltage is between 3.4V and 3.5V - Display 1 rectangle
    Serial.println("[■  ] Battery Low");
  } else {
    // Battery voltage is below 3.4V - Display low battery message
    Serial.println("[   ] Low Battery - Charge Now!");
  }
}


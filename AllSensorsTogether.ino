/*Backwards violin - IDD 2016 */
 
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL); 
#endif

#include "Adafruit_LIS3DH.h"
#include "Adafruit_Sensor.h"

// analog sensors 

int forceSensorPin = A0;
int softPodPin = A1;
int valFS =0;
int valSP =0;
int accX=0, accY=0, accZ=0;

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();


void setup(void) {
  
  Serial.begin(9600);
  Serial.println("LIS3DH test!");
  
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
  
}

void loop() {
  //acce output
  lis.read();      // get X Y and Z data at once
  sensors_event_t event; 
  lis.getEvent(&event);
  accX = abs(event.acceleration.x)/7;
  accY = abs(event.acceleration.y)/7;
  accZ = abs(event.acceleration.z)/7;
  Serial.print("X: "); Serial.print(accX);
  Serial.print("\tY: "); Serial.print(accY); 
  Serial.print("\tZ: "); Serial.print(accZ); 
  Serial.print(" m/s^2 ");
  // analog sensors output
  valFS = analogRead(forceSensorPin)/32;
  valSP = analogRead(softPodPin)/580;
  Serial.print("Force: ");
  Serial.print(valFS); 
  Serial.print("\t Potentiometer: ");
  Serial.println(valSP); 
  delay(200); 
}

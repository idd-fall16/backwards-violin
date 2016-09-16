/******************
 * The Backwards Violin
 * control code for reading sensors and sending 
 * MIDI signals using the ardumidi library 
 * referenced by Bjoern's MIDI over Serial example
 */

// turn off cloud functionality
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL); 
#endif


#include "ardumidi.h"
#include "Adafruit_LIS3DH.h"
#include "Adafruit_Sensor.h"

#define OFF_THRESHOLD 5

#define SP_PIN A1
#define FS_PIN A0

#define CHANNEL 1


int old_pitch = -2;
int old_volume = -2;
int old_accel = -2;
int new_pitch = -1;
int new_volume = -1;
int new_accel = -1;
byte program = 1;

int i=0;

int accX=0, accY=0, accZ=0;

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();


void setup() {
  Serial.begin(115200);
  lis.begin(0x18);   // change this to 0x19 for alternative i2c address
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
}

/*
 * return reading in the range 0-7
 */
int read_from_softpot() {
  int valSP = analogRead(SP_PIN)/580;
//  Serial.print("\t Potentiometer: ");
//  Serial.println(valSP); 

 return i%8;
//  return valSP;
}

/*
 * return in the range 0-127
 */
int read_from_force() {
  int valFS = analogRead(FS_PIN)/32;
//  Serial.print("Force: ");
//  Serial.print(valFS); 

  return i%5 * 30;
//  return valFS;
}

/*
 * 
 */
void read_from_accel() {
  lis.read();      // get X Y and Z data at once
  sensors_event_t event; 
  lis.getEvent(&event);
  accX = abs(event.acceleration.x)/7;
  accY = abs(event.acceleration.y)/7;
  accZ = abs(event.acceleration.z)/7;
//  Serial.print("X: "); Serial.print(accX);
//  Serial.print("\tY: "); Serial.print(accY); 
//  Serial.print("\tZ: "); Serial.print(accZ); 
//  Serial.print(" m/s^2 ");


  accZ = i%2 * 11;
}


/*
 * turn reading into a midi note
 */
int softpot_to_pitch() {
  //read from sensor
  int sp = read_from_softpot();
  //bucket the resistance to a note on the octave
  int note;
  switch (sp) {
    case 0:
      note = MIDI_C;
      break;
    case 1:
      note = MIDI_D;
      break;
    case 2:
      note = MIDI_E;
      break;
    case 3:
      note = MIDI_F;
      break;
    case 4:
      note = MIDI_G;
      break;
    case 5:
      note = MIDI_A;
      break;
    case 6:
      note = MIDI_B;
      break;
    case 7:
      note = MIDI_C + MIDI_OCTAVE;
      break;
    default: 
      note = MIDI_C;
      break;
  }
 
  return note;
}

/*
 * turn reading into a valid midi volume signature
 */
int force_to_volume() {
  //read from sensor
  int force = read_from_force();
  if (force < OFF_THRESHOLD) {
    //turn it off
    return 0;
  } else {
    //if force is greater than low_thresh, return scaled positive number from 0 - 127
    return force;
  }    
}

int accel_to_indicator() {
  if (accZ < 9) {
    return -1;
  } else if (accZ > 10) {
    return 1;
  }
}

/*
 * check all relevant dimensions for change
 */
bool pitch_changed() {
  return new_pitch != old_pitch;
}

bool volume_changed() {
  return new_volume != old_volume;
}

bool accel_changed() { 
  return new_accel != old_accel;
}

/*
 * turn signals into midi commands
 * and send out to synth
 */
void send_midi_commands() {
  old_pitch = new_pitch;
  new_pitch = softpot_to_pitch();
    
  old_volume = new_volume;
  new_volume = force_to_volume(); 

  old_accel = new_accel;
  new_accel = accel_to_indicator();

  if (accel_changed()) {
    if (new_accel = -1) {
      program = 1;
    } else {
      program = 41;
    }
    midi_program_change(CHANNEL, program);
  }
  
  if (pitch_changed()) {
      midi_note_off(CHANNEL,new_pitch,127);
      midi_note_on(CHANNEL,new_pitch,new_volume);
  }

  if (volume_changed()) {
      //turn it off
      if (new_volume == 0) {
        midi_note_off(CHANNEL,old_pitch,127); 
        midi_note_off(CHANNEL,new_pitch,127); 
      } else {
        //adjust volume
        midi_note_off(CHANNEL,old_pitch,127); 
        midi_note_off(CHANNEL,new_pitch,127);
        midi_note_on(CHANNEL,new_pitch,new_volume);
      }
  }
}

void loop() {
  i++;
  send_midi_commands();
//  Serial.println("sent commands");
  delay(1000); //response sensitivity
}

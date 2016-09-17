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

#define POT_THRESH 10

int old_pitch = -2;
int old_volume = -2;
int old_accel = -2;
int new_pitch = -1;
int new_volume = -1;
int new_accel = -1;
byte program = 1;

int i=0;

int accX=0, accY=0, accZ=0;

int pot[POT_THRESH];
int pot_index = 0;

int pitch_bend = 8192;

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
  Serial.print("\t Potentiometer: ");
  Serial.println(valSP); 

// return i%8;
  return valSP;
}

/*
 * return in the range 0-127
 */
int read_from_force() {
  int valFS = analogRead(FS_PIN)/32;
  Serial.print("Force: ");
  Serial.println(valFS); 

//  return 127;
//  return i%5 * 30;
  return valFS;
}

/*
 * (0, 1)
 */
void read_from_accel() {
  lis.read();      // get X Y and Z data at once
  sensors_event_t event; 
  lis.getEvent(&event);
  accX = event.acceleration.x/7;
//  accY = abs(event.acceleration.y)/7;
//  accZ = abs(event.acceleration.z)/7;
//  Serial.print("X: "); Serial.print(accX);
//  Serial.print("\tY: "); Serial.print(accY); 
//  Serial.println("Z: "); Serial.println(accZ); 
//  Serial.print(" m/s^2 ");
//  accZ = i%2 * 11;
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
//  return MIDI_C;
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
  read_from_accel();
  return accX;
}

/*
 * check all relevant dimensions for change
 */
bool pitch_changed() {
  return new_pitch != old_pitch;
}

bool volume_changed() {
  return new_volume + 5 < old_volume || new_volume - 5 > old_volume;
}

bool accel_changed() { 
  return new_accel != old_accel;
}

/*
 * turn signals into midi commands
 * and send out to synth
 * 0 to 16,383
 */
void send_midi_commands() {
  if (i%10 == 0) {
    if (new_accel == 1) {
      pitch_bend = pitch_bend + 1000;
      if (pitch_bend > 8192) {
        pitch_bend = 8192;
      }
    } else if (new_accel == -1) {
      pitch_bend = pitch_bend - 1000;
      if (pitch_bend < 0) {
        pitch_bend = 0;
      }
    }
    midi_pitch_bend(CHANNEL, pitch_bend);
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

void debounce() {  
//  Serial.print("DEBOUNCE");
//  bool midi = false;

  bool consistent_pitch = true;
  
  int cur_pitch = softpot_to_pitch();
  pot[pot_index] = cur_pitch;
  pot_index = (pot_index + 1) % POT_THRESH;

  for (int i = 0; i < POT_THRESH; i++) {
    if (pot[i] != cur_pitch) {
//      Serial.print("FALSE");
      consistent_pitch = false;
    }
  }

  if (consistent_pitch) {
      old_pitch = new_pitch;
      new_pitch = softpot_to_pitch(); 
//      midi = true; 
  }

  old_volume = new_volume;
  new_volume = force_to_volume(); 

  old_accel = new_accel;
  new_accel = accel_to_indicator();

//  return midi;
}

void loop() {
  i++;
  debounce();
//  send_midi_commands();
  
  
  delay(10); //response sensitivity
}

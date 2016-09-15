/******************
 * MIDI over Serial Example
 * for RedBear Duo using a modified ardumidi library (included)
 * Sends MIDI messages over the Serial (USB) connection to a laptop
 * Demonstrates note on, note off, and a controller change
 * Bjoern Hartmann for IDD, Fall 2016
 */

#include "ardumidi.h"

// turn off cloud functionality
#if defined(ARDUINO) 
SYSTEM_MODE(MANUAL); 
#endif

#define CHANNEL 1
#define OFF_THRESHOLD 5

#define SP_PIN 4
#define FSR_PIN 5
#define ACCEL_PIN 6

int old_pitch = -2;
int old_volume = -2;
int new_pitch = -1;
int new_volume = -1;

int i=0;

void setup() {
  Serial.begin(115200);
//  pinMode(SP_PIN,INPUT);
//  pinMode(FSR_PIN,INPUT);
//  pinMode(ACCEL_PIN,INPUT);
}

int read_from_softpot() {
//  TODO
  return 60 + i%3 * 2;
}

int read_from_force() {
//  TODO
  return 127 / (i%5 + 1);
}

int softpot_to_pitch() {
  //read from sensor
  int sp = read_from_softpot();
  
  //bucket the resistance to a note on the octave
    //TODO: switch statement

  int note = sp;
  return note;
}

int force_to_volume() {
  //read from sensor
  int force = read_from_force();
  if (force < OFF_THRESHOLD) {
    //turn it off
    return 0;
  } else {
    //if force is greater than low_thresh, return scaled positive number from 0 - 127
    int scaled_force = force;
//    TODO: SCALE FORCE
    return scaled_force;
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

void send_midi_commands() {
  old_pitch = new_pitch;
  new_pitch = softpot_to_pitch();
    
  old_volume = new_volume;
  new_volume = force_to_volume();  
  
  if (pitch_changed()) {
      midi_note_off(CHANNEL,old_pitch,127);
      midi_note_on(CHANNEL,new_pitch,new_volume);
  }

  if (volume_changed()) {
      //turn it off
      if (new_volume == 0) {
        midi_note_off(CHANNEL,new_pitch,127); 
      } else {
        //adjust volume
        midi_note_off(CHANNEL,new_pitch,127);
        midi_note_on(CHANNEL,new_pitch,new_volume);
      }
  }
  delay(1000); //response sensitivity
}

void loop() {
  i++;
  send_midi_commands();
}

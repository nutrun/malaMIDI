#include <SdFat.h>
// https://github.com/MajicDesigns/MD_MIDIFile
#include <MD_MIDIFile.h>
#include <SoftwareSerial.h>

#define STEREO_MIDI_OUT_TX_PIN 8
#define DIN_MIDI_OUT_TX_PIN 9
#define SD_SELECT_PIN 10

unsigned short play = 0;
byte midiInByte;
SdFat SD;
MD_MIDIFile stereoMidiOutFile;
MD_MIDIFile dinMidiOutFile;
SoftwareSerial stereoMidiOutSerial = SoftwareSerial();
SoftwareSerial dinMidiOutSerial = SoftwareSerial();

void stereoMidiCallback(midi_event *midiEvent);
void dinMidiCallback(midi_event *midiEvent);
void midiSilence(void);

void setup() {
  int midiFileErr;

  // MIDI in
  Serial.begin(31250);
  while (!Serial);

  stereoMidiOutSerial.setTX(STEREO_MIDI_OUT_TX_PIN);
  stereoMidiOutSerial.begin(31250);
  while (!stereoMidiOutSerial);
  
  dinMidiOutSerial.setTX(DIN_MIDI_OUT_TX_PIN);
  dinMidiOutSerial.begin(31250);
  while (!dinMidiOutSerial);

  if (!SD.begin(SD_SELECT_PIN, SPI_HALF_SPEED)) {
    // TODO print error to display
    while (1);
  }

  stereoMidiOutFile.begin(&SD);
  stereoMidiOutFile.setMidiHandler(stereoMidiCallback);
  stereoMidiOutFile.setFilename("STEREOJACK/ARP.mid");
  midiFileErr = stereoMidiOutFile.load();

  if (midiFileErr != -1) {
    // TODO print error to display
    while (1);
  }

  dinMidiOutFile.begin(&SD);
  dinMidiOutFile.setMidiHandler(dinMidiCallback);
  dinMidiOutFile.setFilename("DIN/CHORDS.mid");
  midiFileErr = dinMidiOutFile.load();

  if (midiFileErr != -1) {
    // TODO print error to display
    while (1);
  }

  stereoMidiOutFile.looping(true);
  dinMidiOutFile.looping(true);
}

void loop() {
  if (Serial.available()) {
    midiInByte = Serial.read();

    if (midiInByte == 0xFA) { // Start
      stereoMidiOutFile.restart();
      dinMidiOutFile.restart();
      play = 1;
    } else if (midiInByte == 0xFC) { // Stop
      midiSilence();
      play = 0;
    }

    if (play == 1) {
      // Process 4 ticks on every incoming clock.
      if (midiInByte == 0xF8) { // clock
        if (!stereoMidiOutFile.isEOF()) {
          stereoMidiOutFile.processEvents(4);
        }

        if (!dinMidiOutFile.isEOF()) {
          dinMidiOutFile.processEvents(4);
        }
      }
    }
  }
}

void stereoMidiCallback(midi_event *e) {
  stereoMidiOutSerial.write(e->data[0] | e->channel);
  stereoMidiOutSerial.write(&e->data[1], e->size - 1);
}

void dinMidiCallback(midi_event *e) {
  dinMidiOutSerial.write(e->data[0] | e->channel);
  dinMidiOutSerial.write(&e->data[1], e->size - 1);
}

void midiSilence(void) {
  midi_event ev;
  ev.size = 0;
  ev.data[ev.size++] = 0xb0;
  ev.data[ev.size++] = 120;
  ev.data[ev.size++] = 0;

  for (ev.channel = 0; ev.channel < 16; ev.channel++) {
    stereoMidiCallback(&ev);
    dinMidiCallback(&ev);
  }
}

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
MD_MIDIFile SMF;

SoftwareSerial stereoMidiOutSerial = SoftwareSerial();
SoftwareSerial dinMidiOutSerial = SoftwareSerial();

void midiCallback(midi_event *midiEvent);
void midiSilence(void);

void setup() {
  int SMFerr;

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

  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setFilename("TEST.mid");
  SMFerr = SMF.load();

  if (SMFerr != -1) {
    // TODO print error to display
    while (1);
  }

  SMF.looping(true);
}

void loop() {
  if (Serial.available()) {
    midiInByte = Serial.read();

    if (midiInByte == 0xFA) { // Start
      SMF.restart();
      play = 1;
    } else if (midiInByte == 0xFC) { // Stop
      midiSilence();
      play = 0;
    }

    if (play == 1) {
      if (midiInByte == 0xF8) { // clock
        if (!SMF.isEOF()) {
          SMF.processEvents(1);
        }
      }
    }
  }
}

void midiCallback(midi_event *e) {
  stereoMidiOutSerial.write(e->data[0] | e->channel);
  stereoMidiOutSerial.write(&e->data[1], e->size - 1);
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
    midiCallback(&ev);
  }
}

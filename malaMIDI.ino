#include <SdFat.h>
// https://github.com/MajicDesigns/MD_MIDIFile
#include <MD_MIDIFile.h>
#include <SoftwareSerial.h>

#define MIDI_IN_RX_PIN 8
#define MIDI_OUT_TX_PIN 9
#define SD_SELECT_PIN 10

unsigned short play = 0;
byte midiInByte;

SdFat SD;
MD_MIDIFile SMF;

SoftwareSerial midiInSerial = SoftwareSerial();
SoftwareSerial midiOutSerial = SoftwareSerial();

void midiCallback(midi_event *midiEvent);
void midiSilence(void);

void setup() {
  int SMFerr;

  Serial.begin(9600);
  while (!Serial);

  midiOutSerial.setTX(MIDI_OUT_TX_PIN);
  midiOutSerial.begin(31250);
  while (!midiOutSerial);
  Serial.println(F("Initialized midiOutSerial."));

  midiInSerial.setRX(MIDI_IN_RX_PIN);
  midiInSerial.begin(31250);
  while (!midiInSerial);
  Serial.println(F("Initialized midiInSerial."));

  if (!SD.begin(SD_SELECT_PIN, SPI_HALF_SPEED)) {
    Serial.println(F("Failed to initialize SD card."));
    while (1);
  }

  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setFilename("TEST.mid");
  SMFerr = SMF.load();

  if (SMFerr != -1) {
    Serial.println(F("SMF load error."));
    Serial.println(SMFerr);
    while (1);
  }

  SMF.looping(true);
}

void loop() {
  if (midiInSerial.available()) {
    midiInByte = midiInSerial.read();

    if (midiInByte == 0xFA) { // Start
      Serial.println(F("Start"));
      midiInSerial.flush();
      SMF.restart();
      play = 1;
    } else if (midiInByte == 0xFC) { // Stop
      Serial.println(F("Stop"));
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
  midiOutSerial.write(e->data[0] | e->channel);
  midiOutSerial.write(&e->data[1], e->size - 1);
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

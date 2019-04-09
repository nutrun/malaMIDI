# malaMIDI

MIDI sequencer. Plays MIDI files from an SD card and syncs with a master device (e.g drum machine) via MIDI for start, stop and tempo. 

Depends on https://github.com/nutrun/ArduinoCore-avr/tree/master/libraries/SoftwareSerial for single pin SoftwareSerial communication.

Setting https://github.com/MajicDesigns/MD_MIDIFile/blob/ea736bd12c1226991caf11f4d61bacf4198c2d4f/src/MD_MIDIFile.h#L371 to `1` since only playing single track MIDI files.

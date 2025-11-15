#include <avr/pgmspace.h>
#include <MIDI.h>
#include <PS2KeyAdvanced.h>

/* Keyboard constants  Change to suit your Arduino
   define pins used for data and clock from keyboard */
#define DATAPIN 4
#define IRQPIN 3

#define RANGE_IDX_DELAY_0 0
#define RANGE_IDX_PHASER_1 1
#define RANGE_IDX_ARP_2 2
#define RANGE_IDX_PORT_3 3
#define RANGE_IDX_OSC1_4 4
#define RANGE_IDX_CUToFF_5 5
#define RANGE_IDX_LFODEPTH_6 6
#define RANGE_IDX_LFORATE_7 7

#define MAX_RANGES 9           // adjust as needed
byte rangeValues[MAX_RANGES];  // holds all internal 0–127 values

#define MAX_TOGGLES 4
bool toggleValues[MAX_TOGGLES];  // holds all 0-1 toggle values

#define MAX_PIANO_KEYS 38
bool togglePianoKeys[MAX_PIANO_KEYS];  // holds all piano pressed/notpressed values

enum KeyType {
  UNKNOWN = 0,
  SIMPLE,  // sends one CC on key press
  TOGGLE,
  RANGE,
  PIANO  // NoteOn on press, NoteOff on release
};

struct KeyBehavior {
  uint8_t type;        // KeyType
  uint8_t midiCode;    // SIMPLE/TOGGLE/RANGE: CC number; PIANO: note value
  uint8_t midiValue;   // SIMPLE: control value; otherwise unused
  uint8_t state;       // TOGGLE & PIANO only
  uint8_t increments;  // RANGE only
  uint8_t rangeIndex;  // RANGE only
};

struct KeyEntry {
  uint8_t keyCode;  // PS2 scan code
  KeyBehavior bh;   // Behavior
};

const KeyEntry keyMap[] PROGMEM = {
  // SIMPLE
  { PS2_KEY_F1, { SIMPLE, 0x06, 0x01, 0, 0, 0 } },
  { PS2_KEY_F2, { SIMPLE, 0x06, 0x08, 0, 0, 0 } },
  { PS2_KEY_F3, { SIMPLE, 0x06, 0x11, 0, 0, 0 } },
  { PS2_KEY_F4, { SIMPLE, 0x06, 0x1B, 0, 0, 0 } },
  { PS2_KEY_F5, { SIMPLE, 0x06, 0x25, 0, 0, 0 } },
  { PS2_KEY_F6, { SIMPLE, 0x06, 0x2F, 0, 0, 0 } },
  { PS2_KEY_F7, { SIMPLE, 0x06, 0x39, 0, 0, 0 } },
  { PS2_KEY_F8, { SIMPLE, 0x06, 0x43, 0, 0, 0 } },
  { PS2_KEY_F9, { SIMPLE, 0x06, 0x4A, 0, 0, 0 } },
  { PS2_KEY_F10, { SIMPLE, 0x06, 0x6C, 0, 0, 0 } },
  { PS2_KEY_F11, { SIMPLE, 0x06, 0x5A, 0, 0, 0 } },
  { PS2_KEY_F12, { SIMPLE, 0x06, 0x7E, 0, 0, 0 } },

  // ------------------------------------------------------------
  // PIANO keys (first line)
  // ------------------------------------------------------------
  { PS2_KEY_EUROPE2, { PIANO, 24, 0, 0, 0, 0 } },
  { PS2_KEY_A, { PIANO, 25, 0, 0, 0, 1 } },
  { PS2_KEY_Z, { PIANO, 26, 0, 0, 0, 2 } },
  { PS2_KEY_S, { PIANO, 27, 0, 0, 0, 3 } },
  { PS2_KEY_X, { PIANO, 28, 0, 0, 0, 4 } },
  { PS2_KEY_C, { PIANO, 29, 0, 0, 0, 5 } },
  { PS2_KEY_F, { PIANO, 30, 0, 0, 0, 6 } },
  { PS2_KEY_V, { PIANO, 31, 0, 0, 0, 7 } },
  { PS2_KEY_G, { PIANO, 32, 0, 0, 0, 8 } },
  { PS2_KEY_B, { PIANO, 33, 0, 0, 0, 9 } },
  { PS2_KEY_H, { PIANO, 34, 0, 0, 0, 10 } },
  { PS2_KEY_N, { PIANO, 35, 0, 0, 0, 11 } },
  { PS2_KEY_M, { PIANO, 36, 0, 0, 0, 12 } },
  { PS2_KEY_K, { PIANO, 37, 0, 0, 0, 13 } },
  { PS2_KEY_COMMA, { PIANO, 38, 0, 0, 0, 14 } },
  { PS2_KEY_L, { PIANO, 39, 0, 0, 0, 15 } },
  { PS2_KEY_DOT, { PIANO, 40, 0, 0, 0, 16 } },
  { PS2_KEY_DIV, { PIANO, 41, 0, 0, 0, 17 } },
  { PS2_KEY_APOS, { PIANO, 42, 0, 0, 0, 18 } },

  // ------------------------------------------------------------
  // PIANO keys (second line)
  // ------------------------------------------------------------
  { PS2_KEY_Q, { PIANO, 36, 0, 0, 0, 19 } },
  { PS2_KEY_2, { PIANO, 37, 0, 0, 0, 20 } },
  { PS2_KEY_W, { PIANO, 38, 0, 0, 0, 21 } },
  { PS2_KEY_3, { PIANO, 39, 0, 0, 0, 22 } },
  { PS2_KEY_E, { PIANO, 40, 0, 0, 0, 23 } },
  { PS2_KEY_R, { PIANO, 41, 0, 0, 0, 24 } },
  { PS2_KEY_5, { PIANO, 42, 0, 0, 0, 25 } },
  { PS2_KEY_T, { PIANO, 43, 0, 0, 0, 26 } },
  { PS2_KEY_6, { PIANO, 44, 0, 0, 0, 27 } },
  { PS2_KEY_Y, { PIANO, 45, 0, 0, 0, 28 } },
  { PS2_KEY_7, { PIANO, 46, 0, 0, 0, 29 } },
  { PS2_KEY_U, { PIANO, 47, 0, 0, 0, 30 } },
  { PS2_KEY_I, { PIANO, 48, 0, 0, 0, 31 } },
  { PS2_KEY_9, { PIANO, 49, 0, 0, 0, 32 } },
  { PS2_KEY_O, { PIANO, 50, 0, 0, 0, 33 } },
  { PS2_KEY_0, { PIANO, 51, 0, 0, 0, 34 } },
  { PS2_KEY_P, { PIANO, 52, 0, 0, 0, 35 } },
  { PS2_KEY_OPEN_SQ, { PIANO, 53, 0, 0, 0, 36 } },
  { PS2_KEY_EQUAL, { PIANO, 54, 0, 0, 0, 37 } },
  { PS2_KEY_CLOSE_SQ, { PIANO, 55, 0, 0, 0, 38 } },


  // ----- TOGGLE
  { PS2_KEY_SYSRQ, { TOGGLE, 13, 0, 0, 0, 0 } },
  { PS2_KEY_SCROLL, { TOGGLE, 26, 0, 0, 0, 1 } },
  { PS2_KEY_BREAK, { TOGGLE, 34, 0, 0, 0, 2 } },
  { PS2_KEY_SINGLE, { TOGGLE, 39, 0, 0, 0, 3 } },

  // ----- RANGE
  { PS2_KEY_INSERT, { RANGE, 14, 0, 0, 1, 0 } },
  { PS2_KEY_DELETE, { RANGE, 14, 0, 0, 0, 0 } },

  { PS2_KEY_HOME, { RANGE, 27, 0, 0, 1, 1 } },
  { PS2_KEY_END, { RANGE, 27, 0, 0, 0, 1 } },

  { PS2_KEY_PGUP, { RANGE, 35, 0, 0, 1, 2 } },
  { PS2_KEY_PGDN, { RANGE, 35, 0, 0, 0, 2 } },

  { PS2_KEY_UP_ARROW, { RANGE, 5, 0, 0, 1, 3 } },
  { PS2_KEY_DN_ARROW, { RANGE, 5, 0, 0, 0, 3 } },

  { PS2_KEY_R_ARROW, { RANGE, 11, 0, 0, 1, 4 } },
  { PS2_KEY_L_ARROW, { RANGE, 11, 0, 0, 0, 4 } },

  // ----- SIMPLE KP0–KP3
  { PS2_KEY_KP0, { SIMPLE, 68, 0x00, 0, 0, 0 } },
  { PS2_KEY_KP1, { SIMPLE, 68, 0x16, 0, 0, 0 } },
  { PS2_KEY_KP2, { SIMPLE, 68, 0x2b, 0, 0, 0 } },
  { PS2_KEY_KP3, { SIMPLE, 68, 0x40, 0, 0, 0 } },
  { PS2_KEY_KP_DOT, { SIMPLE, 68, 0x5a, 0, 0, 0 } },
  { PS2_KEY_KP_ENTER, { SIMPLE, 68, 0x73, 0, 0, 0 } },

  // ----- RANGE keypad
  { PS2_KEY_KP7, { RANGE, 74, 0, 0, 1, 5 } },
  { PS2_KEY_KP4, { RANGE, 74, 0, 0, 0, 5 } },

  { PS2_KEY_KP8, { RANGE, 72, 0, 0, 1, 6 } },
  { PS2_KEY_KP5, { RANGE, 72, 0, 0, 0, 6 } },

  { PS2_KEY_KP9, { RANGE, 66, 0, 0, 1, 7 } },
  { PS2_KEY_KP6, { RANGE, 66, 0, 0, 0, 7 } },

  { PS2_KEY_KP_MINUS, { RANGE, 71, 0, 0, 1, 8 } },
  { PS2_KEY_KP_PLUS, { RANGE, 71, 0, 0, 0, 8 } },
};

const uint16_t keyMapCount = sizeof(keyMap) / sizeof(keyMap[0]);

uint16_t c;

PS2KeyAdvanced keyboard;

// Create and bind the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();


void setup() {

  MIDI.begin();

  // Configure the keyboard library
  keyboard.begin(DATAPIN, IRQPIN);


  keyboard.typematic(0, 0);  // Get fastest
  //keyboard.typematic(31, 3);  // Get slowest
}

bool getKeyBehavior(uint8_t scanCode, KeyBehavior &out) {
  for (uint16_t i = 0; i < keyMapCount; i++) {
    uint8_t code = pgm_read_byte(&keyMap[i].keyCode);
    if (code == scanCode) {
      memcpy_P(&out, &keyMap[i].bh, sizeof(KeyBehavior));
      return true;
    }
  }
  return false;  // unmapped
}

void loop() {


  if (keyboard.available()) {

    // read the next key
    c = keyboard.read();

    uint16_t keycode = c & 0xFF;
    bool keyDown = !(c & PS2_BREAK);

    if (keycode < 0 || keycode > 255)
      return;

    //KeyBehavior &kb = keyMap[c];

    // KeyBehavior kb;
    // memcpy_P(&kb, &keyMap[keycode], sizeof(KeyBehavior));

    KeyBehavior kb;
    if (!getKeyBehavior(keycode, kb))
      return;


    if (kb.type == UNKNOWN)
      return;

    // ---- KEY RELEASE ----
    if (!keyDown) {
      if (kb.type == PIANO) {
        MIDI.sendNoteOff(kb.midiCode, 0, 1);

      }
    } else {

      if (kb.type == PIANO) {
        bool &pianoVal = togglePianoKeys[kb.rangeIndex];

        if (!pianoVal){
          MIDI.sendNoteOn(kb.midiCode, 127, 1);  // Send a Note (pitch 42, velo 127 on channel 1)
          pianoVal = !pianoVal;
        }
      } else if (kb.type == SIMPLE) {
        MIDI.sendControlChange(kb.midiCode, kb.midiValue, 1);

      } else if (kb.type == TOGGLE) {
        bool &toggleVal = toggleValues[kb.rangeIndex];

        if (!toggleVal)
          MIDI.sendControlChange(kb.midiCode, 127, 1);
        else
          MIDI.sendControlChange(kb.midiCode, 0, 1);
        toggleVal = !toggleVal;

      } else if (kb.type == RANGE) {
        byte &val = rangeValues[kb.rangeIndex];
        if (kb.increments && val < 127) val++;
        else if (!kb.increments && val > 0) val--;
        MIDI.sendControlChange(kb.midiCode, val, 1);
      }


      // // ---- KEY PRESS ----
      // switch (kb.type) {

      //   case 1:
      //     MIDI.sendControlChange(kb.midiCode, kb.midiValue, 1);
      //     break;

      //   case 2:
      //     bool &toggleVal = toggleValues[kb.rangeIndex];

      //     if (!toggleVal)
      //       MIDI.sendControlChange(kb.midiCode, 1, 1);
      //     else
      //       MIDI.sendControlChange(kb.midiCode, 0, 1);
      //     toggleVal = !toggleVal;
      //     break;

      //   case 3:
      //       byte &val = rangeValues[kb.rangeIndex];
      //       if (kb.increments && val < 127) val++;
      //       else if (!kb.increments && val > 0) val--;
      //       MIDI.sendControlChange(kb.midiCode, val, 1);
      //       break;

      //   case 4:
      //     //if (!kb.state) {
      //       MIDI.sendNoteOn(42, 127, 1);
      //       delay(500);
      //       //kb.state = true;
      //     //}


      //     break;
      // }
    }

    //               MIDI.sendNoteOn(42, 127, 1);    // Send a Note (pitch 42, velo 127 on channel 1)
    //         delay(500);		            // Wait for a second
    //         MIDI.sendNoteOff(42, 0, 1);     // Stop the note
    // delay(500);
  }

  delay(10);  // minimal debounce
}
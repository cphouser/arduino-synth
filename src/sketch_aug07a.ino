
#include <MozziGuts.h>
#include <mozzi_fixmath.h>
#include <Oscil.h>
#include <Phasor.h>
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>

#include <PS2KeyAdvanced.h>
#include <PS2KeyCode.h>
#include <PS2KeyTable.h>

#include <TM1637Display.h>

#include "voice.h"

#define DATAPIN 4
#define IRQPIN  3
#define AUDIOUT 9
#define POT1 0
#define POT2 5
#define POT3 2
#define POT4 1
#define POT5 3
#define POT6 4
#define BUTT_A 0
#define BUTT_B 1
#define BUTT_C 2
#define BUTT_D 3
#define BUTT_E 4
#define BUTT_F 5
#define BUTT_G 6
#define BUTT_H 7
#define BUTT_I 8
#define BUTT_J 9
#define BUTT_K 10
#define BUTT_L 11
#define BUTT_M 12
#define BUTT_N 13
#define BUTT_O 14
#define BUTT_P 15
#define BUTT_NONE 255
#define CLK 1
#define DIO 2
#define CONTROL_RATE 128
#define MAX_VOICES 5

static uint8_t buttonVal();
static void sampleButtons();
static void changeState(uint8_t new_press);
static uint8_t countKey(uint16_t key);
static void readKey();
static void voice_on(uint8_t key);
static void voice_off(uint8_t key);

uint8_t control_clock = 0;

uint8_t osc_table = 0;

uint8_t active_btn = BUTT_NONE;//last button pressed
uint8_t btn_sample = 0;

Voice voice_arr [MAX_VOICES];
uint8_t key_arr [MAX_VOICES];
uint8_t key_count = 0;
uint8_t last_voice_idx = MAX_VOICES;

#define PS2_TYPMAT_DISABLE 0xF8
PS2KeyAdvanced keyboard;
TM1637Display display(CLK, DIO);

bool display_switch = true;
//uint8_t pin_disp[4] = {0,0,0,0};

void setup()
{
  //Serial.begin(115200);
  pinMode(AUDIOUT, OUTPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  keyboard.begin(DATAPIN, IRQPIN); // Configure the keyboard library
  //keyboard.byteCommand(PS2_TYPMAT_DISABLE);
  //delay(10);
  //while (keyboard.read());
  display.setBrightness(0x0a);
  display.clear();
  startMozzi(CONTROL_RATE);
}

void updateControl(){
  //if (display.update()) {
  //  //print debug info to led display
  //  if (display_switch) {
  //    display_switch = false;
  //    display.showNumberDec(key_count, true);
  //    //display.showNumberHexEx(cases);
  //  } else {
  //    display_switch = true;
  //    display.showNumberDec(last_voice_idx);
  //    //display.showNumberHexEx(voice_arr[last_voice_idx].v_on);
  //    //display.setSegments(key_arr, 4);
  //  }
  //}
  switch (control_clock & 0x03) {
  case 0:
    if (!control_clock && (active_btn != BUTT_NONE)) {
      //register press if sample > 1/8*max (max: 1/4 control rate)
      if (btn_sample > 3) {
        changeState(active_btn);
      }
      btn_sample = 0;
      active_btn = BUTT_NONE;
    }
    break;
  case 1:
    readKey();
    break;
  case 2:
    sampleButtons();
    break;
  case 3:
    //updateVoices();
    while (!voice_arr[last_voice_idx].v_on && last_voice_idx < MAX_VOICES)
      last_voice_idx++;
    for (uint8_t i = last_voice_idx; i < MAX_VOICES; i++)
      voice_arr[i].update();
    break;
  }
  control_clock++;
}

int updateAudio(){
  int8_t out = 0;
  for (uint8_t i = last_voice_idx; i < MAX_VOICES; i++)
    out += voice_arr[i].next() >> 2;
  return out;
  //adds each of the oscillator values, returns the result, bitshifted twice to the right
}

void loop(){
  audioHook();
}

static void voice_on(uint8_t key) {
  uint8_t i = MAX_VOICES;
  uint16_t attack = (mozziAnalogRead(POT1) << 1) + 100;
  uint16_t decay = mozziAnalogRead(POT2) << 1;
  while (--i && voice_arr[i].v_on);
  last_voice_idx = (i < last_voice_idx) ? i : last_voice_idx;
  voice_arr[i].on(key, attack, decay); 
}

static void voice_off(uint8_t key) {
  uint8_t i = MAX_VOICES;
  do {
    if (voice_arr[--i].v_key == key)
      voice_arr[i].off();
  } while (i);
}

static void readKey() {
  uint16_t c = keyboard.read(); //read the next key
  if (c) {
    uint8_t key = countKey(c);
    if (key) {
      if (key >> 7)
        voice_off(key & 0x7F);
      else
        voice_on(key);
    }
  }
}

static uint8_t countKey(uint16_t c) {
  uint8_t key = c & 0xFF;
  // press or hold event
  if (c < 1000) {
    if (key_count) {
      uint8_t i = key_count;
      do {
        if (key_arr[--i] == key)
          return 0;
      } while (i);
    }
    key_arr[key_count] = key;
    key_count++;
    return key;
  }
  // release event
  else {
    uint8_t i = key_count;
    do {
      if (key_arr[--i] == key)
        key_arr[i] = key_arr[--key_count];
    } while (i);
    return key | 0x80;
  }
}

static void changeState(uint8_t new_press) {
  switch (new_press) {
  case BUTT_A:
    if (osc_table == 3)
      osc_table = 0;
    else
      osc_table++;
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setTable(osc_table);
    break;
  }
}

static void sampleButtons() {
  uint8_t cur_btn = buttonVal();
  if (cur_btn != BUTT_NONE) {
    if (active_btn == BUTT_NONE) {
      btn_sample = 0;
      active_btn = cur_btn;
    }
    else if (active_btn == cur_btn) {
      btn_sample++;
    }

  }
}

//returns any valid button press 0-15, 255 if multiple/none
static uint8_t buttonVal() {
  uint8_t b_reg = PINB;
  uint8_t d_reg = PIND;
  // state = 0b<pins 13-10><pins 8-5>
  uint8_t state = (((d_reg >> 5) | (b_reg << 3)) & 0x0F) |
    ((b_reg << 2) & 0xF0);
  switch (state){
    case 0:
      return BUTT_NONE;
    case 0x11:
      return BUTT_E;
    case 0x12:
      return BUTT_D;
    case 0x14:
      return BUTT_B;
    case 0x18:
      return BUTT_C;
    case 0x21:
      return BUTT_J;
    case 0x22:
      return BUTT_I;
    case 0x24:
      return BUTT_G;
    case 0x28:
      return BUTT_H;
    case 0x41:
      return BUTT_P;
    case 0x42:
      return BUTT_O;
    case 0x44:
      return BUTT_M;
    case 0x48:
      return BUTT_N;
    case 0x81:
      return BUTT_A;
    case 0x82:
      return BUTT_F;
    case 0x84:
      return BUTT_K;
    case 0x88:
      return BUTT_L;
    default://to avoid compiler warning
      return BUTT_NONE;
  }
}

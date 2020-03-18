
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

uint8_t buttonVal();
void sampleButtons();
void changeState(uint8_t new_press);

uint8_t control_clock = 0;

uint8_t osc_table = 0;

uint8_t active_btn = BUTT_NONE;//last button pressed
uint8_t btn_sample = 0;

//Voice voice_arr [5];
Voice voice_1(CONTROL_RATE>>2);
Voice voice_2(CONTROL_RATE>>2);
Voice voice_3(CONTROL_RATE>>2);
Voice voice_4(CONTROL_RATE>>2);
Voice voice_5(CONTROL_RATE>>2);

//uint16_t c;
PS2KeyAdvanced keyboard;
TM1637Display display(CLK, DIO);

bool display_switch = true;
uint8_t pin_disp[4] = {0,0,0,0};

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
  keyboard.begin( DATAPIN, IRQPIN ); // Configure the keyboard library
  display.setBrightness(0x0a);
  display.clear();
  startMozzi(CONTROL_RATE);
}

void updateControl(){
  if (display.update()) {
    //print debug info to led display
    if (display_switch) {
      display_switch = false;
      //display.showNumberDec(active_btn);
      //display.showNumberHexEx(buttonVal());
    } else {
      display_switch = true;
      display.setSegments(pin_disp, 4);
    }
  }
  switch (control_clock & 0x03) {
  case 0:
    if (!control_clock && (active_btn != BUTT_NONE)) {
      //register press if sample > 1/8*max (max: 1/4 control rate)
      if (btn_sample > 3) {
        //pin_disp[0] = active_btn;
        changeState(active_btn);
      }
      btn_sample = 0;
      active_btn = BUTT_NONE;
    }
    break;
  case 1:
    if (keyboard.available()) {//keyb read function
      uint16_t c = keyboard.read(); //read the next key
      if ( c > 0 ) {
        uint8_t key = c & 0xFF;
        if (c < 1000) {
          uint16_t attack = mozziAnalogRead(POT1) << 1;
          uint16_t decay = mozziAnalogRead(POT2) << 1;
          if ((voice_1.v_key == key)
              || (voice_2.v_key == key)
              || (voice_3.v_key == key)
              || (voice_4.v_key == key)
              || (voice_5.v_key == key)) {}
          //initialize a new key press
          else if (voice_1.v_key==0)
            voice_1.on(key, attack, decay);
          else if (voice_2.v_key==0)
            voice_2.on(key, attack, decay);
          else if (voice_3.v_key==0)
            voice_3.on(key, attack, decay);
          else if (voice_4.v_key==0)
            voice_4.on(key, attack, decay);
          else if (voice_5.v_key==0)
            voice_5.on(key, attack, decay);
        } else {//finish a key press
          if (key==voice_1.v_key)
            voice_1.off();
          if (key==voice_2.v_key)
            voice_2.off();
          if (key==voice_3.v_key)
            voice_3.off();
          if (key==voice_4.v_key)
            voice_4.off();
          if (key==voice_5.v_key)
            voice_5.off();
        }
      }
    }
    break;
  case 2:
    sampleButtons();
    break;
  case 3:
    voice_1.update();
    voice_2.update();
    voice_3.update();
    voice_4.update();
    voice_5.update();
  }
  control_clock++;
}

int updateAudio(){
  int8_t note1 = voice_1.next();
  int8_t note2 = voice_2.next();
  int8_t note3 = voice_3.next();
  int8_t note4 = voice_4.next();
  int8_t note5 = voice_5.next();
  return ((note1+note2+note3+note4+note5)>>2);
  //adds each of the oscillator values, returns the result, bitshifted twice to the right
}

void loop(){
  audioHook();
}

void changeState(uint8_t new_press) {
  switch (new_press) {
  case BUTT_A:
    if (osc_table == 3)
      osc_table = 0;
    else
      osc_table++;
    voice_1.setTable(osc_table);
    voice_2.setTable(osc_table);
    voice_3.setTable(osc_table);
    voice_4.setTable(osc_table);
    voice_5.setTable(osc_table);
    break;
  }
}

void sampleButtons() {
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
uint8_t buttonVal() {
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

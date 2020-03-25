
#include <MozziGuts.h>
#include <mozzi_fixmath.h>
//#include <StateVariable.h>
#include <AudioDelay.h>
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

#define AUDIOUT 9
#define CLK 1
#define DIO 2
#define CONTROL_RATE 128
#define MAX_VOICES 5

class ctlsig_state {
 public:
  uint8_t mix_sig;


  uint16_t atkLen()
  {
    return mozziAnalogRead(POT1) << 2;
  }

  uint16_t decLen()
  {
    return mozziAnalogRead(POT2) << 2;
  }

  void update()
  {
    //mix_state.m_delay.set(mozziAnalogRead(POT4)>>2);
    mix_sig = mozziAnalogRead(POT3)>>2;
  }
 private:
  enum ANALOG_IN {
    POT1 = 0,
    POT2 = 5,
    POT3 = 2,
    POT4 = 1,
    POT5 = 3,
    POT6 = 4,
  };


} ctlsig_state;

class voice_state {
 public:
  uint8_t osc_table = 0;
  uint8_t atk_curve = 0;//0  1    2    3      4
  uint8_t dec_curve = 0;//x  x^2  x^4  x^1/4  x^1/2

  void on(uint8_t key)
  {
    uint8_t i = MAX_VOICES;
    while (--i && voice_arr[i].playing());
    last_voice_idx = (i < last_voice_idx) ? i : last_voice_idx;

    voice_arr[i].on(key, ctlsig_state.atkLen(), ctlsig_state.decLen());
    voice_arr[i].setAtkCurve(atk_curve);
    voice_arr[i].setDecCurve(dec_curve);
  }

  void off(uint8_t key)
  {
    uint8_t i = MAX_VOICES;
    do {
      if (voice_arr[--i].v_key == key)
        voice_arr[i].off();
    } while (i);
  }

  uint8_t envNext()
  {
    uint16_t m_env_sum = 0;
    for (uint8_t i = last_voice_idx; i < MAX_VOICES; i++)
      m_env_sum += voice_arr[i].envNext();
    return m_env_sum >> 2;
  }

  void update()
  {
    while (!voice_arr[last_voice_idx].playing())
      ++last_voice_idx;
  }

  int8_t mix()
  {
    int16_t out = 0;
    for (uint8_t i = last_voice_idx; i < MAX_VOICES; i++)
      out += voice_arr[i].next();
    return (out >> 2);
  }

  void changeTable()
  {
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setTable(osc_table);
  }

 private:
  uint8_t last_voice_idx = MAX_VOICES;
  Voice voice_arr [MAX_VOICES];

} voice_state;

class mix_state {
 public:
  uint8_t del_wet = 0;

  int8_t next(int8_t dry_mix)
  {
    int8_t wet_mix = dry_mix & m_env;//m_delay.next(dry_out);
    return (((del_wet*wet_mix)>>8)
            + ((((255-del_wet)*dry_mix))>>8));
  }

  void envNext(uint8_t env_mix)
  {
    m_env = env_mix;
  }

  void update()
  {
    del_wet = ctlsig_state.mix_sig;
  }

 private:
  AudioDelay <512> m_delay;
  uint8_t m_env = 0;
  //StateVariable <LOWPASS> lpf;

} mix_state;

class key_state {
 public:
  void init()
  {
    keyboard.begin( DATAPIN, IRQPIN ); // Configure the keyboard library
  }

  void readKey()
  {
    uint16_t c = keyboard.read(); //read the next key
    if (c) {
      uint8_t key = countKey(c);
      if (key) {
        if (key >> 7)
          voice_state.off(key & 0x7F);
        else
          voice_state.on(key);
      }
    }
  }

 private:
  enum PS2_PINS {
                 IRQPIN = 3,
                 DATAPIN = 4
  };
  uint8_t key_arr [MAX_VOICES];
  uint8_t key_count = 0;
  PS2KeyAdvanced keyboard;

  uint8_t countKey(uint16_t c)
  {
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
} key_state;

class button_state {
 public:
  void init()
  {
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(10, INPUT);
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    pinMode(13, INPUT);
  }

  void update()
  {
    if (active_btn != BUTT_NONE) {
      //register press if sample > 1/8*max (max: 1/4 control rate)
      if (btn_sample > 3) {
        changeState(active_btn);
      }
      btn_sample = 0;
      active_btn = BUTT_NONE;
    }
  }

  void sample()
  {
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

 private:
  enum BUTTONS {
                BUTT_A = 0,
                BUTT_B = 1,
                BUTT_C = 2,
                BUTT_D = 3,
                BUTT_E = 4,
                BUTT_F = 5,
                BUTT_G = 6,
                BUTT_H = 7,
                BUTT_I = 8,
                BUTT_J = 9,
                BUTT_K = 10,
                BUTT_L = 11,
                BUTT_M = 12,
                BUTT_N = 13,
                BUTT_O = 14,
                BUTT_P = 15,
                BUTT_NONE = 255
  };
  uint8_t active_btn = BUTT_NONE;//last button pressed
  uint8_t btn_sample = 0;

  void changeState(uint8_t new_press)
  {
    switch (new_press) {
    case BUTT_A:
      if (voice_state.osc_table == 3)
        voice_state.osc_table = 0;
      else
        voice_state.osc_table++;
      voice_state.changeTable();
      break;
    case BUTT_L:
      if (voice_state.dec_curve == 4)
        voice_state.dec_curve = 0;
      else
        voice_state.dec_curve++;
      break;
    case BUTT_O:
      if (voice_state.atk_curve == 4)
        voice_state.atk_curve = 0;
      else
        voice_state.atk_curve++;
      break;
    }

  }

  //returns any valid button press 0-15, 255 if multiple/none
  uint8_t buttonVal()
  {
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
} button_state;

TM1637Display display(CLK, DIO);

bool display_switch = true;
//uint8_t pin_disp[4] = {0,0,0,0};

void setup()
{
  pinMode(AUDIOUT, OUTPUT);
  button_state.init();
  display.setBrightness(0x0a);
  display.clear();
  key_state.init();
  startMozzi(CONTROL_RATE);
}

void updateControl()
{
  static uint8_t control_clock;
  switch (control_clock % 4) {
  case 0:
    if (!control_clock) {
      button_state.update();
    }
    else {
      ctlsig_state.update();
      mix_state.update();
    }
    break;
  case 1:
    key_state.readKey();
    break;
  case 2:
    button_state.sample();
    break;
  case 3:
    voice_state.update();
    break;
  }
  if (display.update()) {
    //print debug info to led display
    if (display_switch) {
      display_switch = false;
      //display.showNumberDec(mozziAnalogRead(POT3)>>2);
      //display.showNumberHexEx(cases);
    } else {
      display_switch = true;
      //display.setSegments(key_arr, 4);
    }
  }
  mix_state.envNext(voice_state.envNext());
  control_clock++;
}

int updateAudio() {return mix_state.next(voice_state.mix());}

void loop() {audioHook();}


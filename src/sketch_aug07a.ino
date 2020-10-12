#include <MozziGuts.h>
#include <mozzi_fixmath.h>
#include <AudioDelay.h>
#include <Oscil.h>
#include <Phasor.h>
#include <IntMap.h>
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
 ctlsig_state() : atk_range(0, 1024, 21, 5000)
    , dec_range(0, 1024, 0, 5000) {}

  uint8_t bitmix_sig;
  uint8_t bitmix_mix;
  uint8_t delay_sig;
  uint8_t delay_mix;

  uint16_t atkLen()
  {return atk_range(mozziAnalogRead(POT1));}

  uint16_t decLen()
  {return dec_range(mozziAnalogRead(POT2));}

  void envNext(uint8_t env_mix)
  {env_sum = env_mix;}

  void update()
  {
    bitmix_mix = mozziAnalogRead(POT3)>>2;
    bitmix_sig = env_sum + (mozziAnalogRead(POT4)>>3);
    delay_mix = mozziAnalogRead(POT5)>>2;
    delay_sig = env_sum + (mozziAnalogRead(POT6)>>3);
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
  IntMap atk_range;
  IntMap dec_range;
  uint8_t env_sum;
  

} ctlsig_state;


class voice_state {
 public:

  void on(uint8_t key)
  {
    uint8_t i = MAX_VOICES;
    while (--i && voice_arr[i].playing());
    last_voice_idx = (i < last_voice_idx) ? i : last_voice_idx;

    //voice_arr[i].setAtkCurve(atk_curve);
    //voice_arr[i].setDecCurve(dec_curve);
    //set level after curve
    //voice_arr[i].setAtkLevel(atk_level);
    //voice_arr[i].setDecLevel(dec_level);
    voice_arr[i].on(key, ctlsig_state.atkLen(), ctlsig_state.decLen());
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
    for (uint8_t i = last_voice_idx; i < MAX_VOICES; i++)
      voice_arr[i].envUpdate();
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
    osc_table = (osc_table == 3) ? 0 : (osc_table + 1);
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setTable(osc_table);
  }

  void changeDecCurve()
  {
    if (dec_curve == 3)
      dec_curve = 1;
    else
      dec_curve++;
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setDecCurve(dec_curve);
  }

  void changeAtkCurve()
  {
    if (atk_curve == 3)
      atk_curve = 1;
    else
      atk_curve++;
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setAtkCurve(atk_curve);
  }

  void changeDecLevel()
  {
    dec_level = (dec_level == 4) ? 0 : (dec_level + 1);
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setDecLevel((16<<dec_level)-1);
  }

  void changeAtkLevel()
  {
    atk_level = (atk_level == 4) ? 0 : (atk_level + 1);
    for (uint8_t i = MAX_VOICES; --i; )
      voice_arr[i].setAtkLevel(255>>atk_level);
  }

 private:
  uint8_t last_voice_idx = MAX_VOICES;
  Voice voice_arr [MAX_VOICES];
  uint8_t osc_table = 0;
  int8_t atk_curve = 1;//1  2    3    -4      -2
  int8_t dec_curve = 1;//x  x^2  x^3  x^1/4  x^1/2
  uint8_t atk_level = 0;//255 127 63 31  15
  uint8_t dec_level = 0;//15  31  63 127 255
  //uint8_t env_cache = 0;

} voice_state;


class mix_state {
 public:
  int8_t next(int8_t dry_mix)
  {
    int8_t wet_mix = bitmix(dry_mix, ctlsig_state.bitmix_sig);
    int8_t dry_2 = mix(dry_mix, wet_mix, ctlsig_state.bitmix_mix);
    int8_t wet_2 = m_delay.next(dry_2, ctlsig_state.delay_sig);
    //return bitmix(dry_2, wet_2);
    return mix(dry_2, wet_2, ctlsig_state.delay_mix);
  }

  void changeBitmixOp()
  {bitmix_operation = (bitmix_operation == 4) ? 0 : (bitmix_operation + 1);}

  void init()
  {m_delay.set(256);}

 private:
  AudioDelay <256> m_delay;
  uint8_t bitmix_operation = 0;

  static int8_t mix(int8_t dry, int8_t wet, uint8_t mix_amt)
  {return ((mix_amt*wet)>>8) + (((255-mix_amt)*dry)>>8);}

  int8_t bitmix(int8_t dry, uint8_t signal) {
    switch (bitmix_operation) {
    case 0:
      return dry & signal;
    case 1:
      return dry & ~signal;
    case 2:
      return dry | signal;
    case 3:
      return dry | ~signal;
    case 4:
      return dry ^ signal;
    default: return 0;
    }
  }

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
      voice_state.changeTable();
      break;
    case BUTT_B:
      mix_state.changeBitmixOp();
      break;
    case BUTT_K:
      voice_state.changeDecLevel();
      break;
    case BUTT_L:
      voice_state.changeDecCurve();
      break;
    case BUTT_N:
      voice_state.changeAtkLevel();
      break;
    case BUTT_O:
      voice_state.changeAtkCurve();
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
  //Serial.begin(9600);
  //Serial.print("ok");
  pinMode(AUDIOUT, OUTPUT);
  button_state.init();
  display.setBrightness(0x0a);
  display.clear();
  key_state.init();
  mix_state.init();
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
      //mix_state.update();
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
  //ctlsig_state.envNext(voice_state.env_cache);
  control_clock++;
}


int updateAudio()
{
  ctlsig_state.envNext(voice_state.envNext());
  return mix_state.next(voice_state.mix());
}


void loop()
{audioHook();}


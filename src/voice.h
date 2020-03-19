//class for synth voices

#ifndef H_VOICE
#define H_VOICE

//#include <inttypes.h>
#include <Oscil.h>
#include <Ead.h>
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <mozzi_fixmath.h>
#include <Arduino.h>

#define ENV_RATE 32

class Voice {
 public:
  uint8_t v_on;
  uint8_t v_key;
  Q16n16 v_freq;
  uint8_t v_gain;

  Voice();
  void on(uint8_t key, int attack, int decay);
  void off();
  void setTable(int8_t tab_idx);
  int8_t next();
  void update();
 private:
  Oscil <2048, AUDIO_RATE> v_osc;
  Ead v_env;

  Q16n16 keyFreq(uint8_t key);

};

#endif

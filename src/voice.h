//class for synth voices

#ifndef H_VOICE
#define H_VOICE

//#include <inttypes.h>
#include <Oscil.h>
#include "ADSR.h"
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <mozzi_fixmath.h>

#include <Arduino.h>

#define ENV_RATE 128

class Voice {
 public:
  uint8_t v_key;
  Q16n16 v_freq;

  Voice();
  void on(uint8_t key, int attack, int decay);
  void off();
  void setTable(int8_t tab_idx);
  int8_t next();
  uint8_t envNext();
  bool playing();

  void setAtkCurve(uint8_t mode);
  void setDecCurve(uint8_t mode);
 private:
  Oscil <2048, AUDIO_RATE> v_osc;
  ADSR <ENV_RATE, ENV_RATE> v_env;
  uint8_t v_env_cache;
  uint8_t v_env_atk_curve;
  uint8_t v_env_dec_curve;

  Q16n16 keyFreq(uint8_t key);
};

#endif

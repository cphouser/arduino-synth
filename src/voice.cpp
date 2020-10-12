
//#include <inttypes.h>
#include <Oscil.h>
#include "CurvyADSR.h"
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>
#include <mozzi_fixmath.h>

#include "voice.h"
#include <Arduino.h>

const int8_t* wav_table[] = {SIN2048_DATA,
                            SAW2048_DATA,
                            TRIANGLE2048_DATA,
                            SQUARE_NO_ALIAS_2048_DATA};

Voice::Voice()
{
  v_freq = 0;
  v_key = 0;
  //v_env_cache = 0;
  v_osc.setTable(wav_table[0]);
  v_env_atk_curve = 1;
  v_env_dec_curve = 1;
}

void Voice::on(uint8_t key, int attack, int decay)
{
  v_key = key;
  v_freq = keyFreq(key);
  v_osc.setFreq_Q16n16(v_freq);
  v_env.setTimes(attack,decay,65535, 50);
  v_env.setADCurves(v_env_atk_level, v_env_atk_curve,
                    v_env_dec_level, v_env_dec_curve);
  //v_env_cache = 0;
  v_env.noteOn();
}

void Voice::off()
{
  v_freq = 0;
  v_key = 0;
  v_env.noteOff();
}

void Voice::envUpdate() {
  v_env.update();
}

uint8_t Voice::envNext()
{
  //v_env_cache = v_env.next();
  return v_env.next();
}

void Voice::setAtkCurve(uint8_t mode)
{v_env_atk_curve = mode;}

void Voice::setDecCurve(uint8_t mode)
{v_env_dec_curve = mode;}

void Voice::setDecLevel(uint8_t level)
{v_env_dec_level = level;}

void Voice::setAtkLevel(uint8_t level)
{v_env_atk_level = level;}

void Voice::setTable(int8_t tab_idx)
{v_osc.setTable(wav_table[tab_idx]);}

int8_t Voice::next()
{
  int8_t dry_out = ((v_env.next()*v_osc.next())>>8);
  return dry_out;
}

bool Voice::playing()
{return v_env.playing();}

Q16n16 Voice::keyFreq(uint8_t key)
{
  switch(key & 0xFF) {
  case 0x5E: return ((Q16n16) 17146184); //C1
  case 0x3B: return ((Q16n16) 18165268); //C1#
  case 0x4B: return ((Q16n16) 19245302); //D1
  case 0x5F: return ((Q16n16) 20390216); //D1#
  case 0x38: return ((Q16n16) 21602632); //E1
  case 0x49: return ((Q16n16) 22887137); //F1
  case 0x67: return ((Q16n16) 24247665); //F1#
  case 0x4D: return ((Q16n16) 25690112); //G1
  case 0x4A: return ((Q16n16) 27217101); //G1#
  case 0x36: return ((Q16n16) 28835840); //A1(440)
  case 0x37: return ((Q16n16) 30550262); //A1#
  case 0x55: return ((Q16n16) 32366920); //B1
  case 0x5D: return ((Q16n16) 34291712); //C2
  case 0x56: return ((Q16n16) 36331192); //C2#
  case 0x46: return ((Q16n16) 38491259); //D2
  case 0x35: return ((Q16n16) 40779776); //D2#
  case 0x34: return ((Q16n16) 43204608); //E2
  case 0x52: return ((Q16n16) 45774275); //F2
  case 0x59: return ((Q16n16) 48495985); //F2#
  case 0x43: return ((Q16n16) 51379569); //G2
  case 0x44: return ((Q16n16) 54434857); //G2#
  case 0x62: return ((Q16n16) 57671680); //A2
  case 0x33: return ((Q16n16) 61101179); //A2#
  case 0x45: return ((Q16n16) 64734495); //B2
  case 0x54: return ((Q16n16) 68583424); //C3
  case 0x58: return ((Q16n16) 72661729); //C3#
  case 0x53: return ((Q16n16) 76982518); //D3
  case 0x61: return ((Q16n16) 81560207); //D3#
  case 0x32: return ((Q16n16) 86409871); //E3
  case 0x57: return ((Q16n16) 91547894); //F3
  case 0x63: return ((Q16n16) 96991969); //F3#
  case 0x5A: return ((Q16n16) 102759137); //G3
  case 0x41: return ((Q16n16) 108869714); //G3#
  case 0x40: return ((Q16n16) 115343360); //A3
  case 0x31: return ((Q16n16) 122202358); //A3#
  case 0x51: return ((Q16n16) 129468334); //B3
  case 0x1D: return ((Q16n16) 137166848); //C4
  default://to avoid compiler warning
    return 0;
  }
}

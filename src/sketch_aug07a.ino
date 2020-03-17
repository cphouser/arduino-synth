#include <TM1637Display.h>


//#include <ADSR.h>
//#include <AudioConfigHiSpeed14bitPwm.h>
//#include <AudioConfigStandard9bitPwm.h>
//#include <AudioConfigStandardPlus.h>
//#include <AudioConfigTeensy3_12bit.h>
//#include <AudioDelay.h>
//#include <AudioDelayFeedback.h>
//#include <AutoMap.h>
//#include <AutoRange.h>
//#include <CapPoll.h>
//#include <CircularBuffer.h>
//#include <cogl_sqrti.h>
//#include <ControlDelay.h>
//#include <DCfilter.h>
//#include <Ead.h>
//#include <EventDelay.h>
//#include <IntMap.h>
//#include <Line.h>
//#include <LowPassFilter.h>
//#include <meta.h>
//#include <Metronome.h>
#include <MozziGuts.h>
//#include <mozzi_analog.h>
//#include <mozzi_config.h>
#include <mozzi_fixmath.h>
//#include <mozzi_midi.h>
//#include <mozzi_rand.h>
//#include <mozzi_utils.h>
//#include <mult16x16.h>
//#include <mult16x8.h>
//#include <mult32x16.h>
#include <Oscil.h>
//#include <OverSample.h>
//#include <PDResonant.h>
#include <Phasor.h>
//#include <Portamento.h>
//#include <primes.h>
//#include <RCpoll.h>
//#include <ReverbTank.h>
//#include <RollingAverage.h>
//#include <RollingStat.h>
//#include <Sample.h>
//#include <SampleHuffman.h>
//#include <Smooth.h>
//#include <Stack.h>
//#include <StateVariable.h>
//#include <twi_nonblock.h>
//#include <WavePacket.h>
//#include <WavePacketSample.h>
//#include <WaveShaper.h>
#include <tables/sin2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include <tables/square_no_alias_2048_int8.h>

#include <PS2KeyAdvanced.h>
#include <PS2KeyCode.h>
#include <PS2KeyTable.h>

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
#define BUTT_NONE -1
#define CLK 1
#define DIO 2
#define CONTROL_RATE 128

Q16n16 note(uint8_t);
char buttonVal();
void buttonsManager(char);

        //Cycle Bounds:3,0,1,1,1, 0,0,1,1,1, 3,1,1, 3,1,1
        //Button Label:A,B,C,D,E, F,G,H,I,J, K,L,M, N,O,P
uint8_t buttons[16] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0};
uint8_t activeButton = BUTT_NONE;//last button pressed
bool button_q = false;
char pollTimer = 0;
byte activePoll = 0;//countdown timer 64-0 (1seconds) till last button can be pressed again
byte noisePoll = 0;
bool display_switch = true;

//Voice voice_arr [5];
Voice voice_1(CONTROL_RATE);
Voice voice_2(CONTROL_RATE);
Voice voice_3(CONTROL_RATE);
Voice voice_4(CONTROL_RATE);
Voice voice_5(CONTROL_RATE);

uint16_t c;
PS2KeyAdvanced keyboard;
TM1637Display display(CLK, DIO);


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
    if (display_switch) {
      display.showNumberDec(mozziAnalogRead(POT6));
    } else {
      display.setSegments(buttons, 4);
    }
  }
  if (keyboard.available()) {//keyb read function
    c = keyboard.read(); //read the next key
    if ( c > 0 ) {
      uint8_t key = c & 0xFF;
      //Q16n16 note_val = note(c & 0xFF);
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
      } else {
        if (key==voice_1.v_key)//finish a key press
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
  voice_1.update();
  voice_2.update();
  voice_3.update();
  voice_4.update();
  voice_5.update();
  buttonsManager(buttonVal());
  if (button_q) {
    voice_1.setTable(buttons[BUTT_A]);
    voice_2.setTable(buttons[BUTT_A]);
    voice_3.setTable(buttons[BUTT_A]);
    voice_4.setTable(buttons[BUTT_A]);
    voice_5.setTable(buttons[BUTT_A]);
    button_q = false;
  }
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

char buttonVal() {//returns any valid button press 0-15, -1 if multiple/none
  char state = 0;
  for (int i=5; i<=8; i++){
    if (digitalRead(i) && state==0){
      state = i-4;
      for (int j=10; j<=13; j++){
        if (digitalRead(j) && state<=4){
          state = state+4*(j-9);
        } else if (digitalRead(j) && state>4){
          return BUTT_NONE;
        }
      }
    }else if (digitalRead(i) && state>0){
      return BUTT_NONE;
    }
  }
  switch (state){
    case 0:
      return BUTT_NONE;
    case 5:
      return BUTT_E;
    case 6:
      return BUTT_D;
    case 7:
      return BUTT_B;
    case 8:
      return BUTT_C;
    case 9:
      return BUTT_J;
    case 10:
      return BUTT_I;
    case 11:
      return BUTT_G;
    case 12:
      return BUTT_H;
    case 13:
      return BUTT_M;
    case 14:
      return BUTT_L;
    case 15:
      return BUTT_P;
    case 16:
      return BUTT_K;
    case 17:
      return BUTT_A;
    case 18:
      return BUTT_F;
    case 19:
      return BUTT_N;
    case 20:
      return BUTT_O;
    default://to avoid compiler warning
      return 0;
  }
}

void buttonsManager(char pressed){
  if (pollTimer == 0) {//do nothing
    if (pressed == BUTT_NONE) {//release or multiple presses
    } else {
      activeButton = pressed;
      pollTimer = 64;
    }
    return;
  } if (pollTimer > 1) {
      if (pressed == BUTT_NONE) {
        pollTimer--;
      } else if (activeButton == pressed) {
        activePoll++;
        pollTimer--;
      } else {
        noisePoll++;
        pollTimer--;
      }
  } else {
    if (activePoll > noisePoll) {
      switch (activeButton){//cycling actions
        case BUTT_A://osctog
          if (buttons[BUTT_A] < 3){
            buttons[BUTT_A]++;
          } else {
            buttons[BUTT_A]=0;
          }
          button_q = true;
          break;
        case BUTT_B:
          break;
        case BUTT_C://hipass invert attack/decay
          if (buttons[BUTT_C] == 1) {
            buttons[BUTT_C] = 0;
          } else {
            buttons[BUTT_C] = 1;
          }
          break;
        case BUTT_D://effect 1 attack toggle
          if (buttons[BUTT_D] == 1) {
            buttons[BUTT_D] = 0;
          } else {
            buttons[BUTT_D] = 1;
          }
          break;
        case BUTT_E://effect 2 attack toggle
          if (buttons[BUTT_E] == 1) {
            buttons[BUTT_E] = 0;
          } else {
            buttons[BUTT_E] = 1;
          }
          break;
        case BUTT_F:
          break;
        case BUTT_G:
          break;
        case BUTT_H://lopass invert attack/decay
          if (buttons[BUTT_H] == 1) {
            buttons[BUTT_H] = 0;
          } else {
            buttons[BUTT_H] = 1;
          }
          break;
        case BUTT_I://effect 1 decay toggle
          if (buttons[BUTT_I] == 1) {
            buttons[BUTT_I] = 0;
          } else {
            buttons[BUTT_I] = 1;
          }
          break;
        case BUTT_J://effect 2 decay toggle
          if (buttons[BUTT_J] == 1) {
            buttons[BUTT_J] = 0;
          } else {
            buttons[BUTT_J] = 1;
          }
          break;
        case BUTT_K://attack function cycle
          if (buttons[BUTT_K] < 3){
            buttons[BUTT_K]++;
          } else {
            buttons[BUTT_K]=0;
          }
          break;
        case BUTT_L://hipass attack toggle
          if (buttons[BUTT_L] == 1) {
            buttons[BUTT_L] = 0;
          } else {
            buttons[BUTT_L] = 1;
          }
          break;
        case BUTT_M://lopass attack toggle
          if (buttons[BUTT_M] == 1) {
            buttons[BUTT_M] = 0;
          } else {
            buttons[BUTT_M] = 1;
          }
          break;
        case BUTT_N://decay function cycle
          if (buttons[BUTT_N] < 3){
            buttons[BUTT_N]++;
          } else {
            buttons[BUTT_N]=0;
          }
          break;
        case BUTT_O://hipass decay toggle
          if (buttons[BUTT_O] == 1) {
            buttons[BUTT_O] = 0;
          } else {
            buttons[BUTT_O] = 1;
          }
          break;
        case BUTT_P://lopass decay toggle
          if (buttons[BUTT_P] == 1) {
            buttons[BUTT_P] = 0;
          } else {
            buttons[BUTT_P] = 1;
          }
          break;
      }
    }
    pollTimer = 0;
    noisePoll = 0;
    activePoll = 0;
    activeButton = BUTT_NONE;
  }
}


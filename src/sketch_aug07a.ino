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

#define DATAPIN 4
#define IRQPIN  3
#define AUDIOUT 9
#define POT1 0
#define POT2 1
#define POT3 2
#define POT4 3
#define POT5 4
#define POT6 5
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

struct Note {
  Q16n16 noteFreq = 0;
//  NoteEnvelope atkDec = new NoteEnvelope(CONTROL_RATE);
};

Q16n16 note(uint16_t);
char buttonVal();
void buttonsManager(char);

Oscil <2048, AUDIO_RATE> osc1;
Oscil <2048, AUDIO_RATE> osc2;
Oscil <2048, AUDIO_RATE> osc3;
Oscil <2048, AUDIO_RATE> osc4;
Oscil <2048, AUDIO_RATE> osc5;

const int8_t* waveTables[4] = {SIN2048_DATA, SAW2048_DATA, TRIANGLE2048_DATA, SQUARE_NO_ALIAS_2048_DATA};

        //Cycle Bounds:3,0,1,1,1, 0,0,1,1,1, 3,1,1, 3,1,1
        //Button Label:A,B,C,D,E, F,G,H,I,J, K,L,M, N,O,P
uint8_t buttons[16] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0};
uint8_t activeButton = BUTT_NONE;//last button pressed
bool button_q = false;
char pollTimer = 0;
byte activePoll = 0;//countdown timer 64-0 (1seconds) till last button can be pressed again
byte noisePoll = 0;
uint8_t display_btn;
Note noteOne;
Note noteTwo;
Note noteThree;
Note noteFour;
Note noteFive;

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
  osc1.setTable(SIN2048_DATA);
  osc2.setTable(SIN2048_DATA);
  osc3.setTable(SIN2048_DATA);
  osc4.setTable(SIN2048_DATA);
  osc5.setTable(SIN2048_DATA);
}

void updateControl(){
  osc1.setFreq_Q16n16(noteOne.noteFreq);
  osc2.setFreq_Q16n16(noteTwo.noteFreq);
  osc3.setFreq_Q16n16(noteThree.noteFreq);
  osc4.setFreq_Q16n16(noteFour.noteFreq);
  osc5.setFreq_Q16n16(noteFive.noteFreq);
  //noteOne.atkDec.nextVal();
  //noteTwo.atkDec.nextVal();
  //noteThree.atkDec.nextVal();
  //noteFour.atkDec.nextVal();
  //noteFive.atkDec.nextVal();

  if (display.update()) {
    if (activeButton != display_btn) {
      display_btn = activeButton;
      display.showNumberDec(display_btn, true);
    } else {
      display.setSegments(buttons, 4);
    }
  }
  if( keyboard.available() ) {//keyb read function
    c = keyboard.read(); // read the next key
    if( c > 0 ) {
      Q16n16 note_val = note(c);
      if(c < 1000) {
        if (note_val == noteOne.noteFreq
        || note_val == noteTwo.noteFreq
        || note_val == noteThree.noteFreq
        || note_val == noteFour.noteFreq
        || note_val == noteFive.noteFreq){

        } else if(noteOne.noteFreq==0){//initialize a new key press
        noteOne.noteFreq = note_val;
          //noteOne.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteTwo.noteFreq==0){
          noteTwo.noteFreq = note_val;
          //noteTwo.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteThree.noteFreq==0){
          noteThree.noteFreq = note_val;
          //noteThree.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteFour.noteFreq==0){
          noteFour.noteFreq = note_val;
          //noteFour.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteFive.noteFreq==0){
          noteFive.noteFreq = note_val;
          //noteFive.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        }
      } else {
        if(note_val==noteOne.noteFreq){//finish a key press
          noteOne.noteFreq = 0;
        } if(note_val==noteTwo.noteFreq){
          noteTwo.noteFreq = 0;
        } if(note_val==noteThree.noteFreq){
          noteThree.noteFreq = 0;
        } if(note_val==noteFour.noteFreq){
          noteFour.noteFreq = 0;
        } if(note_val==noteFive.noteFreq){
          noteFive.noteFreq = 0;
        }
      }
    }
  }
  buttonsManager(buttonVal());
  if (button_q) {
    osc1.setTable(waveTables[buttons[BUTT_A]]);
    osc2.setTable(waveTables[buttons[BUTT_A]]);
    osc3.setTable(waveTables[buttons[BUTT_A]]);
    osc4.setTable(waveTables[buttons[BUTT_A]]);
    osc5.setTable(waveTables[buttons[BUTT_A]]);
    button_q = false;
  }
}

int updateAudio(){
  int8_t note1 = osc1.next();
  int8_t note2 = osc2.next();
  int8_t note3 = osc3.next();
  int8_t note4 = osc4.next();
  int8_t note5 = osc5.next();
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
    if (activePoll/noisePoll > 0) {
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

Q16n16 note(uint16_t val) {
  //int noteCode = 0;
  //if(val > 999) {
  //  noteCode = 1000;
  //}
  switch(val & 0xFF){
  case 0x5E: return ((Q16n16) 17146184); //C1
  case 0x3B: return ((Q16n16) 18165268); //C1#
  case 0x4B: return ((Q16n16) 19245302); //D1
  case 0x5F: return ((Q16n16) 20390216); //D1#
  case 0x38: return ((Q16n16) 21602632); //E1
  case 0x49: return ((Q16n16) 22887137); //F1
  case 0x67: return ((Q16n16) 24247665); //F1#
  case 0x4D: return ((Q16n16) 25690112); //G1
  case 0x4A: return ((Q16n16) 27217101); //G1#
  case 0x36: return ((Q16n16) 28835840); //A1
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

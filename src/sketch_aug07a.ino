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
//#include <tables/logistic2048_int8.h>
//#include <tables/cyc2048_int8.h>

#include <PS2KeyAdvanced.h>
#include <PS2KeyCode.h>
#include <PS2KeyTable.h>
//#include "NoteEnvelope.h"

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

struct Note {
  float noteFreq = 0;
//  NoteEnvelope atkDec = new NoteEnvelope(CONTROL_RATE);
};


int note(uint16_t);
float findFreq(int);
char buttonVal();
void buttonsManager(char);

Oscil <2048, AUDIO_RATE> osc1[] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA), Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA),
  Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc2[] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA), Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA),
  Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc3[] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA), Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA),
  Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc4[] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA), Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA),
  Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc5[] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA), Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA),
  Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA)
};

//int8_t* waveTables[] = {"SIN2048_DATA","SAW2048_DATA","TRIANGLE2048_DATA","SQUARE_NO_ALIAS_2048_DATA"};
//float oneFreq = 0;
//float twoFreq = 0;
//float thrFreq = 0;
//float fouFreq = 0;
//float fivFreq = 0;

//int buttonTiming = 0;
  //Cycle Bounds:4,0,1,1,1, 0,0,1,1,1, 3,1,1, 3,1,1
  //Button Label:A,B,C,D,E, F,G,H,I,J, K,L,M, N,O,P
unsigned char buttons[] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0};
char activeButton = BUTT_NONE;//last button pressed
char pollTimer = 0;
char activePoll = 0;//countdown timer 64-0 (1seconds) till last button can be pressed again
char noisePoll = 0;
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
  startMozzi(CONTROL_RATE);
  //osc.setFreq(one_freq);
  //noteOne.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
  //noteTwo.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
  //noteThree.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
  //noteFour.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
  //noteFive.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
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
  //display.
  //Serial.begin(9600);
  display.setBrightness(0x0a);
  display.showNumberDec(1,true);
  /*
  #if defined(ARDUINO_ARCH_AVR)
    Serial.println( F( "PS2 Advanced Key Simple Test:" ) );
  #elif defined(ARDUINO_ARCH_SAM)
    Serial.println( "PS2 Advanced Key Simple Test:" );
  #endif
  */
}

void updateControl(){
  osc1[buttons[BUTT_A]].setFreq(noteOne.noteFreq);
  osc2[buttons[BUTT_A]].setFreq(noteTwo.noteFreq);
  osc3[buttons[BUTT_A]].setFreq(noteThree.noteFreq);
  osc4[buttons[BUTT_A]].setFreq(noteFour.noteFreq);
  osc5[buttons[BUTT_A]].setFreq(noteFive.noteFreq);
  //noteOne.atkDec.nextVal();
  //noteTwo.atkDec.nextVal();
  //noteThree.atkDec.nextVal();
  //noteFour.atkDec.nextVal();
  //noteFive.atkDec.nextVal();
  
  if( keyboard.available() ) {//keyb read function
  c = keyboard.read(); // read the next key

  if( c > 0 ) {
      int note_val = note(c);//note_val: 4 digits, 0xxx vs 1xxx --> key is on or off (key == xxx)
      //Serial.print(" ");
      //Serial.print(note_val);
      if(note_val < 1000) {
        if (findFreq(note_val) == noteOne.noteFreq 
        || findFreq(note_val) == noteTwo.noteFreq 
        || findFreq(note_val) == noteThree.noteFreq 
        || findFreq(note_val) == noteFour.noteFreq
        || findFreq(note_val) == noteFive.noteFreq){

        } else if(noteOne.noteFreq==0){//initialize a new key press
          noteOne.noteFreq = findFreq(note_val);
          //noteOne.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteTwo.noteFreq==0){
          noteTwo.noteFreq = findFreq(note_val);
          //noteTwo.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteThree.noteFreq==0){
          noteThree.noteFreq = findFreq(note_val);
          //noteThree.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteFour.noteFreq==0){
          noteFour.noteFreq = findFreq(note_val);
          //noteFour.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        } else if(noteFive.noteFreq==0){
          noteFive.noteFreq = findFreq(note_val);
          //noteFive.atkDec.instantiateEnv(buttons[BUTT_K],buttons[BUTT_N]);
        }
      } if(findFreq(note_val - 1000)==noteOne.noteFreq){//finish a key press
        noteOne.noteFreq = 0;
      } if(findFreq(note_val - 1000)==noteTwo.noteFreq){
        noteTwo.noteFreq = 0;
      } if(findFreq(note_val - 1000)==noteThree.noteFreq){
        noteThree.noteFreq = 0;
      } if(findFreq(note_val - 1000)==noteFour.noteFreq){
        noteFour.noteFreq = 0;
      } if(findFreq(note_val - 1000)==noteFive.noteFreq){
        noteFive.noteFreq = 0;
      }
    }
  }
  buttonsManager(buttonVal());
  /*
  Serial.print("\n");
  Serial.print(pollTimer);
  Serial.print("-");
  Serial.print(noisePoll);
  Serial.print("-");
  Serial.print(activePoll);
  Serial.print("-");
  Serial.print(buttonVal());
  
  
  for(int i=0;i<=15;i++){
    if(buttons[i]!=0){
      int j = buttons[i];
      Serial.print(" ");
      Serial.print(i);
      Serial.print("-");
      Serial.print(j);
      Serial.print(" ");
      //display.showNumberDec(i);
    }
  }
  
  //Serial.print(oneFreq);
  
  Serial.print(" ");
  Serial.print(noteOne.atkDec.currentVal());
  //Serial.print(" ");
  //Serial.print(fouFreq);
  //Serial.print(" ");
  //Serial.print(fivFreq);
  
  Serial.print("\n");
  */
  //Serial.print(mozziAnalogRead(POT1));
  //display.showNumberDec(POT1,true);
}

int updateAudio(){
    int note1 = osc1[buttons[BUTT_A]].next();
    //int envl1 = 
    //int note2 = osc2[buttons[BUTT_A]].next();
    //int note3 = osc3[buttons[BUTT_A]].next();
    //int note4 = osc4[buttons[BUTT_A]].next();
    //int note5 = osc5[buttons[BUTT_A]].next();
    return note1;
  //return ((note1+note2+note3+note4+note5)>>2);
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
    //noteOne.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
    //noteTwo.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
    //noteThree.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
    //noteFour.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
    //noteFive.noteFreq.setTable(waveTables[buttons[BUTT_A]]);
    if (activePoll/noisePoll > 0) {
      switch (activeButton){//cycling actions
        case BUTT_A://osctog
          if (buttons[BUTT_A] < 4){
            buttons[BUTT_A]++;
          } else {
            buttons[BUTT_A]=0;
          }
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

int note(uint16_t val) {
  int noteCode = 0;
  if(val > 999) {
    noteCode = 1000;
  }
  switch(val & 0xFF){
    case 0x5E:
      return (noteCode+110);//C1
    case 0x3B:
      return (noteCode+111);//C1#
    case 0x4B:
      return (noteCode+120);//D1
    case 0x5F:
      return (noteCode+121);//D1#
    case 0x38:
      return (noteCode+130);//E1
    case 0x49:
      return (noteCode+140);//F1
    case 0x67:
      return (noteCode+141);//F1#
    case 0x4D:
      return (noteCode+150);//G1
    case 0x4A:
      return (noteCode+151);//G1#
    case 0x36:
      return (noteCode+160);//A1
    case 0x37:
      return (noteCode+161);//A1#
    case 0x55:
      return (noteCode+170);//B1
    case 0x5D:
      return (noteCode+210);//C2
    case 0x56:
      return (noteCode+211);//C2#
    case 0x46:
      return (noteCode+220);//D2
    case 0x35:
      return (noteCode+221);//D2#
    case 0x34:
      return (noteCode+230);//E2
    case 0x52:
      return (noteCode+240);//F2
    case 0x59:
      return (noteCode+241);//F2#
    case 0x43:
      return (noteCode+250);//G2
    case 0x44:
      return (noteCode+251);//G2#
    case 0x62:
      return (noteCode+260);//A2
    case 0x33:
      return (noteCode+261);//A2#
    case 0x45:
      return (noteCode+270);//B2
    case 0x54:
      return (noteCode+310);//C3
    case 0x58:
      return (noteCode+311);//C3#
    case 0x53:
      return (noteCode+320);//D3
    case 0x61:
      return (noteCode+321);//D3#
    case 0x32:
      return (noteCode+330);//E3
    case 0x57:
      return (noteCode+340);//F3
    case 0x63:
      return (noteCode+341);//F3#
    case 0x5A:
      return (noteCode+350);//G3
    case 0x41:
      return (noteCode+351);//G3#
    case 0x40:
      return (noteCode+360);//A3
    case 0x31:
      return (noteCode+361);//A3#
    case 0x51:
      return (noteCode+370);//B3
    case 0x1D:
      return (noteCode+410);//C4
    default://to avoid compiler warning
      return 0;
  }
}

float findFreq(int note_val){
  switch(note_val) {
    case 110:
      return(261.626);
    case 111:
      return(277.183);
    case 120:
      return(293.665);
    case 121:
      return(311.127);
    case 130:
      return(329.628);
    case 140:
      return(349.228);
    case 141:
      return(369.994);
    case 150:
      return(391.995);
    case 151:
      return(415.305);
    case 160:
      return(440);
    case 161:
      return(466.164);
    case 170:
      return(493.883);
    case 210:
      return(523.251);
    case 211:
      return(554.365);
    case 220:
      return(587.33);
    case 221:
      return(622.254);
    case 230:
      return(659.255);
    case 240:
      return(698.456);
    case 241:
      return(739.989);
    case 250:
      return(783.991);
    case 251:
      return(830.609);
    case 260:
      return(880);
    case 261:
      return(932.328);
    case 270:
      return(987.767);
    case 310:
      return(1046.502);
    case 311:
      return(1108.731);
    case 320:
      return(1174.659);
    case 321:
      return(1244.508);
    case 330:
      return(1318.51);
    case 340:
      return(1396.913);
    case 341:
      return(1479.978);
    case 350:
      return(1567.982);
    case 351:
      return(1661.219);
    case 360:
      return(1760);
    case 361:
      return(1864.655);
    case 370:
      return(1975.533);
    case 410:
      return(2093.005);
    default://to avoid compiler warning
      return 0;
  } }

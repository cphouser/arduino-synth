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
#include <tables/logistic2048_int8.h>
#include <tables/cyc2048_int8.h>


#include <PS2KeyAdvanced.h>
#include <PS2KeyCode.h>
#include <PS2KeyTable.h>


#define DATAPIN 4
#define IRQPIN  3
#define DIGIN1 5
#define DIGIN2 6
#define DIGIN3 7
#define DIGIN4 8
#define DIGINA 10
#define DIGINB 11
#define DIGINC 12
#define DIGIND 13
#define AUDIOUT 9
#define POT1 0

Oscil <2048, AUDIO_RATE> osc1[6] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA),
  Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA), Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA),
  Oscil <2048, AUDIO_RATE>(LOGISTIC2048_DATA), Oscil <2048, AUDIO_RATE>(CYC2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc2[6] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA),
  Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA), Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA),
  Oscil <2048, AUDIO_RATE>(LOGISTIC2048_DATA), Oscil <2048, AUDIO_RATE>(CYC2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc3[6] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA),
  Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA), Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA),
  Oscil <2048, AUDIO_RATE>(LOGISTIC2048_DATA), Oscil <2048, AUDIO_RATE>(CYC2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc4[6] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA),
  Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA), Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA),
  Oscil <2048, AUDIO_RATE>(LOGISTIC2048_DATA), Oscil <2048, AUDIO_RATE>(CYC2048_DATA)
};
Oscil <2048, AUDIO_RATE> osc5[6] = {
  Oscil <2048, AUDIO_RATE>(SIN2048_DATA), Oscil  <2048, AUDIO_RATE>(SAW2048_DATA),
  Oscil  <2048, AUDIO_RATE>(TRIANGLE2048_DATA), Oscil  <2048, AUDIO_RATE>(SQUARE_NO_ALIAS_2048_DATA),
  Oscil <2048, AUDIO_RATE>(LOGISTIC2048_DATA), Oscil <2048, AUDIO_RATE>(CYC2048_DATA)
};

float oneFreq = 0;
float twoFreq = 0;
float thrFreq = 0;
float fouFreq = 0;
float fivFreq = 0;

int digIn1 = 0;
int buttonTiming = 0;
int oscTog = 0;

uint16_t c;
PS2KeyAdvanced keyboard;


void setup()
{
  startMozzi(CONTROL_RATE);
  //osc.setFreq(one_freq);
  pinMode(AUDIOUT, OUTPUT);
  pinMode(DIGIN1, INPUT);
  pinMode(DIGIN2, INPUT);
  pinMode(DIGIN3, INPUT);
  pinMode(DIGIN4, INPUT);
  pinMode(DIGINA, INPUT);
  pinMode(DIGINB, INPUT);
  pinMode(DIGINC, INPUT);
  pinMode(DIGIND, INPUT);
  keyboard.begin( DATAPIN, IRQPIN ); // Configure the keyboard library
  Serial.begin( 115200 );
  #if defined(ARDUINO_ARCH_AVR)
    Serial.println( F( "PS2 Advanced Key Simple Test:" ) );
  #elif defined(ARDUINO_ARCH_SAM)
    Serial.println( "PS2 Advanced Key Simple Test:" );
  #endif
}

void updateControl(){
  osc1[oscTog].setFreq(oneFreq);
  osc2[oscTog].setFreq(twoFreq);
  osc3[oscTog].setFreq(thrFreq);
  osc4[oscTog].setFreq(fouFreq);
  osc5[oscTog].setFreq(fivFreq);
  
  if( keyboard.available() ) {
  c = keyboard.read(); // read the next key
  if( c > 0 ) {
      int note_val = note(c);
      if(note_val < 1000) {
        if (findFreq(note_val) == oneFreq 
        || findFreq(note_val) == twoFreq 
        || findFreq(note_val) == thrFreq 
        || findFreq(note_val) == fouFreq
        || findFreq(note_val) == fivFreq){
        } else if(oneFreq==0){
          oneFreq = findFreq(note_val);
        } else if(twoFreq==0){
          twoFreq = findFreq(note_val);
        } else if(thrFreq==0){
          thrFreq = findFreq(note_val);
        } else if(fouFreq==0){
          fouFreq = findFreq(note_val);
        } else if(fivFreq==0){
          fivFreq = findFreq(note_val);
        }
      } if(findFreq(note_val - 1000)==oneFreq){
        oneFreq = 0;
      } if(findFreq(note_val - 1000)==twoFreq){
        twoFreq = 0;
      } if(findFreq(note_val - 1000)==thrFreq){
        thrFreq = 0;
      } if(findFreq(note_val - 1000)==fouFreq){
        fouFreq = 0;
      } if(findFreq(note_val - 1000)==fivFreq){
        fivFreq = 0;
      }
    }
  }
  digIn1 = buttonVal(DIGIN1,digIn1);
  if (digIn1 == 1){
    oscTogCounter();
  }
  /*
  Serial.print("\n");
  Serial.print(oneFreq);
  Serial.print(" ");
  Serial.print(twoFreq);
  Serial.print(" ");
  Serial.print(thrFreq);
  Serial.print(" ");
  Serial.print(fouFreq);*/
  Serial.print("\n");
  Serial.print(mozziAnalogRead(POT1));
  Serial.print(" ");
  Serial.print(oscTog);
  Serial.print(" ");
  Serial.print(digIn1);
  Serial.print(" ");
  Serial.print(digitalRead(DIGIN1));
  Serial.print(digitalRead(DIGIN2));
  Serial.print(digitalRead(DIGIN3));
  Serial.print(digitalRead(DIGIN4));
  Serial.print(digitalRead(DIGINA));
  Serial.print(digitalRead(DIGINB));
  Serial.print(digitalRead(DIGINC));
  Serial.print(digitalRead(DIGIND));
}

int updateAudio(){
  return ((osc1[oscTog].next()+osc2[oscTog].next()+osc3[oscTog].next()+osc4[oscTog].next()+osc5[oscTog].next())>>2);
  //return (osc1[oscTog].next());
}

void loop(){
audioHook();
}



int buttonVal(int pin, int var) {//returns state of button on specified digital pin
  boolean state = digitalRead(pin);//currently pressed or not
  
  if (!state && (var == 0) && (buttonTiming == 0)) {
    return 0;//unpressed
  } else if (state && (var == 2)) {
    return 2;//pressed
  } else if (!state && (var == 3) && (buttonTiming == 0)) {
    return 0;//unpressed
  } else if (!state && (var == 3)) {
    buttonTimer();
    return 3;//unpressed
  } else if (state && (var == 1)) {
    return 2;//pressed
  } else if (state && (var == 3)) {
    return 2;//pressed
  } else if (!state && (var == 2)) {
    buttonTimer();
    return 3;//release
  } else if (state && (var == 0)) {
    return 1;//press register
  } else {
    return 0;
  }
}

void buttonTimer(){
  if (buttonTiming == 31) {
    buttonTiming = 0;
  } else {
    buttonTiming++; 
  }
}

void oscTogCounter() {
  if (oscTog == 5) {
    oscTog = 0;
  } else {
    oscTog++;
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
  } }

# Arduino Synth

## Dependencies
* Mozzi 1.0.2 [https://github.com/sensorium/Mozzi]
* PS2Keyboard 1.0.2 [https://github.com/techpaul/PS2KeyAdvanced]

## History
#### 2016-2017
Successfully tested PS2 keyboard input on Arduino Duemillenove, basic wavetable rendering with Mozzi. 
Began adding control input (16 buttons on 8 GPIO pins, 6 slide potentiometers on each analog input). 

#### 2018
Fixed control input hardware (resoldered buttons to accessory board with cleaner wiring and changed button programming to account for noise on GPIO).
Began adding volume envelopes to keyboard input.

#### 2019
Added TM1637 7-segment display, began testing.

#### Future Work
TM1637Display Library does not work with mozzi due to blocking while setting segment values. Try rewriting with mozzi's builtin timer functions.
Completely rewrite sketch - separate IO logic from DSP logic.
Experiment more with digital filters and envelopes. Practice more fixed-point arithmetic.


# Arduino Synth

## Dependencies
* Mozzi 1.1.0 [https://github.com/sensorium/Mozzi]
* PS2KeyAdvanced 1.0.7 [https://github.com/techpaul/PS2KeyAdvanced]
* TM1637-no-delay [https://github.com/xeroxcat/TM1637-no-delay]
* Atmega328 board (tested with an Arduino Duemilanove)

## Planned
#### Power LED display off separate 5v

#### Separate functions and state for keys, voices and mix
`
    key_state -> voice_state -> mix_state
key input-^
  analog input-----^------------^
    button input---^------------^
`
## Changelog (vague)
### Present
#### (3/2020.3)
Attack/Decay envelopes working, definable length and curve. 

#### (3/2020.2) 
LED display working and all original work cleaned up. Experimental implementation of attack-decay envelopes added.

#### (3/2020.1) 
Started this project over 3 years ago as an overambitious first Arduino project. Most of the work over this time has been spent adding and troubleshooting I/O hardware. Current work now is refactoring the current I/O code into more structured interfaces. 

### History
#### 2020
Display library rewritten without blocking with delayMicroseconds.

#### 2019
Added TM1637 7-segment display, began testing.

#### 2018
Fixed control input hardware (resoldered buttons to accessory board with cleaner wiring and changed button programming to account for noise on GPIO).
Began adding volume envelopes to keyboard input.

#### 2016-2017
Successfully tested PS2 keyboard input on Arduino Duemillenove, basic wavetable rendering with Mozzi. 
Began adding control input (16 buttons on 8 GPIO pins, 6 slide potentiometers on each analog input). 


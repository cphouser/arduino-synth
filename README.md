# Arduino Synth

## Dependencies
* Mozzi 1.1.0 [https://github.com/sensorium/Mozzi]
* PS2KeyAdvanced 1.0.7 [https://github.com/techpaul/PS2KeyAdvanced]
* TM1637-no-delay [https://github.com/xeroxcat/TM1637-no-delay]
* Atmega328 board (tested with an Arduino Duemilanove)

## Planned
#### Disable keyboard typematic input
There is a PS/2 command (0xF8) which should disable typmatic input (transmit signal only on press and release). Add function for this command or an arbitrary command to PS2KeyAdvanced. Would allow simplified key processing and reduce load.

#### Attack/Release Ramps
Short ramps on starting/ending a note (separate from pot-envelope which is currently using <Eah.h>) to eliminate the click during those events.

#### Fix <Ead.h> or find another envelope
The envelope in Ead.h doesn't reset when start() is called. Creates problem when a voice is assigned to a new key.

## Changelog (vague)
### Present
(3/2020.2) LED display working and all original work cleaned up. Experimental implementation of attack-decay envelopes added.

(3/2020.1) Started this project over 3 years ago as an overambitious first Arduino project. Most of the work over this time has been spent adding and troubleshooting I/O hardware. Current work now is refactoring the current I/O code into more structured interfaces. 

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


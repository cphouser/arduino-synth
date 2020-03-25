# Arduino Synth
The Arduino library and board, as well as the AVR-GCC compiler, allows hobbyists and prototypers to develop efficiently for microcontrollers using C++, a well documented and structured langugage. The Mozzi Sound Library provides an API on this platform for generating a PWM audio signal using sample tables and real time processing routines. This project aims to create a synthesizer which pushes the capabilities of the hardware and can function as a structured template for similar projects. Different components of the system (keyboard input, control input, note rendering, mixing, etc) are implemented as classes, allowing the synthesis routines to be reused with different hardware interfaces or changed separately from the currently defined hardware interface. As the implementation of experimental features is completed (see **planned** section), these classes will be documented for reuse and extendability. 

(Very) unscripted demo of the current status as of 3/24/20: [https://www.youtube.com/watch?v=s4Vc9mn1Nbo]

## Dependencies
* Mozzi 1.1.0 [https://github.com/sensorium/Mozzi]
* PS2KeyAdvanced 1.0.7 [https://github.com/techpaul/PS2KeyAdvanced]
* TM1637-no-delay [https://github.com/xeroxcat/TM1637-no-delay]
* Atmega328 board (tested with an Arduino Duemilanove)

## Planned
#### Power LED display off separate 5v
Display is needed for displaying current state set by buttons, currently generates too much noise on audio signal to be useful.

#### Link buttons to mixing parameters for envelopes and filters.
Separate filtering functions within the mixer class and define interface for toggling and adjusting parameters.

#### Separate functions and state for keys, voices and mix (mostly done)
```
    key_state -> voice_state -> mix_state
key input-^
  analog input-----^------------^
    button input---^------------^
```
#### Monophonic input mode with unison/chorus
If room 

#### Note pattern record and playback
If room

#### Less generic project name
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


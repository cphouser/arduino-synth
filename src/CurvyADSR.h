/*
 * CurvyADSR.h
 * Based on ADSR.h -
 * Copyright 2012 Tim Barrass.
 * A Part of Mozzi.
 *
 * Mozzi is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * curves for state transitions added by Calvin Houser 2020
 *
 *
 */

#ifndef CurvyADSR_H_
#define CurvyADSR_H_

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "Line.h"
#include "mozzi_fixmath.h"

/** A simple CurvyADSR envelope generator.  This implementation has separate update() and next()
methods, where next() interpolates values between each update().
The "normal" way to use this would be with update() in updateControl(), where it calculates a new internal state each control step,
and then next() is in updateAudio(), called much more often, where it interpolates between the control values.
This also allows the CurvyADSR updates to be made even more sparsely if desired, eg. every 3rd control update.
@tparam CONTROL_UPDATE_RATE The frequency of control updates.
Ordinarily this will be CONTROL_RATE, but an alternative (amongst others) is
to set this as well as the LERP_RATE parameter to AUDIO_RATE, and call both update() and next() in updateAudio().
Such a use would allow accurate envelopes with finer resolution of the control points than CONTROL_RATE.
@tparam LERP_RATE Sets how often next() will be called, to interpolate between updates set by CONTROL_UPDATE_RATE.
This will produce the smoothest results if it's set to AUDIO_RATE, but if you need to save processor time and your
envelope changes slowly or controls something like a filter where there may not be problems with glitchy or clicking transitions,
LERP_RATE could be set to CONTROL_RATE (for instance).  Then update() and next() could both be called in updateControl(),
greatly reducing the amount of processing required compared to calling next() in updateAudio().
@todo Test whether using the template parameters makes any difference to speed,
and rationalise which units do and don't need them.
Template objects are messy when you try to use pointers to them,
you have to include the whole template shebang in the pointer handling.
*/

//long implementation of exponentiation by squaring in mozzi_fixmath
inline
long ipow_long(long base, long exp)
{
	long result = 1;
	while (exp)
    {
      if (exp & 1)
        result *= base;
      exp >>= 1;
      base *= base;
    }
	return result;
}

template <unsigned int CONTROL_UPDATE_RATE, unsigned int LERP_RATE>
class CurvyADSR
{
private:

	const unsigned int LERPS_PER_CONTROL;

	unsigned int update_step_counter;
	unsigned int num_update_steps;
  Q8n0 base_level;
  char active_curve;//could remove and reference struct var thru current_phase (slower?)
  Q8n24 incr_coef;
  bool negative;

	enum {ATTACK,DECAY,SUSTAIN,RELEASE,IDLE};

	struct phase{
		byte phase_type;
		unsigned int update_steps;
		long lerp_steps; // signed, to match params to transition (line) type Q15n16, below
		Q8n0 level;
    char phase_curve = 1; //Positive Number: exponent for phase curve
                      //Ex: 1 - linear envelope, 2 - quadratic envelope curve
	}attack,decay,sustain,release,idle;

	phase * current_phase;

	// Linear audio rate transitions for envelope
	//Line <unsigned long> transition;
	Line <Q8n24> transition; // scale up unsigned char levels for better accuracy, then scale down again for output

	inline
	unsigned int convertMsecToControlUpdateSteps(unsigned int msec){
		return (uint16_t) (((uint32_t)msec*CONTROL_UPDATE_RATE)>>10); // approximate /1000 with shift
	}


	inline
	void setPhase(phase * next_phase) {
		update_step_counter = 1;
		num_update_steps = next_phase->update_steps;
    active_curve = next_phase->phase_curve;
    base_level = current_phase->level;
		current_phase = next_phase;
    if (current_phase->phase_curve == 1) {
      transition.set(((Q8n24)next_phase->level)<<24,next_phase->lerp_steps);
      //Serial.print(((Q8n24)next_phase->level)<<24);
    } else {
      //store increment coefficient sign separately
      negative = next_phase->level > base_level;
      byte phase_delta = negative ? (base_level - next_phase->level) : (next_phase->level - base_level);
      incr_coef = ((Q8n24)(phase_delta))<<24 / ipow_long(num_update_steps, active_curve);
      if (negative)
        transition.set(((Q8n24)base_level<<24) - incr_coef, LERPS_PER_CONTROL);
      else
        transition.set(((Q8n24)base_level<<24) + incr_coef, LERPS_PER_CONTROL);
    }
	}


	inline
	void checkForAndSetNextPhase(phase * next_phase) {
		if (++update_step_counter > num_update_steps){
			setPhase(next_phase);
		} else if (active_curve != 1) {
      Q8n24 next_delta = incr_coef * ipow_long(update_step_counter, active_curve);
      //Serial.print(negative);
      //Serial.print(" ");
      //Serial.println(next_delta);
      if (negative)
        transition.set(((Q8n24)base_level<<24) - next_delta, LERPS_PER_CONTROL);
      else
        transition.set(((Q8n24)base_level<<24) + next_delta, LERPS_PER_CONTROL);
    }
	}


	inline
	void setTime(phase * p, unsigned int msec)
	{
		p->update_steps = convertMsecToControlUpdateSteps(msec);
		p->lerp_steps = (long) p->update_steps * LERPS_PER_CONTROL;
	}


	inline
	void setUpdateSteps(phase * p, unsigned int steps)
	{
		p->update_steps = steps;
		p->lerp_steps = (long) steps * LERPS_PER_CONTROL;
	}


public:

	/** Constructor.
	 */
	CurvyADSR():LERPS_PER_CONTROL(LERP_RATE/CONTROL_UPDATE_RATE)
	{
		attack.phase_type = ATTACK;
		decay.phase_type = DECAY;
		sustain.phase_type = SUSTAIN;
		release.phase_type = RELEASE;
		idle.phase_type = IDLE;
		release.level = 0;
    base_level = 0;
    active_curve = 1;
		adsr_playing = false;
		current_phase = &idle;
	}

	/** Updates the internal controls of the CurvyADSR.
		Call this in updateControl().
		*/
	void update(){ // control rate

		switch(current_phase->phase_type) {

		case ATTACK:
			checkForAndSetNextPhase(&decay);
			break;

		case DECAY:
			checkForAndSetNextPhase(&sustain);
			break;

		case SUSTAIN:
			checkForAndSetNextPhase(&release);
			break;

		case RELEASE:
			checkForAndSetNextPhase(&idle);
			break;

		case IDLE:
			adsr_playing = false;
			break;
		}
	}

	/** Advances one audio step along the CurvyADSR and returns the level.
	Call this in updateAudio().
	@return the next value, as an unsigned char.
	 */
	inline
	unsigned char next()
	{
		unsigned char out = 0;
		if (adsr_playing) out = (transition.next()>>24);
		return out;
	}

	/** Start the attack phase of the CurvyADSR.  This will restart the CurvyADSR no matter what phase it is up to.
	@param reset If true, the envelope will start from 0, even if it is still playing (often useful for effect envelopes).
	If false (default if omitted), the envelope will start rising from the current level, which could be non-zero, if
	it is still playing (most useful for note envelopes).
	*/
	inline
	void noteOn(bool reset=false)
  {
		if (reset) transition.set(0);
		setPhase(&attack);
		adsr_playing = true;
	}

	/** Start the release phase of the CurvyADSR.
	@todo fix release for rate rather than steps (time), so it releases at the same rate whatever the current level.
	*/
	inline
	void noteOff()
  {
		setPhase(&release);
	}

	/** Set the attack level of the CurvyADSR.
	@param value the attack level.
	 */
	inline
	void setAttackCurve(byte level, char curve)
	{
		attack.level=level;
		attack.phase_curve=curve;
	}

	/** Set the decay level of the CurvyADSR.
	@param level the decay level.
	*/
	inline
	void setDecayCurve(byte level, char curve)
	{
		decay.level=level;
		decay.phase_curve=curve;
	}

	/** Set the sustain level of the CurvyADSR.
	@param level the sustain level.  Usually the same as the decay level, 
	for a steady sustained note.
	*/
	inline
	void setSustainCurve(byte level, char curve)
	{
		sustain.level=level;
		sustain.phase_curve=curve;
	}

	/** Set the release level of the CurvyADSR.  Normally you'd make this 0,
	but you have the option of some other level.
	@param level the release level (usually 0).
	*/
	inline
	void setReleaseCurve(byte level, char curve)
	{
		release.level=level;
		release.phase_curve=curve;
	}


		inline
	void setIdleLevel(byte level)
	{
		idle.level=level;
	}

	/** Set the attack and decay levels of the CurvyADSR.  This assumes a conventional
	CurvyADSR where the sustain continues at the same level as the decay, till the release ramps to 0.
	@param attack the new attack level.
	@param decay the new decay level.
	*/
	inline
	void setADCurves(byte atk, char atk_curve, byte dec, char dec_curve)
	{
		setAttackCurve(atk, atk_curve);
		setDecayCurve(dec, dec_curve);
		setSustainCurve(dec, 1); // stay at decay level
		setReleaseCurve(1, 1);
		setIdleLevel(0);
	}

	/** Set the attack, decay, sustain and release levels.
	@param attack the new attack level.
	@param decay the new sustain level.
	@param attack the new sustain level.
	@param decay the new release level.
	*/
	inline
	void setCurves(byte atk, char atk_curve, byte dec, char dec_curve,
                 byte stn, char stn_curve, byte rel, char rel_curve)
	{
		setAttackCurve(atk, atk_curve);
		setDecayCurve(dec, dec_curve);
		setSustainCurve(stn, stn_curve);
		setReleaseCurve(rel, rel_curve);
		setIdleLevel(0);
	}

	/** Set the attack time of the CurvyADSR in milliseconds.
	The actual time taken will be resolved within the resolution of CONTROL_RATE.
	@param msec the unsigned int attack time in milliseconds.
	@note Beware of low values (less than 20 or so, depending on how many steps are being taken),
	in case internal step size gets calculated as 0, which would mean nothing happens.
	 */
	inline
	void setAttackTime(unsigned int msec)
	{
		setTime(&attack, msec);
	}

	/** Set the decay time of the CurvyADSR in milliseconds.
	The actual time taken will be resolved within the resolution of CONTROL_RATE.
	@param msec the unsigned int decay time in milliseconds.
	@note Beware of low values (less than 20 or so, depending on how many steps are being taken),
	in case internal step size gets calculated as 0, which would mean nothing happens.
	*/
	inline
	void setDecayTime(unsigned int msec)
	{
		setTime(&decay, msec);
	}

	/** Set the sustain time of the CurvyADSR in milliseconds.
	The actual time taken will be resolved within the resolution of CONTROL_RATE.
	The sustain phase will finish if the CurvyADSR recieves a noteOff().
	@param msec the unsigned int sustain time in milliseconds.
	@note Beware of low values (less than 20 or so, depending on how many steps are being taken),
	in case internal step size gets calculated as 0, which would mean nothing happens.
	*/
	inline
	void setSustainTime(unsigned int msec)
	{
		setTime(&sustain, msec);
	}

	/** Set the release time of the CurvyADSR in milliseconds.
	The actual time taken will be resolved within the resolution of CONTROL_RATE.
	@param msec the unsigned int release time in milliseconds.
	@note Beware of low values (less than 20 or so, depending on how many steps are being taken),
	in case internal step size gets calculated as 0, which would mean nothing happens.
	*/
	inline
	void setReleaseTime(unsigned int msec)
	{
		setTime(&release, msec);
	}


	inline
	void setIdleTime(unsigned int msec)
	{
		setTime(&idle, msec);
	}

	/** Set the attack, decay and release times of the CurvyADSR in milliseconds.
	The actual times will be resolved within the resolution of CONTROL_RATE.
	@param attack_ms the new attack time in milliseconds.
	@param decay_ms the new decay time in milliseconds.
	@param sustain_ms the new sustain time in milliseconds.
	@param release_ms the new release time in milliseconds.
	@note Beware of low values (less than 20 or so, depending on how many steps are being taken),
	in case internal step size gets calculated as 0, which would mean nothing happens.
	*/
	inline
	void setTimes(unsigned int attack_ms, unsigned int decay_ms, unsigned int sustain_ms, unsigned int release_ms)
	{
		setAttackTime(attack_ms);
		setDecayTime(decay_ms);
		setSustainTime(sustain_ms);
		setReleaseTime(release_ms);
		setIdleTime(65535); // guarantee step size of line will be 0
	}

	/** Set the attack time of the CurvyADSR, expressed as the number of update steps (not CurvyADSR::next() interpolation steps) in the attack phase.
	@param steps the number of times CurvyADSR::update() will be called in the attack phase.
	 */
	inline
	void setAttackUpdateSteps(unsigned int steps)
	{
		setUpdateSteps(&attack, steps);
	}

	/** Set the decay time of the CurvyADSR, expressed as the number of update steps (not CurvyADSR::next() interpolation steps) in the decay phase.
	@param steps the number of times CurvyADSR::update() will be called in the decay phase.
	 */
	inline
	void setDecayUpdateSteps(unsigned int steps)
	{
		setUpdateSteps(&decay, steps);
	}

	/** Set the sustain time of the CurvyADSR, expressed as the number of update steps (not CurvyADSR::next() interpolation steps) in the sustain phase.
	@param steps the number of times CurvyADSR::update() will be called in the sustain phase.
	*/
	inline
	void setSustainUpdateSteps(unsigned int steps)
	{
		setUpdateSteps(&sustain, steps);
	}

	/** Set the release time of the CurvyADSR, expressed as the number of update steps (not CurvyADSR::next() interpolation steps) in the release phase.
	@param steps the number of times CurvyADSR::update() will be called in the release phase.
	 */
	inline
	void setReleaseUpdateSteps(unsigned int steps)
	{
		setUpdateSteps(&release, steps);
	}


		inline
	void setIdleUpdateSteps(unsigned int steps)
	{
		setUpdateSteps(&idle, steps);
	}

	/** Set the attack, decay and release times of the CurvyADSR, expressed in update steps (not CurvyADSR::next() interpolation steps).
	@param attack_steps the number of update steps in the attack phase
	@param decay_steps the number of update steps in the decay phase
	@param sustain_steps the number of update steps in the sustain phase
	@param release_steps the number of update steps in the release phase
	*/
	inline
	void setAllUpdateSteps(unsigned int attack_steps, unsigned int decay_steps, unsigned int sustain_steps, unsigned int release_steps)
	{
		setAttackUpdateSteps(attack_steps);
		setDecayUpdateSteps(decay_steps);
		setSustainUpdateSteps(sustain_steps);
		setReleaseUpdateSteps(release_steps);
		setIdleUpdateSteps(65535); // guarantee step size of line will be 0
	}


bool adsr_playing;

	/** Tells if the envelope is currently playing.
	@return true if playing, false if in IDLE state
	*/
	inline
	bool playing()
	{
		return adsr_playing;
	}
};

/** @example 07.Envelopes/CurvyADSR_Envelope/CurvyADSR_Envelope.ino
This is an example of how to use the CurvyADSR class.
*/

#endif /* CurvyADSR_H_ */

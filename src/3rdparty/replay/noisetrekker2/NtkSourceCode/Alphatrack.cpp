// SPDX-License-Identifier: LicenseRef-NoiseTrekker2
// CSynth V1.4 Class for C++
// Description: Analogue Physical Modelling Simulation of monophonic
// Vintage modular synthesizer in realtime. 32 Bit float resolution.
// 
// By Juan Antonio Arguelles Rius <gx11488@agger.net>
//
// Feel free to comment/send any suggestions, optimizations or 
// depuration of this code. 
//
// Or implement your new members/routines.
//
// Current Components:
//
// * 2 Oscillators [5 Modes: Sine, Sawtooth, 
//   Pulse[with Pulse Width control], Noise and Oscillator Off]
//   including Volume Control for each OSC.
// * Sub Oscillator
// * 1 VCF (Voltage Controlled Filter)
//   [3 Types: Lowpass, Highpass, Thru] with Resonance control.
// * 2 ADSR Envelope Generators.
// * 2 Low Frequency Oscillators.
// * Modulation Matrix parameters

float SIN[360]; // Sine float-precalculated table, in absolute degrees.

/* Struct used to store/update synthesizer parameters */

struct SynthParameters
{

char presetname[20];

unsigned char osc1_waveform; /* 0 - 4 */
unsigned char osc2_waveform;
	
int osc1_pw; /* 0 - 512 */
int osc2_pw;	

unsigned char osc2_detune; /* 0 - 128 */
unsigned char osc2_finetune; /* 0 - 128 */
	
unsigned char vcf_cutoff; /* 0 - 128 */
unsigned char vcf_resonance; /* 0 - 128 */

unsigned char vcf_type; /* 0 - 2 */ 

/* Envelopes and LFO's properties */

int env1_attack; /* In samples */
int env1_decay;
unsigned char env1_sustain; /* 0 -128 */
int env1_release;

int env2_attack; /* In samples */
int env2_decay;
unsigned char env2_sustain; /* 0 -128 */
int env2_release;

int lfo1_period;
int lfo2_period;

/* Envelopes and LFO's modulation variables */

unsigned char lfo1_osc1_pw;
unsigned char lfo1_osc2_pw;
unsigned char lfo1_osc1_pitch;
unsigned char lfo1_osc2_pitch;
unsigned char lfo1_osc1_volume;
unsigned char lfo1_osc2_volume;	
unsigned char lfo1_vcf_cutoff;
unsigned char lfo1_vcf_resonance;	

unsigned char lfo2_osc1_pw;
unsigned char lfo2_osc2_pw;
unsigned char lfo2_osc1_pitch;
unsigned char lfo2_osc2_pitch;
unsigned char lfo2_osc1_volume;
unsigned char lfo2_osc2_volume;	
unsigned char lfo2_vcf_cutoff;
unsigned char lfo2_vcf_resonance;	

unsigned char env1_osc1_pw;
unsigned char env1_osc2_pw;
unsigned char env1_osc1_pitch;
unsigned char env1_osc2_pitch;
unsigned char env1_osc1_volume;
unsigned char env1_osc2_volume;	
unsigned char env1_vcf_cutoff;
unsigned char env1_vcf_resonance;	

unsigned char env2_osc1_pw;
unsigned char env2_osc2_pw;
unsigned char env2_osc1_pitch;
unsigned char env2_osc2_pitch;
unsigned char env2_osc1_volume;
unsigned char env2_osc2_volume;	
unsigned char env2_vcf_cutoff;
unsigned char env2_vcf_resonance;

unsigned char osc3_volume;
bool osc3_switch;

unsigned char ptc_glide;
unsigned char glb_volume;

/* hehe, 46 parameters =] */
};

class CSynth{
public:

/* Synthesizer Prototype Functions */

	/* Initialization or parameter update */

	
	char ENV1_STAGE;
	char ENV2_STAGE;

	void SynthReset(void);
	void SynthChangeParameters(SynthParameters TSP);

	/* Work functions */

	float SynthGetSample(void);
	void SynthNoteOn(float note, float speed);
	void SynthNoteOff(void);
	
private:

	/* Internal Use */

	void SynthEnvUpdate(void);
	void SynthLfoAdvance(void);
	void SynthEnvRun(void);
	float SynthGetOscValue(char type,float offset);
	float SynthFilter(float input);
	
/* Synthesizer properties */

	char OSC1_WAVEFORM;
	char OSC2_WAVEFORM;
	
	float OSC1_PW;
	float OSC2_PW;	

	float T_OSC1_PW;
	float T_OSC2_PW;
	float T_OSC1_VOLUME;
	float T_OSC2_VOLUME;

	float GLOBAL_VOLUME;

	float T_OSC1_STEP;
	float T_OSC2_STEP;
	float VT_OSC1_STEP;
	float VT_OSC2_STEP;
	
	float OSC2_DETUNE;
	float OSC2_FINETUNE;	
	
	float VCF_CUTOFF;
	float VCF_RESONANCE;
	char VCF_TYPE;

/* Envelopes and LFO's properties */

	int ENV1_ATTACK;
	int ENV1_DECAY;
	float ENV1_SUSTAIN;
	int ENV1_RELEASE;

	float ENV1_A_COEF;
	float ENV1_D_COEF;
	float ENV1_R_COEF;

	int ENV2_ATTACK;
	int ENV2_DECAY;
	float ENV2_SUSTAIN;
	int ENV2_RELEASE;
	
	float ENV2_A_COEF;
	float ENV2_D_COEF;
	float ENV2_R_COEF;

	float LFO1_PERIOD;
	float LFO2_PERIOD;

	int LFO1_GR;
	int LFO2_GR;
	
	int LFO1_SUBGRCOUNTER;
	int LFO2_SUBGRCOUNTER;
	
	int LFO1_SUBGRMAX;
	int LFO2_SUBGRMAX;
	
	float LFO1_VALUE;
	float LFO2_VALUE;

/* Envelopes and LFO's modulation variables */

	float LFO1_OSC1_PW;
	float LFO1_OSC2_PW;
	float LFO1_OSC1_PITCH;
	float LFO1_OSC2_PITCH;
	float LFO1_OSC1_VOLUME;
	float LFO1_OSC2_VOLUME;	
	float LFO1_VCF_CUTOFF;
	float LFO1_VCF_RESONANCE;	

	float LFO2_OSC1_PW;
	float LFO2_OSC2_PW;
	float LFO2_OSC1_PITCH;
	float LFO2_OSC2_PITCH;
	float LFO2_OSC1_VOLUME;
	float LFO2_OSC2_VOLUME;	
	float LFO2_VCF_CUTOFF;
	float LFO2_VCF_RESONANCE;	

	float ENV1_OSC1_PW;
	float ENV1_OSC2_PW;
	float ENV1_OSC1_PITCH;
	float ENV1_OSC2_PITCH;
	float ENV1_OSC1_VOLUME;
	float ENV1_OSC2_VOLUME;	
	float ENV1_VCF_CUTOFF;
	float ENV1_VCF_RESONANCE;	

	float ENV2_OSC1_PW;
	float ENV2_OSC2_PW;
	float ENV2_OSC1_PITCH;
	float ENV2_OSC2_PITCH;
	float ENV2_OSC1_VOLUME;
	float ENV2_OSC2_VOLUME;	
	float ENV2_VCF_CUTOFF;
	float ENV2_VCF_RESONANCE;	

/* Internal rendering variables */

	float OSC1_POSITION;
	float OSC2_POSITION;
	float OSC3_POSITION;

	float TOSC1_POSITION;
	float TOSC2_POSITION;
	
	float OSC1_STEP;
	float OSC2_STEP;
	float OSC3_STEP;

	int ENV1_COUNTER;
	int ENV2_COUNTER;

	float ENV1_VALUE;
	float ENV2_VALUE;

	float OSC3_VOLUME;
	bool OSC3_SWITCH;

	float OSC1_freakpw1;
	float OSC1_freakpw2;

	float OSC2_freakpw1;
	float OSC2_freakpw2;

	float sbuf0;
	float sbuf1;

	float GS_VAL;
	float PTC_GLIDE;

	float GLB_VOLUME;
};


/* This next function resets the synthesizer to default values */

void CSynth::SynthReset(void)
{
	/* Synthesizer General Reset */

	GS_VAL=0;
	GLB_VOLUME=1.0f;
	OSC1_WAVEFORM=1; /* Sawtooth */
	OSC2_WAVEFORM=1; /* Sawtooth */
	
	OSC1_PW=0; /* Square (Middle Width) */
	OSC2_PW=0; /* Square */	

	T_OSC1_PW=0;
	T_OSC2_PW=0;

	T_OSC1_VOLUME=0;
	T_OSC2_VOLUME=0;

	GLOBAL_VOLUME=0;

	OSC2_DETUNE=0; /* No Semitone Detune */
	OSC2_FINETUNE=0.1f;	/* 1/10 Semitone detune */
	
	VCF_CUTOFF=0.5f; /* 10000Hz Cutoff */
	VCF_RESONANCE=0.5f; /* Not very weird =] */
	VCF_TYPE=0; /* LowPass filter */

	ENV1_ATTACK=2560; /* About 59 miliseconds */
	ENV1_DECAY=2560; /* The same here */
	ENV1_SUSTAIN=0.3f; /* Sustain volume at 1/3 */
	ENV1_RELEASE=16384; /* About 371 Miliseconds */ 

	ENV2_ATTACK=2560; /* About 59 miliseconds */
	ENV2_DECAY=2560; /* The same here */
	ENV2_SUSTAIN=0.3f; /* Sustain volume at 1/3 */
	ENV2_RELEASE=16384; /* About 371 Miliseconds */ 

	SynthEnvUpdate(); /* Update And Compute ENV1 and ENV2 coefficients */

	LFO1_PERIOD=16;
	LFO2_PERIOD=16;

	LFO1_GR=0;
	LFO2_GR=0;

	LFO1_SUBGRCOUNTER=0;
	LFO2_SUBGRCOUNTER=0;

	LFO1_SUBGRMAX=200;
	LFO2_SUBGRMAX=200;

	LFO1_VALUE=0;
	LFO2_VALUE=0;

	LFO1_OSC1_PW=0;
	LFO1_OSC2_PW=0;
	LFO1_OSC1_PITCH=0;
	LFO1_OSC2_PITCH=0;
	LFO1_OSC1_VOLUME=0;
	LFO1_OSC2_VOLUME=0;	
	LFO1_VCF_CUTOFF=0;
	LFO1_VCF_RESONANCE=0;	
	
	LFO2_OSC1_PW=0;
	LFO2_OSC2_PW=0;
	LFO2_OSC1_PITCH=0;
	LFO2_OSC2_PITCH=0;
	LFO2_OSC1_VOLUME=0;
	LFO2_OSC2_VOLUME=0;	
	LFO2_VCF_CUTOFF=0;
	LFO2_VCF_RESONANCE=0;	

	ENV1_OSC1_PW=0;
	ENV1_OSC2_PW=0;
	ENV1_OSC1_PITCH=0;
	ENV1_OSC2_PITCH=0;
	ENV1_OSC1_VOLUME=1.0;
	ENV1_OSC2_VOLUME=1.0;	
	ENV1_VCF_CUTOFF=0;
	ENV1_VCF_RESONANCE=0;	

	ENV2_OSC1_PW=0;
	ENV2_OSC2_PW=0;
	ENV2_OSC1_PITCH=0;
	ENV2_OSC2_PITCH=0;
	ENV2_OSC1_VOLUME=0;
	ENV2_OSC2_VOLUME=0;	
	ENV2_VCF_CUTOFF=0;
	ENV2_VCF_RESONANCE=0;	
	
	OSC1_freakpw1=0;
	OSC1_freakpw2=0;
	OSC2_freakpw1=0;
	OSC2_freakpw2=0;

	OSC1_POSITION=0;
	OSC2_POSITION=0;
	OSC3_POSITION=0;
	TOSC1_POSITION=0;
	TOSC2_POSITION=0;
	
	OSC3_SWITCH=false;

	OSC1_STEP=0;
	OSC2_STEP=0;
	OSC3_STEP=0;
	
	T_OSC1_STEP=0;
	T_OSC2_STEP=0;
	VT_OSC1_STEP=0;
	VT_OSC2_STEP=0;
	
	ENV1_STAGE=0;
	ENV2_STAGE=0;

	ENV1_COUNTER=0;
	ENV2_COUNTER=0;

	ENV1_VALUE=0;
	ENV2_VALUE=0;
	
	GS_VAL=0;
	
	OSC3_VOLUME=0;

	PTC_GLIDE=1.0f;

	/* Initializing SINETABLE */

	for (int ini=0;ini<360;ini++)
		SIN[ini]=(float)sin(ini*0.0174532);

	// mvtiaine: reset also sbuf0 and sbuf1
	sbuf0 = 0;
	sbuf1 = 0;
}

/* This next function returns (gives) a 32-bit float value (sample),
where:

  This function is for internal use only.

- Type is the wavetype [0-Sine, 1-Sawtooth, 2-Pulse, 3-Random, 4-Thru (off)]
- offset is a float oscillator offset. Range: [0.0,512.0)
- pwi is a float pulsewidth amount. Range: [0.0,512.0]
*/

float CSynth::SynthGetOscValue(char type,float offset)
{
	switch(type)
	{

		/* Sine */	

		case 0: 
			return (float)sin(offset*0.0122718)*16384;
		break;
		
		/* SawTooth */

		case 1: 
			return offset*64;
		break;
		
		/* Pulse */

		case 2:
			if (offset<0)
			return 16384;
			else
			return -16384;
		break;
		
		/* Noise */

		case 3:
			return (float)rand()-16384;
		break;
		
		/* Silence */

		case 4: 
			return 0;
		break;
		
		/* Default Nothing */
		
		default:
			return 0;
		break;
	}
}

/* This next function is for internal use only. Makes the LFO's run. */

void CSynth::SynthLfoAdvance(void)
{
		LFO1_SUBGRCOUNTER++;
		LFO2_SUBGRCOUNTER++;
		
		if(LFO1_SUBGRCOUNTER>LFO1_SUBGRMAX)
		{
			LFO1_SUBGRCOUNTER=0;
			LFO1_GR++;

			if(LFO1_GR>359)
			LFO1_GR=0;

			LFO1_VALUE=SIN[LFO1_GR];

		}

		if(LFO2_SUBGRCOUNTER>LFO2_SUBGRMAX)
		{
			LFO2_SUBGRCOUNTER=0;
			LFO2_GR++;
			if(LFO2_GR>359)
				LFO2_GR=0;
			LFO2_VALUE=SIN[LFO2_GR];

		}

}

/* The cooler NoteOn typical message for this Class CSynth Objects =]. */

void CSynth::SynthNoteOn(float note, float speed)
{
	
	/* Triggering ENV1 and ENV2, most synths trigger ENVs when 
	a note on message is received... but, well, change that if
	you want. */

	ENV1_STAGE=1; /* '0' is off, '1' starts the attack */
	ENV2_STAGE=1;
	
	ENV1_COUNTER=0; /* Envelope stage counter, in samples */
	ENV2_COUNTER=0;

	/* Attack ENVELOPES from current level 8)....
	Most coders reset env-value to 0 and compute, and this is
	a terrible error, since we have to compute a relative ramp
	from the current level to 1.0, since not always we receive noteon
	when the last note is released or the volume is 0.
	This will avoid 'clicking' and make the synth smoother. */

	/* The same process is made on Note Off message too */
	
	/* Compute correct attack coefficients here */

	ENV1_A_COEF=(1.0f-ENV1_VALUE)/ENV1_ATTACK;
	ENV2_A_COEF=(1.0f-ENV2_VALUE)/ENV2_ATTACK;

	/* Assign resampling steps to each oscillator: */

	OSC1_STEP=(float)pow(2.0,note/12.0f);
	OSC2_STEP=(float)pow(2.0,(note+OSC2_FINETUNE+OSC2_DETUNE)/12.0f);
	OSC3_STEP=OSC1_STEP*0.5f;
	
	OSC1_freakpw1=OSC1_STEP*2;
	OSC1_freakpw2=512-OSC1_STEP*2;

	OSC2_freakpw1=OSC2_STEP*2;
	OSC2_freakpw2=512-OSC2_STEP*2;
	GLOBAL_VOLUME=speed;
}

/* Envelope run function */

void CSynth::SynthEnvRun(void)
{
	/* ENV1 */

	switch(ENV1_STAGE)
	{

	/* Attack */

	case 1:
	ENV1_VALUE+=ENV1_A_COEF;
	ENV1_COUNTER++;

	if (ENV1_COUNTER==ENV1_ATTACK)
	{
		ENV1_COUNTER=0;
		ENV1_STAGE=2;
	}
	break;

	/* Decay */

	case 2:
	ENV1_VALUE-=ENV1_D_COEF;
	ENV1_COUNTER++;

	if (ENV1_COUNTER==ENV1_DECAY)
	{
		ENV1_COUNTER=0;
		ENV1_STAGE=3;
	}
	break;

	/* Sustain */

	case 3:
	ENV1_VALUE=ENV1_SUSTAIN;
	break;

	/* Release */

	case 4:
	ENV1_VALUE-=ENV1_R_COEF;
	ENV1_COUNTER++;

	if (ENV1_COUNTER==ENV1_RELEASE)
	{
		ENV1_COUNTER=0;
		ENV1_STAGE=0; /* Stop the rock ENV1 */
	}
	break;
		
	} // Envelope 1 runned

	/* ENV2 */

	switch(ENV2_STAGE)
	{

	/* Attack */

	case 1:
	ENV2_VALUE+=ENV2_A_COEF;
	ENV2_COUNTER++;

	if (ENV2_COUNTER==ENV2_ATTACK)
	{
		ENV2_COUNTER=0;
		ENV2_STAGE=2;
	}
	break;

	/* Decay */

	case 2:
	ENV2_VALUE-=ENV2_D_COEF;
	ENV2_COUNTER++;

	if (ENV2_COUNTER==ENV2_DECAY)
	{
		ENV2_COUNTER=0;
		ENV2_STAGE=3;
	}
	break;

	/* Sustain */

	case 3:
	ENV2_VALUE=ENV2_SUSTAIN;
	break;

	/* Release */

	case 4:
	ENV2_VALUE-=ENV2_R_COEF;
	ENV2_COUNTER++;

	if (ENV2_COUNTER==ENV2_RELEASE)
	{
		ENV2_COUNTER=0;
		ENV2_STAGE=0; /* Stop the rock ENV2 */
	}
	break;
		
	} // Envelope 2 runned
}

/* 'Note Off' message for CSynth class objects */

void CSynth::SynthNoteOff(void)
{
	if (ENV1_STAGE>0 && ENV1_STAGE<4)
	{
		ENV1_R_COEF=ENV1_VALUE/(float)ENV1_RELEASE;
		ENV1_COUNTER=0;
		ENV1_STAGE=4;
	}

	if (ENV2_STAGE>0 && ENV2_STAGE<4)
	{
		ENV2_R_COEF=ENV2_VALUE/(float)ENV2_RELEASE;
		ENV2_COUNTER=0;
		ENV2_STAGE=4;
	}
}

/* This function must be called every time that you change any
   envelope (ENV1 or ENV2) value: ATTACK, DECAY, SUSTAIN or RELEASE.

   Technically: it compute 'increase-coef' for each envelope stage.
*/

void CSynth::SynthEnvUpdate(void)
{
	/* Update ENV1 */

	ENV1_A_COEF=1.0f/(float)ENV1_ATTACK;
	ENV1_D_COEF=(1.0f-ENV1_SUSTAIN)/(float)ENV1_DECAY;
	ENV1_R_COEF=ENV1_SUSTAIN/(float)ENV1_RELEASE;

	/* Update ENV2 */

	ENV2_A_COEF=1.0f/(float)ENV2_ATTACK;
	ENV2_D_COEF=(1.0f-ENV2_SUSTAIN)/(float)ENV2_DECAY;
	ENV2_R_COEF=ENV2_SUSTAIN/(float)ENV2_RELEASE;
}

/* The cool/render function, gets the next synth sample. */

float CSynth::SynthGetSample(void)
{

GS_VAL=0;

/* Oscillator1 On */

if(OSC1_WAVEFORM!=4) 
{

T_OSC1_VOLUME=
LFO1_VALUE*LFO1_OSC1_VOLUME
+LFO2_VALUE*LFO2_OSC1_VOLUME
+ENV1_VALUE*ENV1_OSC1_VOLUME
+ENV2_VALUE*ENV2_OSC1_VOLUME
;

T_OSC1_PW=OSC1_PW
+LFO1_VALUE*LFO1_OSC1_PW*256
+LFO2_VALUE*LFO2_OSC1_PW*256
+ENV1_VALUE*ENV1_OSC1_PW*256
+ENV2_VALUE*ENV2_OSC1_PW*256;

GS_VAL+=SynthGetOscValue(OSC1_WAVEFORM,TOSC1_POSITION)*T_OSC1_VOLUME;

T_OSC1_STEP=OSC1_STEP
+LFO1_VALUE*LFO1_OSC1_PITCH
+LFO2_VALUE*LFO2_OSC1_PITCH
+ENV1_VALUE*ENV1_OSC1_PITCH
+ENV2_VALUE*ENV2_OSC1_PITCH;

if (T_OSC1_STEP<0.0f)T_OSC1_STEP=0.0f;

// Glide Work --------------------------------------------------------

if(VT_OSC1_STEP<T_OSC1_STEP)
{
	VT_OSC1_STEP+=PTC_GLIDE;
	if(VT_OSC1_STEP>T_OSC1_STEP)VT_OSC1_STEP=T_OSC1_STEP;
}

if(VT_OSC1_STEP>T_OSC1_STEP)
{
	VT_OSC1_STEP-=PTC_GLIDE;
	if(VT_OSC1_STEP<T_OSC1_STEP)VT_OSC1_STEP=T_OSC1_STEP;
}


// Phase distortion OSC1

float pw=T_OSC1_PW+256;
if(pw<OSC1_freakpw1)pw=OSC1_freakpw1;
if(pw>OSC1_freakpw2)pw=OSC1_freakpw2;
float lcoef=256/pw;
float rcoef=256/(512-pw);

if(OSC1_POSITION<T_OSC1_PW)
TOSC1_POSITION+=VT_OSC1_STEP*lcoef;
else
TOSC1_POSITION+=VT_OSC1_STEP*rcoef;

OSC1_POSITION+=VT_OSC1_STEP;
if (OSC1_POSITION>=256){OSC1_POSITION-=512;TOSC1_POSITION=-256;}

}

/* Oscillator2 On */

if(OSC2_WAVEFORM!=4) 
{

T_OSC2_VOLUME=
LFO1_VALUE*LFO1_OSC2_VOLUME
+LFO2_VALUE*LFO2_OSC2_VOLUME
+ENV1_VALUE*ENV1_OSC2_VOLUME
+ENV2_VALUE*ENV2_OSC2_VOLUME;

T_OSC2_PW=OSC2_PW
+LFO1_VALUE*LFO1_OSC2_PW*256
+LFO2_VALUE*LFO2_OSC2_PW*256
+ENV1_VALUE*ENV1_OSC2_PW*256
+ENV2_VALUE*ENV2_OSC2_PW*256;

GS_VAL+=SynthGetOscValue(OSC2_WAVEFORM,TOSC2_POSITION)*T_OSC2_VOLUME;

T_OSC2_STEP=OSC2_STEP
+LFO1_VALUE*LFO1_OSC2_PITCH
+LFO2_VALUE*LFO2_OSC2_PITCH
+ENV1_VALUE*ENV1_OSC2_PITCH
+ENV2_VALUE*ENV2_OSC2_PITCH;

if (T_OSC2_STEP<0.0f)T_OSC2_STEP=0.0f;

// Glide Work --------------------------------------------------------

if(VT_OSC2_STEP<T_OSC2_STEP)
{
	VT_OSC2_STEP+=PTC_GLIDE;
	if(VT_OSC2_STEP>T_OSC2_STEP)VT_OSC2_STEP=T_OSC2_STEP;
}

if(VT_OSC2_STEP>T_OSC2_STEP)
{
	VT_OSC2_STEP-=PTC_GLIDE;
	if(VT_OSC2_STEP<T_OSC2_STEP)VT_OSC2_STEP=T_OSC2_STEP;
}

// Phase distortion OSC2

float pw=T_OSC2_PW+256;
if(pw<OSC2_freakpw1)pw=OSC2_freakpw1;
if(pw>OSC2_freakpw2)pw=OSC2_freakpw2;
float lcoef=256/pw;
float rcoef=256/(512-pw);

if(OSC2_POSITION<T_OSC2_PW)
TOSC2_POSITION+=VT_OSC2_STEP*lcoef;
else
TOSC2_POSITION+=VT_OSC2_STEP*rcoef;

OSC2_POSITION+=VT_OSC2_STEP;
if (OSC2_POSITION>=256){OSC2_POSITION-=512;TOSC2_POSITION=-256;}

}

if(OSC3_SWITCH) /* SubOscillator On */
{
GS_VAL+=SynthGetOscValue(OSC1_WAVEFORM,OSC3_POSITION)*T_OSC1_VOLUME*OSC3_VOLUME;
OSC3_POSITION+=OSC3_STEP;
if (OSC3_POSITION>=256)OSC3_POSITION-=512;
}

GS_VAL=SynthFilter(GS_VAL)*GLB_VOLUME;

/* Advance all, oscillator, envelopes, and lfo's */

SynthEnvRun();
SynthLfoAdvance();

/* Return value */
return GS_VAL*GLOBAL_VOLUME;
}

void CSynth::SynthChangeParameters(SynthParameters TSP)
{
/* Function body */

OSC1_WAVEFORM=TSP.osc1_waveform;
OSC2_WAVEFORM=TSP.osc2_waveform;

OSC1_PW=(float)TSP.osc1_pw-256;
OSC2_PW=(float)TSP.osc2_pw-256;

OSC2_DETUNE=(float)TSP.osc2_detune-64.0f;
OSC2_FINETUNE=(float)TSP.osc2_finetune*0.0078125f;	

VCF_CUTOFF=(float)TSP.vcf_cutoff*0.0078125f;
VCF_RESONANCE=(float)TSP.vcf_resonance*0.0078125f;
VCF_TYPE=TSP.vcf_type;

ENV1_ATTACK=TSP.env1_attack+1;
ENV1_DECAY=TSP.env1_decay+1;
ENV1_SUSTAIN=(float)TSP.env1_sustain*0.0078125f;
ENV1_RELEASE=TSP.env1_release+1;

ENV2_ATTACK=TSP.env2_attack+1;
ENV2_DECAY=TSP.env2_decay+1;
ENV2_SUSTAIN=(float)TSP.env2_sustain*0.0078125f;
ENV2_RELEASE=TSP.env2_release+1;
	
LFO1_PERIOD=(float)TSP.lfo1_period+1;
LFO1_SUBGRMAX=f2i(((float)SamplesPerTick*0.000277f)*LFO1_PERIOD);

LFO2_PERIOD=(float)TSP.lfo2_period+1;
LFO2_SUBGRMAX=f2i(((float)SamplesPerTick*0.000277f)*LFO2_PERIOD);

/* Envelopes and LFO's matrix modulation variables */

LFO1_OSC1_PW=      ((float)TSP.lfo1_osc1_pw-64)*0.015625f;
LFO1_OSC2_PW=      ((float)TSP.lfo1_osc2_pw-64)*0.015625f;
LFO1_OSC1_PITCH=   ((float)TSP.lfo1_osc1_pitch-64)*0.015625f;
LFO1_OSC2_PITCH=   ((float)TSP.lfo1_osc2_pitch-64)*0.015625f;
LFO1_OSC1_VOLUME=  ((float)TSP.lfo1_osc1_volume-64)*0.015625f;
LFO1_OSC2_VOLUME=  ((float)TSP.lfo1_osc2_volume-64)*0.015625f;
LFO1_VCF_CUTOFF=   ((float)TSP.lfo1_vcf_cutoff-64)*0.015625f;
LFO1_VCF_RESONANCE=((float)TSP.lfo1_vcf_resonance-64)*0.015625f;

LFO2_OSC1_PW=      ((float)TSP.lfo2_osc1_pw-64)*0.015625f;
LFO2_OSC2_PW=      ((float)TSP.lfo2_osc2_pw-64)*0.015625f;
LFO2_OSC1_PITCH=   ((float)TSP.lfo2_osc1_pitch-64)*0.015625f;
LFO2_OSC2_PITCH=   ((float)TSP.lfo2_osc2_pitch-64)*0.015625f;
LFO2_OSC1_VOLUME=  ((float)TSP.lfo2_osc1_volume-64)*0.015625f;
LFO2_OSC2_VOLUME=  ((float)TSP.lfo2_osc2_volume-64)*0.015625f;
LFO2_VCF_CUTOFF=   ((float)TSP.lfo2_vcf_cutoff-64)*0.015625f;
LFO2_VCF_RESONANCE=((float)TSP.lfo2_vcf_resonance-64)*0.015625f;

ENV1_OSC1_PW=      ((float)TSP.env1_osc1_pw-64)*0.015625f;
ENV1_OSC2_PW=      ((float)TSP.env1_osc2_pw-64)*0.015625f;
ENV1_OSC1_PITCH=   ((float)TSP.env1_osc1_pitch-64)*0.015625f;
ENV1_OSC2_PITCH=   ((float)TSP.env1_osc2_pitch-64)*0.015625f;
ENV1_OSC1_VOLUME=  ((float)TSP.env1_osc1_volume-64)*0.015625f;
ENV1_OSC2_VOLUME=  ((float)TSP.env1_osc2_volume-64)*0.015625f;
ENV1_VCF_CUTOFF=   ((float)TSP.env1_vcf_cutoff-64)*0.015625f;
ENV1_VCF_RESONANCE=((float)TSP.env1_vcf_resonance-64)*0.015625f;

ENV2_OSC1_PW=      ((float)TSP.env2_osc1_pw-64)*0.015625f;
ENV2_OSC2_PW=      ((float)TSP.env2_osc2_pw-64)*0.015625f;
ENV2_OSC1_PITCH=   ((float)TSP.env2_osc1_pitch-64)*0.015625f;
ENV2_OSC2_PITCH=   ((float)TSP.env2_osc2_pitch-64)*0.015625f;
ENV2_OSC1_VOLUME=  ((float)TSP.env2_osc1_volume-64)*0.015625f;
ENV2_OSC2_VOLUME=  ((float)TSP.env2_osc2_volume-64)*0.015625f;
ENV2_VCF_CUTOFF=   ((float)TSP.env2_vcf_cutoff-64)*0.015625f;
ENV2_VCF_RESONANCE=((float)TSP.env2_vcf_resonance-64)*0.015625f;
OSC3_VOLUME=       ((float)TSP.osc3_volume-64)*0.015625f;
PTC_GLIDE=		   ((float)TSP.ptc_glide*(float)TSP.ptc_glide)*0.0000015625f;
OSC3_SWITCH=TSP.osc3_switch;
GLB_VOLUME=		   ((float)TSP.glb_volume)*0.0078125f;

SynthEnvUpdate(); /* Update envelopes coefficients */
}

/* Next Function: used to reset synthparameters Structure */
/* Well, I think the default preset is not very cool, but nah! */

void ResetSynthParameters(SynthParameters *TSP)
{
snprintf(TSP->presetname,8,"Untitled");
TSP->osc1_waveform=1;
TSP->osc2_waveform=1;
TSP->osc1_pw=256;
TSP->osc2_pw=256;
TSP->osc2_detune=64;
TSP->osc2_finetune=16;
TSP->vcf_cutoff=64;
TSP->vcf_resonance=64;
TSP->vcf_type=0;
TSP->env1_attack=2560;
TSP->env1_decay=2560;
TSP->env1_sustain=16;
TSP->env1_release=16384;
TSP->env2_attack=2560;
TSP->env2_decay=2560;
TSP->env2_sustain=16;
TSP->env2_release=16384;
TSP->lfo1_period=16;
TSP->lfo2_period=16;
TSP->lfo1_osc1_pw=64;
TSP->lfo1_osc2_pw=64;
TSP->lfo1_osc1_pitch=64;
TSP->lfo1_osc2_pitch=64;
TSP->lfo1_osc1_volume=64;
TSP->lfo1_osc2_volume=64;	
TSP->lfo1_vcf_cutoff=64;
TSP->lfo1_vcf_resonance=64;	
TSP->lfo2_osc1_pw=64;
TSP->lfo2_osc2_pw=64;
TSP->lfo2_osc1_pitch=64;
TSP->lfo2_osc2_pitch=64;
TSP->lfo2_osc1_volume=64;
TSP->lfo2_osc2_volume=64;	
TSP->lfo2_vcf_cutoff=64;
TSP->lfo2_vcf_resonance=64;	
TSP->env1_osc1_pw=64;
TSP->env1_osc2_pw=64;
TSP->env1_osc1_pitch=64;
TSP->env1_osc2_pitch=64;
TSP->env1_osc1_volume=128;
TSP->env1_osc2_volume=128;	
TSP->env1_vcf_cutoff=64;
TSP->env1_vcf_resonance=64;	
TSP->env2_osc1_pw=64;
TSP->env2_osc2_pw=64;
TSP->env2_osc1_pitch=64;
TSP->env2_osc2_pitch=64;
TSP->env2_osc1_volume=64;
TSP->env2_osc2_volume=64;
TSP->env2_vcf_cutoff=64;
TSP->env2_vcf_resonance=64;	
TSP->osc3_volume=128;
TSP->ptc_glide=64;
TSP->osc3_switch=false;
TSP->glb_volume=128;

}

float CSynth::SynthFilter(float input)
{
  if (VCF_TYPE<2)
  {
	input++;
  	float f=VCF_CUTOFF
	+LFO1_VALUE*LFO1_VCF_CUTOFF
	+LFO2_VALUE*LFO2_VCF_CUTOFF
	+ENV1_VALUE*ENV1_VCF_CUTOFF
	+ENV2_VALUE*ENV2_VCF_CUTOFF;
  
	float q=VCF_RESONANCE
	+LFO1_VALUE*LFO1_VCF_RESONANCE
	+LFO2_VALUE*LFO2_VCF_RESONANCE
	+ENV1_VALUE*ENV1_VCF_RESONANCE
	+ENV2_VALUE*ENV2_VCF_RESONANCE;
	
	if (f<0.05f)f=0.05f;
	if (f>0.90f)f=0.90f;
	if (q<0.05f)q=0.05f;
	if (q>0.98f)q=0.98f;

  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  sbuf0 = fa * sbuf0 + f * (input + fb * (sbuf0 - sbuf1)); 
  sbuf1 = fa * sbuf1 + f * sbuf0;
  
  return (VCF_TYPE==0?sbuf1:input-sbuf1);  
  }
  else 
  return input;
}

class rFilter{
private: 
	float buffy0;
	float buffy1;

public:

	rFilter()
	{
			buffy0=0;
			buffy0=1;
	};

	float fWork(float input,float f)
	{
	float fa = float(1.0 - f); 
	buffy0 = fa * buffy0 + f * input; 
	buffy1 = fa * buffy1 + f * buffy0;
	return buffy1;  
};

};

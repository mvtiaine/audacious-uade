// SPDX-License-Identifier: LicenseRef-NoiseTrekker2
//
// This is a Public Source Code.[Open Source Project]
//
// Roland TB303 Software Synthesizer Emulation v0.1b:
//
// By Juan Antonio Arguelles Rius - arguru@vermail.net
//
// Please, feel free to make any suggestion, verification,
// correction, to this code, since I dont have the Roland tb303
// nor hardware documentation about this machine.
//
// (C)Tb303 is a trademark from Roland.
//
// Code based on Propellerhead's Rebirth v2
//
struct flag303
{
#ifdef WORDS_BIGENDIAN
	unsigned reserved4_flag:1;
	unsigned reserved3_flag:1;
	unsigned reserved2_flag:1;
	unsigned pause:1;
	unsigned transposedown_flag:1;
	unsigned transposeup_flag:1;
	unsigned accent_flag:1;
	unsigned slide_flag:1;
#else
	unsigned slide_flag:1;
	unsigned accent_flag:1;
	unsigned transposeup_flag:1;
	unsigned transposedown_flag:1;
	unsigned pause:1;
	unsigned reserved2_flag:1;
	unsigned reserved3_flag:1;
	unsigned reserved4_flag:1;
#endif
} __attribute__((packed, aligned(4))); // mvtiaine: ensure consistent alignment/padding

struct para303{
unsigned char enabled;			//Enabled				  UBYTE           0       0x00 = off, 0x01 = on (pattern mode)
unsigned char selectedpattern;	//Selected pattern        UBYTE           1       0x00 to 0x20 (pattern mode)
unsigned char tune;			    //Tune                    UBYTE           2       0x00 to 0x7f (pattern mode)
unsigned char cutoff;			//Cutoff                  UBYTE           3       0x00 to 0x7f (pattern mode)
unsigned char resonance;		//Resonance               UBYTE           4       0x00 to 0x7f (pattern mode)
unsigned char envmod;			//EnvMod                  UBYTE           5       0x00 to 0x7f (pattern mode)
unsigned char decay;		    //Decay                   UBYTE           6       0x00 to 0x7f (pattern mode)
unsigned char accent;		    //Accent                  UBYTE           7       0x00 to 0x7f (pattern mode)
unsigned char waveform;		    //Waveform                UBYTE           8       0x00 = triangle, 0x01 = square (pattern mode)
unsigned char patternlength[32];
unsigned char tone[32][16];
struct flag303 flag[32][16];

    //    32*Pattern (1088 bytes)             9,9+(1*34),9+(2*34),9+(3*34)...
    //    |   Shuffle         UBYTE                   0x00 = off, 0x01 = on
    //    |   Pattern length  UBYTE                   0x01 to 0x10
    //    |
    //    |   16*Step (32 bytes)              11,11+(1*34),11+(2*34),11+(3*34)...
    //    |   |   Tone/pitch  UBYTE                   0x00 to 0x0c
    //    |   |   Flags       BITMASK8                bit 0 = No slide/Slide (0x01)
    //    |   |                                       bit 1 = No accent/Accent (0x02)
    //    |   |                                       bit 2 = Normal/Transpose up (0x04)
    //    |   |                                       bit 3 = Normal/Transpose down (0x08)
    //    \   \                                       bit 4 = Pause/Note (0x10)

};


class gear303
{
public:
	unsigned char tbPattern; // From 0 to 31, 255 - Off
	unsigned char tbLine; // From 0 to 15, 255 - Off
	
	gear303();
	void tbNoteOn(int tbNote,para303 *PARAT303);

	float tbGetSample(void);
	void tbUpdate(para303 *tbpars);	
	bool hpf;
	float tbBuf0;
	float tbBuf1;
	float tbVolume;
	
private:

	float tbFilter(float input,float f,float q);

	// 303 Parameters
	float tbTune;
	float tbCutoff;
	float tbResonance;
	float tbEnvmod;
	float tbDecay;
	float tbAccent;
	float tbSample;
	float tbRealCutoff;
	float tbRealVolume;
	float tbOscSpeedFreak;
    float tbTargetVolume;
	float tbCurrentVolume;
	float tbInnertime;
	// Oscillator variables
	float tbOscPosition;
	float tbOscSpeed;
	
	// Filter Input/Output history
	
	// Waveform Type
	char tbWaveform;
};

	// Constructor, 303 initialization

gear303::gear303()
{
	tbPattern=255;
	tbLine=255;
	tbCurrentVolume=0.5;
	tbVolume=0.5f;
	tbRealVolume=0.5f;
	tbInnertime=0.0f;
	tbTargetVolume=0.5f;
	tbRealCutoff=0.5f;
	tbTune=0.0f;
	tbCutoff=0.5f;
	tbResonance=0.5f;
	tbEnvmod=0.5f;
	tbDecay=0.5f;
	tbAccent=0.5f;
	
	tbBuf0=0.0f;
	tbBuf1=0.0f;
	tbOscSpeedFreak=0.0f;

	tbSample=0.0f;
	tbOscPosition=0.0f;
	tbOscSpeed=0.0f;
	hpf=false;
	tbWaveform=0;
}

	// Update parameters from a pointer to a 'para303' structure

void gear303::tbUpdate(para303 *tbpars)
{
	tbTune=((float)tbpars->tune-64.0f)*0.015625f;
	tbCutoff=(float)tbpars->cutoff*0.0078125f;
	tbResonance=(float)tbpars->resonance*0.0078125f;
	tbEnvmod=(float)tbpars->envmod*0.0078125f;
	tbDecay=(float)tbpars->decay*0.0078125f;
	tbAccent=(float)tbpars->accent*0.0078125f;
	tbWaveform=tbpars->waveform;
}

	// Render 1 32bit-float sample

float gear303::tbGetSample(void)
{
		// Get Oscillator values
	
	switch(tbWaveform)
	{
	
		// SawTooth
	case 0:
		tbSample=tbOscPosition;
	break;

		// Square
	case 1:
		if(tbOscPosition<0)
			tbSample=-16384;
		else
			tbSample=16384;
	break;
	}

	if (tbCurrentVolume<tbTargetVolume)
		tbCurrentVolume+=0.0078125f;
	else
		tbCurrentVolume-=0.0078125f;

		// Run Oscillator

	tbOscPosition+=tbOscSpeed;
	
	if(tbInnertime>0)
	{
		tbInnertime--;
		tbOscSpeed+=tbOscSpeedFreak;
	}
	
	if(tbOscPosition>=16384)tbOscPosition-=32768;
	
	tbRealCutoff=tbCutoff+tbEnvmod;
	tbEnvmod-=tbDecay*tbRealCutoff*0.015625f;

	if(tbRealCutoff<tbCutoff)tbRealCutoff=tbCutoff;
	tbSample*=tbCurrentVolume;
	return tbFilter(tbSample,tbRealCutoff,tbResonance);
}

	// Do Note On

void gear303::tbNoteOn(int tbNote,para303 *PARAT303)
{
	if(PARAT303->flag[tbPattern][tbLine].transposeup_flag)tbNote+=12;
	if(PARAT303->flag[tbPattern][tbLine].transposedown_flag)tbNote-=12;
	tbWaveform=PARAT303->waveform;	
	tbOscSpeedFreak=0;
	float frune=float(tbNote)-17;
	frune+=(float)PARAT303->tune*0.1889763f;
	tbOscSpeed=(float)pow(2.0,frune/12.0f)*64;
	
	if(PARAT303->flag[tbPattern][tbLine].pause)
	{
	tbCutoff=float(PARAT303->cutoff+1)*0.0026041f;
	tbEnvmod=(tbCutoff*2)+(float)PARAT303->envmod*0.0009531f;
	tbResonance=(float)PARAT303->resonance*0.0078125f;
	tbDecay=(128.0f-(float)PARAT303->decay)*0.000122f;
	tbRealVolume=tbVolume;
	}
		
	// Slide check...
	
	// Hay glide? no hay decay...
	if(PARAT303->flag[tbPattern][tbLine].slide_flag)tbDecay=0.0f;

	// Aqui se mira el glide de atras...

	char tbLine2=tbLine > 0 ? tbLine-1 : 0; // mvtiaine: fixed OOB
	if(tbLine<0)tbLine=PARAT303->patternlength[tbPattern]-1;
	bool forcefault=true;
	if(PARAT303->flag[tbPattern][tbLine2].slide_flag)
	{
	forcefault=false;
	frune=float(PARAT303->tone[tbPattern][tbLine2])-17;
	if(PARAT303->flag[tbPattern][tbLine2].transposeup_flag)frune+=12;
	if(PARAT303->flag[tbPattern][tbLine2].transposedown_flag)frune-=12;
	frune+=(float)PARAT303->tune*0.1889763f;
	tbInnertime=SamplesPerTick*0.5f;
	float tbDestiny=tbOscSpeed; // Velocidad Destino
	float tbSource=((float)pow(2.0,frune/12.0f)*64); // Velocidad fuente
	tbOscSpeed=tbSource; // Intercambioce....
	tbOscSpeedFreak=(tbDestiny-tbSource)/tbInnertime; // Calculo del coeficiente del glide
	}

	if(PARAT303->flag[tbPattern][tbLine].accent_flag)
	{	
		float accenta=(float)PARAT303->accent*0.0001765f;
		tbResonance+=accenta;
		tbCutoff+=accenta;
		tbRealVolume*=((accenta*64.0f)+1.0f);
	}

	if(!PARAT303->flag[tbPattern][tbLine].pause && forcefault)
		tbRealVolume=0;

	tbTargetVolume=tbRealVolume;
	
}

//
// Filter routine.
//
// This is a 2pole filter [-12db/Octave] Lowpass filter.
// It seems that the original Roland Tb303 has got a 3pole filter.
//

float gear303::tbFilter(float input,float f,float q)
{
  input++;
  if(f>0.999f)f=0.999f;
  if(f<0.005f)f=0.005f;
  if(q>0.98f)q=0.98f;
  if(q<0.01f)q=0.01f;

  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  tbBuf0 = fa * tbBuf0 + f * (input + fb * (tbBuf0 - tbBuf1)); 
  tbBuf1 = fa * tbBuf1 + f * tbBuf0;
  return (hpf?input-tbBuf1:tbBuf1);
}

void RESET303PARAMETERS(para303 *tbpars)
{
tbpars->enabled=0;
tbpars->selectedpattern=0;
tbpars->tune=64;
tbpars->cutoff=64;
tbpars->resonance=64;
tbpars->envmod=64;
tbpars->decay=64;
tbpars->accent=64;
tbpars->waveform=0;

for (char c=0;c<32;c++)
{
	tbpars->patternlength[c]=16;
	
	for (char d=0;d<16;d++)
	{
	tbpars->tone[c][d]=0;
	tbpars->flag[c][d].slide_flag=0;
	tbpars->flag[c][d].accent_flag=0;
	tbpars->flag[c][d].transposeup_flag=0;
	tbpars->flag[c][d].transposedown_flag=0;
	tbpars->flag[c][d].pause=1;
	tbpars->flag[c][d].reserved2_flag=0;
	tbpars->flag[c][d].reserved3_flag=0;
	tbpars->flag[c][d].reserved4_flag=0;
	}
}

}

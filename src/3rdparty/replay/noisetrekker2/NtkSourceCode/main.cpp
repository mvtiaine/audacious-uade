// SPDX-License-Identifier: LicenseRef-NoiseTrekker2
/* The Original && Best, NoiseTrekker Main Source Code [Part 1]*/

// Includes ----------------------------------------------------------
#ifndef AUDACIOUS_UADE
#include <stdio.h>
#include <CON_All.h>
#include <windows.h>
#include <UTIL_Filters.h>
#include <io.h>
#include <direct.h>
#include <mmsystem.h>
#include "riff.cpp"
#include "resource.h"
#include "Ntkfunctions.h"
#include "Alphatrack.cpp"
#include "tb303.cpp"
#include "cubicspline.cpp"
#endif

// Definitions -------------------------------------------------------

#define delay_size 22100
#define MAX_COMB_FILTERS 10
#define FULLYESNO 0
#define MAXLOOPTYPE 1
#define MAX_TRACKS 16
#define MAX_FILTER 23
#define PBLEN 1572864

// mvtiaine: added big endian support
#define SWAP16(value) \
((uint16_t)( \
	((uint16_t)(value) << 8) | \
	((uint16_t)(value) >> 8) \
))
#define SWAP32(value) \
((uint32_t)( \
	((uint32_t)(value) << 24) | \
	(((uint32_t)(value) & 0x0000FF00U) << 8) | \
	(((uint32_t)(value) & 0x00FF0000U) >> 8) | \
	((uint32_t)(value) >> 24) \
))

size_t fread_swap(void *ptr, size_t size, size_t n, FILE *stream) {
#ifdef WORDS_BIGENDIAN
	if (size == 2) {
		uint16_t *data = (uint16_t *)ptr;
		for (size_t i = 0; i < n; ++i) {
			uint16_t value;
			if (fread(&value, size, 1, stream) != 1) return i;
			*data++ = SWAP16(value);
		}
		return n;
	} else if (size == 4) {
		uint32_t *data = (uint32_t *)ptr;
		for (size_t i = 0; i < n; ++i) {
			uint32_t value;
			if (fread(&value, size, 1, stream) != 1) return i;
			*data++ = SWAP32(value);
		}
		return n;
	} else {
		return fread(ptr, size, n, stream);
	}
#else
	return fread(ptr, size, n, stream);
#endif
}


// Global Variables --------------------------------------------------

FILE *HDSTREAM;

unsigned char freakylines=64;

static unsigned char CompressedPointerBitmap[] = {
0x78, 0x9C, 0x8D, 0xD2, 0x31, 0x12, 0x80, 0x20, 0x0C, 0x04, 0xC0, 0xEB, 0x2D, 0x6D, 0x7C, 0x2A,
0x85, 0x0F, 0xA3, 0xF0, 0x61, 0x29, 0x2C, 0x8C, 0x03, 0xC6, 0x8C, 0x1E, 0xA7, 0x19, 0xE8, 0x76,
0x80, 0xE4, 0x98, 0x00, 0xCC, 0xBE, 0x17, 0xF4, 0x32, 0xB1, 0xCA, 0x5A, 0xB4, 0x71, 0x50, 0xB7,
0x2A, 0x4C, 0x03, 0xC2, 0x38, 0xB0, 0xDD, 0x84, 0xC9, 0x80, 0x9A, 0x06, 0x84, 0x79, 0x83, 0x87,
0x09, 0x30, 0x32, 0x23, 0x10, 0x26, 0x5F, 0x01, 0x56, 0xD1, 0xC5, 0xA9, 0x6F, 0xC6, 0x27, 0x99,
0x31, 0xCD, 0x82, 0x02, 0xDA, 0xF5, 0x9F, 0xC9, 0xEB, 0xF8, 0x3E, 0x0F, 0xD1, 0xF9, 0x5E, 0x0F,
0xEB, 0x9F, 0xED, 0x00, 0x3E, 0x8F, 0x08, 0x72 };

gear303 tb303engine[2];
para303 tb303[2];
unsigned char track3031=0;
unsigned char track3032=0;
unsigned char sl3=0;
int	currentCounter=0;

// SAMPLE COUNTER
struct smpos
{
#ifdef WORDS_BIGENDIAN
	__uint32 first;
	__uint32 last;
#else
	__uint32 last;
	__uint32 first;
#endif
};

union s_access
{
	smpos half;
	__uint64 absolu;
};

int clipc=0;
int	delayedCounter=0;
int	delayedCounter2=0;
int	delayedCounter3=0;
int	delayedCounter4=0;
int	delayedCounter5=0;
int	delayedCounter6=0;

int Lscope[128];
int Rscope[128];
int Cscope=0;
int CONSOLE_WIDTH=800;
int CONSOLE_HEIGHT=600;
int CONSOLE_HEIGHT2=600;
unsigned char tbEditStep=0;
int fluzy=-1;
char GUIMODE=0;
char act303=0;
float REVERBFILTER=0.3f;
bool Scopish=true;
bool AMIMODE=false;
int delays[MAX_COMB_FILTERS]; // delays for the comb filters
int counters[MAX_COMB_FILTERS];
int rev_counter=0;
int rev_counter2=0;
char QUALITYPLAY=1;
char visiblecolums=0;
int rs_coef=32768;
float decays[MAX_COMB_FILTERS][2];
float delay_left_buffer[MAX_COMB_FILTERS][100000];
float delay_right_buffer[MAX_COMB_FILTERS][100000];
char num_echoes=1;
	
int gui_lx=0;
int gui_ly=0;
char gui_pushed=0;
char teac=0;
int liveparam=0;
int livevalue=0;
int sed_display_start=0;
int sed_display_length=0;
int sed_range_start=0;
int sed_range_end=0;
bool sed_range_mode=false;
int csynth_slv=0;
bool fld_chan=false;
bool actlogo=false;
bool QUALITYCHANGE=true;
int VIEWLINE=7; // BOOKMARK
int VIEWLINE2=-6;
int YVIEW=244; 
char is_editing=0;
int poskeynote=0;
int poskeyval;
bool trkchan=true;
int pos_space=0;
int glide=0;			
int multifactor=4;
int left_value=0;
int right_value=0;
int clrc=0;
char seditor=0;
char tipoftheday[256];
int ctipoftheday=0;
char ped_split=0;
float oldspawn[MAX_TRACKS];
float roldspawn[MAX_TRACKS];
char FADEMODE[MAX_TRACKS]; // 0 - Off, 1- In, 2 - Out;
float FADECOEF[MAX_TRACKS];

CSynth Synthesizer[MAX_TRACKS];
Cubic Resampler;

int compressor=0; // 0-->Off 1-->On
int b_buff_xsize=0;
int b_buff_ysize=0;
int player_pos=-1;
int xew=0;
bool sas=false;
int flagger=0;
int LastX=0;
int LastY=0;
int shuffleswitch=0;
int ltretnote=0;
int ltretvalue=0;
int L_MaxLevel=0;
int R_MaxLevel=0;
int retletter=0;
int tretletter=0;
int posletter=0;
Screen* S;
bool poslad=true;
int shufflestep=0;
int lt_items=0;
int lt_index=0;
int last_index=-1;
int gco=0;
int lt_curr=0;
int block_start_track=-1;
int block_end_track=-1;
int block_start=0;
int block_end=0;
int lt_ykar=200;
int gui_action=255;
char SMPT_LIST[2048][64];
bool SACTIVE[256][16];
unsigned FILETYPE[2048];
bool grown=false;
unsigned int Currentpointer=0;
__uint32 res_dec=0;
int ped_row=0;
int ped_line=0;
char appbuffer[_MAX_PATH];
char CS_PAR_NAME[52][24]; // CSynth Parameters Names
int ped_track=0;
int gui_track=0;
int xoffseted;
float gr=0;
float currsygnal=0;
float currsygnal2=0;
float synthsygnal=0;

int po_ctrl=0;
char userscreen=0;
int XLATENCY_TIME=50;
float left_float=0;
float right_float=0;
float left_chorus=0;
float right_chorus=0;
int shuffle=0;
int c_r_release=0;
int c_l_release=0;
short *RawSamples[128][2][16];
short *Player_WL[MAX_TRACKS];
short *Player_WR[MAX_TRACKS];

float allBuffer_L[5760];
float allBuffer_L2[5760];
float allBuffer_L3[5760];
float allBuffer_L4[5760];
float allBuffer_L5[5760];
float allBuffer_L6[5760];

short patternLines[128];
char SampleChannels[128][16];
char Player_SC[MAX_TRACKS];

char SampleType[128][16];
int LoopStart[128][16]; // mvtiaine: long -> int
int LoopEnd[128][16]; // mvtiaine: long -> int
unsigned int Player_LS[MAX_TRACKS];
unsigned int Player_LE[MAX_TRACKS];
unsigned int Player_LL[MAX_TRACKS];
unsigned int Player_NS[MAX_TRACKS];

SynthParameters PARASynth[128];
char LoopType[128][16];
char Player_LT[MAX_TRACKS];
int SampleNumSamples[128][16]; // mvtiaine: long -> int
char Finetune[128][16];
char SampleName[128][16][64];
float SampleVol[128][16];
float CustomVol[128];
float Player_SV[MAX_TRACKS];
float FDecay[128][16];
float Player_FD[MAX_TRACKS];
char Basenote[128][16];
char nameins[128][20];
bool beatsync[128];
short beatlines[128];

char Midiprg[128];
bool Synthprg[128];

char Patbreak=127; // 127 when no jump or yes on patbreak<64 = line to jump.

unsigned char *RawPatterns;
unsigned char *BuffTrack;
unsigned char *BuffPatt;
unsigned char *BuffBlock;
char name[20];
char artist[20];
char style[20];

unsigned char pSequence[256];
unsigned char cPosition=0;
unsigned char sLength=1;
unsigned char nPatterns=1;

int ped_pattad=1;
int ped_patoct=4;
int ped_patsam=1;
int ped_synthpar=1;
unsigned int Rns[MAX_TRACKS];
char FLANGER_ON[MAX_TRACKS];
float FLANGER_AMOUNT[MAX_TRACKS];
float FLANGER_DEPHASE[MAX_TRACKS];
float FLANGER_RATE[MAX_TRACKS];
float FLANGER_AMPL[MAX_TRACKS];
float FLANGER_GR[MAX_TRACKS];
float FLANGER_FEEDBACK[MAX_TRACKS];
int FLANGER_DELAY[MAX_TRACKS];
int FLANGER_OFFSET[MAX_TRACKS];
float foff2[MAX_TRACKS];
float foff1[MAX_TRACKS];
float FLANGE_LEFTBUFFER[MAX_TRACKS][16400];
float FLANGE_RIGHTBUFFER[MAX_TRACKS][16400];
int LastProgram[MAX_TRACKS];
char sp_channelsample[MAX_TRACKS];
char sp_split[MAX_TRACKS];
int TRACKSTATE[MAX_TRACKS]; // 0->Normal 1->Muted
s_access sp_Position[MAX_TRACKS];
float sp_Cvol[MAX_TRACKS];
float sp_Tvol[MAX_TRACKS];
int sp_Stage[MAX_TRACKS];
char LFO_ON[MAX_TRACKS];
float LFORATE[MAX_TRACKS];
float LFOAMPL[MAX_TRACKS];
float LFOGR[MAX_TRACKS];
int restx=0;
int resty=0;
int fsize=0;
bool draw_sampled_wave=false;
bool draw_sampled_wave2=false;
bool draw_sampled_wave3=false;

__int64 sp_Step[MAX_TRACKS];
__int64 Vstep1[MAX_TRACKS];
__int64 glidestep[MAX_TRACKS];
float ramper[MAX_TRACKS];
float fx1[MAX_TRACKS];
float fx2[MAX_TRACKS];
float fy1[MAX_TRACKS];
float fy2[MAX_TRACKS];
float xi0[MAX_TRACKS];
float xi1[MAX_TRACKS];
float xi2[MAX_TRACKS];
float rx1,rx2,ry1,ry2;
float rx12,rx22,ry12,ry22;
int CSend[MAX_TRACKS];
float CCoef[MAX_TRACKS];
float coef[5];
float coeftab[5][128][128][4];
float buf0[MAX_TRACKS],buf1[MAX_TRACKS];
float buf024[MAX_TRACKS],buf124[MAX_TRACKS];
int redraw_everything=0;
int pl_note=0;
int pl_sample=0;	
int pl_vol_row=0;
int pl_pan_row=0;
bool boing=false;
int pl_eff_row=0;
int pl_dat_row=0;
int LastPedRow=-1;
int Subicounter=0;
bool rawrender=false;
bool po_ctrl2=true;
int TRACKMIDICHANNEL[MAX_TRACKS];
float CCut[MAX_TRACKS];
float TCut[MAX_TRACKS];
float ICut[MAX_TRACKS];
float TPan[MAX_TRACKS];
float LVol[MAX_TRACKS];
float RVol[MAX_TRACKS];

int FType[MAX_TRACKS];
int FRez[MAX_TRACKS];
float DThreshold[MAX_TRACKS];
float DClamp[MAX_TRACKS];
bool Disclap[MAX_TRACKS];
bool Dispan[MAX_TRACKS];
float DSend[MAX_TRACKS];	
int sp_TickCounter=0;
int LVColor=0;
int RVColor=0;
int BeatsPerMin=125;
int TicksPerBeat=4;
char Songtracks=8;
int SamplesPerSec=44100;
unsigned int SamplesPerSub=0;
unsigned int SubCounter=0;
unsigned int PosInTick=0;
int Songplaying=0;
int DelayType=0;
int player_line=0;
char actuloop=0;
int namesize=8;
int plx=0;
float delay_left_final=0;
float delay_right_final=0;
float Feedback=0.6f;
float Feedback2=0.5f;
float Feedback3=0.45f;
float Feedback4=0.4f;
float Feedback5=0.53f;
float Feedback6=0.46f;

int delay_time=0;
float mas_vol=1.0f;

float lbuff_chorus[131072];
float rbuff_chorus[131072];
float lchorus_feedback=0.6f;
float rchorus_feedback=0.5f;
int lchorus_delay=10584;
int rchorus_delay=15876;
int c_l_tvol=32768;
int c_r_tvol=32768;
int c_l_cvol=32768;
int c_r_cvol=32768;
int c_threshold=32;
int mlimit=0;
long axswave=0;
unsigned int lchorus_counter=44100;
unsigned int rchorus_counter=44100;
unsigned int lchorus_counter2=44100-lchorus_delay;
unsigned int rchorus_counter2=44100-rchorus_delay;

int hd_isrecording=0;
char sr_isrecording=0;

int po_shift=0;
int snamesel=0;
#ifndef AUDACIOUS_UADE
signed char n_midioutdevices=0;
signed char n_midiindevices=0;
MIDIINCAPS caps_midiin[255];
MIDIOUTCAPS caps_midiout[255];
HMIDIOUT midiout_handle=NULL;
HMIDIIN midiin_handle=NULL;
Bitmap* MOUSEBACK;
Bitmap* NTKLOGO;
Bitmap* SKIN303;
Bitmap* KNOB1;
Bitmap* PFONT;
Bitmap* FRAMEWORK;

HDC dc;
HFONT fnt;
HFONT old_f;
MEMORYSTATUS memstate;
#endif // AUDACIOUS_UADE
rFilter LFP_L;
rFilter LFP_R;

bool midiin_changed=false;
bool midiout_changed=false;

signed char c_midiin=-1;
signed char c_midiout=-1;
/*
ok, make sure your exit function is something like:

int postAction(Console* C);

and don't call it directly.  instead add:
  setPostAction(postAction);

to the initConsole

This will attach the function to the library exit code, and will be executed automatically
*/
// Initializing Console -----------------------------------------
#ifndef AUDACIOUS_UADE
int initConsole(int& Width, int& Height, int& FullScreen, int& Flags, Screen* S)
{
LoadSettings();				/* Load configuration file */
restx=CONSOLE_WIDTH-640;
resty=CONSOLE_HEIGHT-480;
CONSOLE_HEIGHT2=CONSOLE_HEIGHT-22;
mlimit=619+restx;
fsize=638+restx;
visiblecolums=CONSOLE_WIDTH/128;
ctipoftheday=rand() % 15;	
switch(ctipoftheday){
case 0:sprintf(tipoftheday,"Tip Of The Day: Pressing CTRL+I will interpolate effect value on the marked block.");break;
case 1:sprintf(tipoftheday,"Tip Of The Day: The right mouse button will have a secondary action on most buttons.");break;
case 2:sprintf(tipoftheday,"Tip Of The Day: Dont set excesive track reverb send values, to get better quality.");break;
case 3:sprintf(tipoftheday,"Tip Of The Day: Use CTRL+Z to paste only effect data of the marked block.");break;
case 4:sprintf(tipoftheday,"Remember: MIDI is not audio, realtime fx will no affect to midi sound.");break;
case 5:sprintf(tipoftheday,"'ArGuru': I hate Cubase...");break;
case 6:sprintf(tipoftheday,"Tip Of The Day: Get very-latest 'NoiseTrekker' versions at 'http://agarrense.freeservers.com'.");break;
case 7:sprintf(tipoftheday,"Tip Of The Day: On lower CPUs, you can renderize patterns to wav, and use them as samples without any loss of quality.");break;
case 8:sprintf(tipoftheday,"Tip Of The Day: Volume note-cut command 'F?' is very useful to avoid sample-clicking.");break;
case 9:sprintf(tipoftheday,"Tip Of The Day: Left-Clicking on pattern editor will mute/unmute any track.");break;
case 10:sprintf(tipoftheday,"Tip Of The Day: Pattern command '16xx' will reset the Filter LFO of the track. No parameter required.");break;
case 11:sprintf(tipoftheday,"Tip Of The Day: With a '90' value on the panning column you can change midi controllers values.");break;
case 12:sprintf(tipoftheday,"Tip Of The Day: Get lastest music-software stuff at www.maz-sound.de.");break;
case 13:sprintf(tipoftheday,"Tip Of The Day: Official 'NoiseTrekker' site by Waka-X at http://twnn.cjb.net.");break;
case 14:sprintf(tipoftheday,"Tip Of The Day: Pressing Left-Shift will 'key-repeating' while editing.");break;

default:sprintf(tipoftheday,"Tip Of The Day: See html doc included to see help and pattern commands.");break;
}

L_MaxLevel=0;
R_MaxLevel=0;

   if (!S->isModeAvailable(CONSOLE_WIDTH,CONSOLE_HEIGHT))
   {
	   MessageBox(NULL,"Screen resolution is not supported on your gfxcard","Error!",MB_OK);
    return -1;
  }

  HICON icon=(HICON)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_UFO),IMAGE_ICON,0,0,0);
  setWindowIcon(icon);
  setWindowTitle("NoiseTrekker");
  Width=CONSOLE_WIDTH;
  Height=CONSOLE_HEIGHT;
  
  if (GUIMODE)
  {FullScreen=0;Flags=WindowsFrame+SingleBuffer;}
  else
  {FullScreen=1;Flags=SingleBuffer;}
  
  sprintf(name,"Untitled");
  sprintf(artist,"Somebody");
  sprintf(style,"Goa Trance");

  namesize=8;
  IniCsParNames();
  for (unsigned int listcleaner=0;listcleaner<2048;listcleaner++){for (int listcleaner2=0;listcleaner2<64;listcleaner2++){SMPT_LIST[listcleaner][listcleaner2]=0;}}
  
  Read_SMPT();
  init_sample_bank();
  BuffPatt=(unsigned char *)malloc(12288);
  BuffTrack=(unsigned char *)malloc(768);
  BuffBlock=(unsigned char *)malloc(12288);
  
  for (int ipcut=0;ipcut<360;ipcut++)
	SIN[ipcut]=(float)sin((float)ipcut*0.0174532f);
  
  for (ipcut=0;ipcut<384;ipcut+=6)
  {
    *(BuffTrack+ipcut)=121;
	*(BuffTrack+ipcut+1)=255;
	*(BuffTrack+ipcut+2)=255;
	*(BuffTrack+ipcut+3)=255;
	*(BuffTrack+ipcut+4)=0;
	*(BuffTrack+ipcut+5)=0;
  }	

  for (ipcut=0;ipcut<12288;ipcut+=6)
	{
	*(BuffPatt+ipcut)=121;
	*(BuffPatt+ipcut+1)=255;
	*(BuffPatt+ipcut+2)=255;
	*(BuffPatt+ipcut+3)=255;
	*(BuffPatt+ipcut+4)=0;
	*(BuffPatt+ipcut+5)=0;
	*(BuffBlock+ipcut)=121;
	*(BuffBlock+ipcut+1)=255;
	*(BuffBlock+ipcut+2)=255;
	*(BuffBlock+ipcut+3)=255;
	*(BuffBlock+ipcut+4)=0;
	*(BuffBlock+ipcut+5)=0;
	}

  float ve=0;

Initreverb();

setPostAction(postAction);  
  return 0;
}

// Work Function ------------------------------------------------

class SineWave : public InputFilter
{
public:
virtual int Read(char* Buffer, int Len)
{
short *pSamples=(short *)Buffer;

if(!rawrender)
{
	int i;
	switch(QUALITYPLAY){
	case 0:
	for(i=Len-1;i>=0;i-=4){
	GetPlayerValues(mas_vol);
	*pSamples++=left_value;
    *pSamples++=right_value;
	if (left_value>L_MaxLevel)L_MaxLevel=left_value;
	if (right_value>R_MaxLevel)R_MaxLevel=right_value;
	Lscope[Cscope]=left_value;
	Rscope[Cscope]=right_value;
	if(left_value<-32760 || left_value>32760 || right_value<-32760 || right_value>32760)clipc=0xFFDD;
	Cscope++;
	if (Cscope>127){Cscope=0;
	if(L_MaxLevel>128)L_MaxLevel-=128;
	if(R_MaxLevel>128)R_MaxLevel-=128;}}
	break;

	case 1:
	for(i=Len-1;i>=0;i-=4){
	GetPlayerValues2(mas_vol);
	*pSamples++=left_value;
    *pSamples++=right_value;   
	if(left_value<-32760 || left_value>32760 || right_value<-32760 || right_value>32760)clipc=0xFFDD; 
	if (left_value>L_MaxLevel)L_MaxLevel=left_value;
	 if (right_value>R_MaxLevel)R_MaxLevel=right_value;
	 if(L_MaxLevel)L_MaxLevel--;
	 if(R_MaxLevel)R_MaxLevel--;}
	break;

	case 2:
	for(i=Len-1;i>=0;i-=4){
	GetPlayerValues3(mas_vol);
	*pSamples++=left_value;
    *pSamples++=right_value;
	if(left_value<-32760 || left_value>32760 || right_value<-32760 || right_value>32760)clipc=0xFFDD;
	 if (left_value>L_MaxLevel)L_MaxLevel=left_value;
	 if (right_value>R_MaxLevel)R_MaxLevel=right_value;
	 if(L_MaxLevel)L_MaxLevel--;
	 if(R_MaxLevel)R_MaxLevel--;}
	break;
}// SWITCH
}//RawRender
//if(hd_isrecording){for(int wo=0;wo<Len;wo++)fputc(*(Buffer+wo),HDSTREAM);}
	
return Len;
}

}; // CLASS SINEWAVE

SoundStream* ss=NULL;
SineWave*    sw=NULL;

// Action Loop .-------------------------------------------------

int action(Console* C)
{
	
S=C->getScreen();
redraw_everything=0;

if (CON_Reactivated())  // screen has been erased
{
redraw_everything=1;

MemoryInputStream mis((char*)CompressedPointerBitmap,sizeof(CompressedPointerBitmap));
InflateStream is(mis);
Bitmap* BM=newBitmap(is);
TColor Color=C->getScreen()->getColor(0,0,0xf8);
BM->setTransparentColor(Color);
C->getMouse()->setPointer(BM,1,1);
RELEASEINT(BM);
}

  if (ss==NULL)
  {
	MidiGetAll();				/* Retrieves Midi Info */
	Iniplayer();				/* Player initialization */
    sw=new SineWave();
    ss=newSoundStream(sw,44100,16,2,1);
    ss->setFrameSize(10);
	ss->setStreamDelay(XLATENCY_TIME);
	ss->play();
	ss->setVolume(0);
	S->setGammaFade(1.0f,1.0f,1.0f);
	MOUSEBACK=newBitmap(12,20,0);
    NTKLOGO=loadBMP("skin/ntk.bmp");
    SKIN303=loadBMP("skin/ntk303.bmp");
    KNOB1=loadBMP("skin/knob1.bmp");
    PFONT=loadBMP("skin/patternfont.bmp");
    FRAMEWORK=loadBMP("skin/framework2.bmp");

	C->getSound()->setPreferences(44100,16,2);
  }

  if (clrc==0)
  {
  S->clear(0);
  MOUSEBACK->clear(0);
  clrc=1;
  }

  if (S->lock()==0)
  {

	if (userscreen==4 && Scopish)DrawScope();
	
	if (sp_Stage[ped_track] && ped_patsam==sp_channelsample[ped_track] && ped_split==sp_split[ped_track])
	{draw_sampled_wave2=true;
	boing=true;
	}
	else if (boing)
	{
		boing=false;
		draw_sampled_wave3=true;
	}

	
	Draw_Sampled_Wave();
	Draw_Sampled_Wave2();
	Draw_Sampled_Wave3();

  	int Lt_vu=80+L_MaxLevel/64;
	int Rt_vu=80+R_MaxLevel/64;

	if(redraw_everything==1)
	{
	for(int nx=0;nx<CONSOLE_HEIGHT;nx++)
	S->drawHLine(nx,0,CONSOLE_WIDTH,0);
	}

	S->drawHLine(11,80,Lt_vu,0xDD00);
	S->drawHLine(12,80,Lt_vu,0xFF00);
	
	S->drawHLine(17,80,Rt_vu,0xDD00);
	S->drawHLine(18,80,Rt_vu,0xFF00);
	
	S->drawHLine(11,Lt_vu,584,clipc);
	S->drawHLine(12,Lt_vu,584,clipc);
	
	S->drawHLine(17,Rt_vu,584,clipc);
	S->drawHLine(18,Rt_vu,584,clipc);
	
	if (actuloop)Afloop();

	S->unlock();

	
  }

  if (gui_action!=0)vloid();
  if (S->beginDraw()==0)
  {
	dc=(HDC)S->getDeviceContext();
	fnt=CreateFont(8,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"MS Sans Serif");
	old_f=(HFONT)SelectObject(dc,fnt);

if (gui_action!=0) // There are some for me today.....:)
{
	if (gui_action==1)
	{
		lt_ykar=LastY-72;
		ltActualize(0);
	}

	if (gui_action==2)
	{
		int broadcast=lt_index+(LastY-44)/12;
		last_index=-1;
		if (broadcast!=lt_curr)
		{
			lt_curr=broadcast;
			ltActualize(1);
		}
		else
		{
			if (FILETYPE[lt_curr]!=_A_SUBDIR)
			{
			ss->stop();
			StopIfSp();
			WavAlloc(ped_patsam,SMPT_LIST[lt_curr]);
			ss->play();
			}
			else
			{
				_chdir(SMPT_LIST[lt_curr]);
				Read_SMPT();
				ltActualize(0);

			}
			
		}
	}

	if (gui_action==3){cPosition--;Actualize_Sequencer();Actupated(0);}
	if (gui_action==4){cPosition++;Actualize_Sequencer();Actupated(0);}
	if (gui_action==5){Actualize_Sequencer();Actupated(0);}
	if (gui_action==7){pSequence[cPosition]--;Actualize_Sequencer();Actupated(0);Anat(cPosition);}
	if (gui_action==8){pSequence[cPosition]++;Actualize_Sequencer();Actupated(0);Anat(cPosition);}
	if (gui_action==9){sLength--;Actualize_Sequencer();}
	if (gui_action==10){sLength++;Actualize_Sequencer();}
	if (gui_action==11){Actualize_Sequencer();}
	
	if (gui_action==15){Actualize_Track_Ed(0);Actualize_Lfo_Ed(0);}
	if (gui_action==16){Actualize_Track_Ed(0);Actualize_Lfo_Ed(0);}

	if (gui_action==21){ped_patoct--;Actualize_Patterned();}
	if (gui_action==22){ped_patoct++;Actualize_Patterned();}
	if (gui_action==23){ped_pattad--;Actualize_Patterned();}
	if (gui_action==24){ped_pattad++;Actualize_Patterned();}
	
	if (gui_action==25)
	{
	ped_patsam--;

	if(snamesel==2)
		snamesel=0;
	
	Actualize_Patterned();
	RefreshSample();
	NewWavy();
	Actualize_Wave_Ed(0);
	if (userscreen==6)
	Actualize_Midi_Ed(1);
	}
	
	if (gui_action==26)
	{
		ped_patsam++;
		
		if(snamesel==2)
			snamesel=0;
		
		Actualize_Patterned();
		RefreshSample();
		NewWavy();
		Actualize_Wave_Ed(0);
		
	if (userscreen==6)
	Actualize_Midi_Ed(1);
	}
	
	if (gui_action==27)
	{
	int tmp_track=gui_track+((LastX-24)/118);
	
	if (tmp_track>15)tmp_track=15;
	if (tmp_track<0)tmp_track=0;

	if (TRACKSTATE[tmp_track]==0)
	TRACKSTATE[tmp_track]=1;
	else
	TRACKSTATE[tmp_track]=0;
	
	if(userscreen==1)Actualize_Track_Ed(10);
	Actupated(0);
	}

	

	if (gui_action==28){SongPlay();}
	if (gui_action==29){SongStop();Actupated(0);}
	if (gui_action==30){StartRec();}
	if (gui_action==40){Actualize_Master(teac);}
	if (gui_action==50){TCut[ped_track]=float(LastX-88);Actualize_Track_Ed(1);liveparam=1;livevalue=(LastX-88)*2;}   
	if (gui_action==51){FRez[ped_track]=LastX-88;Actualize_Track_Ed(2);liveparam=2;livevalue=(LastX-88)*2;}
	if (gui_action==52){ICut[ped_track]=(LastX-88.0f)*0.00006103515625f;Actualize_Track_Ed(3);}
	if (gui_action==53){Actualize_Track_Ed(teac);}
	if (gui_action==54){DThreshold[ped_track]=float((LastX-318)*512);Actualize_Track_Ed(7);liveparam=4;livevalue=(LastX-318)*2;}
	if (gui_action==55){DClamp[ped_track]=float((LastX-318)*512);Actualize_Track_Ed(8);liveparam=5;livevalue=(LastX-318)*2;}
	if (gui_action==56){DSend[ped_track]=float(((float)LastX-318)/128.0);Actualize_Track_Ed(5);liveparam=3;livevalue=(LastX-318)*2;}
	if (gui_action==57){TPan[ped_track]=((float)LastX-318)/128.0f;Actualize_Track_Ed(9);liveparam=6;livevalue=LastX-318;}
	if (gui_action==60){userscreen=0;draw_mained();Actualize_Main_Ed();}
	if (gui_action==61){userscreen=1;draw_tracked();Actualize_Track_Ed(0);}
	if (gui_action==62){userscreen=2;draw_sampleed();Actualize_Sample_Ed(2,0);}
	if (gui_action==63){userscreen=3;draw_fxed();Actualize_Fx_Ed(teac);}
	if (gui_action==64){userscreen=4;draw_seqed();Actualize_Seq_Ed();}
	if (gui_action==65){userscreen=5;draw_mastered();Actualize_Master_Ed(0);}
	if (gui_action==66){userscreen=6;draw_midied();Actualize_Midi_Ed(0);}
	if (gui_action==67){userscreen=7;draw_lfoed();Actualize_Lfo_Ed(teac);}
	if (gui_action==68){userscreen=8;draw_exted();}
	if (gui_action==69){draw_back();}
	if (gui_action==70){SaveInst();}
	if (gui_action==71){ShowInfo();}
	if (gui_action==72){userscreen=9;draw_303ed();}
	if (gui_action==73){Actualize_303ed(teac);}

	if (gui_action==100){ltActualize(1);}
	if (gui_action==101){Actualize_Sample_Ed(1,teac);}
	if (gui_action==102){SaveMod();}
	if (gui_action==103){LoadMod();}
	if (gui_action==104){SampleVol[ped_patsam][ped_split]=float((LastX-436)/32.0);Actualize_Sample_Ed(0,1);}
	if (gui_action==105){Finetune[ped_patsam][ped_split]=((LastX-436)-64)*2;Actualize_Sample_Ed(0,2);}
	if (gui_action==106){FDecay[ped_patsam][ped_split]=float(LastX-62)/8192.0f;Actualize_Sample_Ed(0,3);}
	if (gui_action==107){Actualize_Sample_Ed(0,teac);}
	if (gui_action==108){Actualize_Main_Ed();}
	if (gui_action==109){Newmod();}
	if (gui_action==110){Actualize_Seq_Ed();}
	if (gui_action==111){Actualize_Seq_Ed();Actualize_Sequencer();}
	if (gui_action==112){Actualize_Fx_Ed(teac);}
	if (gui_action==113){shuffle=LastX-62;Actualize_Master_Ed(4);}
	if (gui_action==114){Actualize_Master_Ed(teac);}
	if (gui_action==150){Actualize_Midi_Ed(teac);}
	if (gui_action==151){mess_box("All Notes Off command send on this track...");}
	if (gui_action==152){mess_box("All Notes Off command send on all tracks...");}
	if (gui_action==153){mess_box("Panic pressed!");}
	if (gui_action==154){Actualize_Lfo_Ed(teac);}
	if (gui_action==125){ped_patsam-=16;Actualize_Patterned();RefreshSample();NewWavy();if (userscreen==6)Actualize_Midi_Ed(1);}
	if (gui_action==126){ped_patsam+=16;Actualize_Patterned();RefreshSample();NewWavy();if (userscreen==6)Actualize_Midi_Ed(1);}
	if (gui_action==129){Actualize_Patterned();}

	if (gui_action==115)
	{
		if (pSequence[cPosition]>9)
		pSequence[cPosition]-=10;
		else
		pSequence[cPosition]=0;
		
		Anat(cPosition);
		Actualize_Sequencer();
		Actupated(0);
	}
	
	if (gui_action==116)
	{
		if (pSequence[cPosition]<118)
		pSequence[cPosition]+=10;
		else
		pSequence[cPosition]=127;
		Anat(cPosition);
		
		Actualize_Sequencer();
		Actupated(0);
	}

	if (gui_action==169){RawRenderizer();}

	if (gui_action==200){Actualize_Sequencer();player_pos=cPosition;}
	if (gui_action==201){DeleteInstrument();}
	if (gui_action==202){SaveSynth();}
	if (gui_action==203){LoadSynth();}
	if (gui_action==204){mess_box("There're no more patterns allocated.");}
	if (gui_action==205){Actualize_Scopish();}
	if (gui_action==206){RandSynth();}

	if (gui_action==210){Actualize_Wave_Ed(teac);}
	
	if (gui_action==254){
	SongStop();
	mess_box("Returning to windows...");
	Sleep(1000);
	ss->stop();
	mess_box("All memory cleaned.");
	FadeToBlack();
	S->setColor(0,0,0);
	bjbox(0,0,CONSOLE_WIDTH,CONSOLE_HEIGHT);
	S->endDraw();
	return 1;}

	gui_action=255;
}
	
  if (clrc==1 || redraw_everything==1)
	{
		userscreen=0;
		last_index=-1;
		guiDial3(0,6,fsize,16,"Audio Vumeter:",200);
		guiDial3(0,24,392,16,"NOISETREKKER R13 By Juan Antonio Arguelles Rius <arguru@vermail.net>",200);
		mess_box(tipoftheday);
		guiDial3(394,24,58,16,"Current Dir:",200);
		guiDial(394,42,16,14,"-",200);
		guiDial(394,162,16,14,"+",200);
		guiDial3(0,178,fsize,4,"",200);
		guiDial3(0,309,110,16,"User Window",200);
		guiDial3(0,42,96,80,"Player",128);
		guiDial(8,60,80,16,"Play Sng/Pttrn",200);
	 	guiDial(8,78,80,16,"Slider Rec: OFF",200);
		guiDial(8,96,80,16,"Stop     ",200);
		guiDial3(256,42,136,80,"Song Settings",128);
		guiDial3(0,327,fsize,122,"",144);
		guiDial3(262,60,60,16,"Tracks",200);
		guiDial3(262,78,60,16,"B.P.M.",200);
		guiDial3(262,96,60,16,"T.P.B.",200);
		if(restx>0)guiDial3(772,309,restx-2,16,"",200);

		guiDial3(98,42,156,80,"Sequence",128);
		guiDial3(106,60,80,16,"Position",200);
		guiDial3(106,78,80,16,"Pattern",200);
		guiDial3(106,96,80,16,"Song Length",200);
		
		guiDial3(512,458,CONSOLE_WIDTH-514,CONSOLE_HEIGHT-482,"Sample Editor [v0.8]",144);		
		
		guiDial(516,476,29,16,"Cut",200);
		guiDial(547,476,29,16,"Half",200);
		guiDial(516,494,60,16,"Maximize",200);
		guiDial(516,512,60,16,"DC Adjust",200);
		guiDial(516,530,60,16,"Fade In",200);
		guiDial(516,548,60,16,"Fade Out",200);
		
		guiDial(578,476,60,16,"Select All",200);
		guiDial(578,494,60,16,"Unselect",200);
		
		guiDial(578,512,60,16,"View All",170);
		guiDial(578,530,60,16,"VZoom In",200);
		guiDial(578,548,60,16,"VZoom Out",200);

		NewWavy();
		guiDial(646,476,60,16,"Zoom In",200);
		guiDial(646,494,60,16,"Zoom Out",200);

		guiDial(708,476,60,16,"Set Loop S.",200);
		guiDial(708,494,60,16,"Set Loop E.",200);
		
		guiDial3(646,520,60,16,"Range",200);
		guiDial3(708,520,60,16,"View",200);
		
		guiDial3(0,452,fsize,4,"",200);
		guiDial3(0,124,392,52,"",128);
		guiDial3(8,152,80,16,"Step Add",200);
	 	guiDial3(166,152,90,16,"Keyboard Octave",200);
		guiDial3(8,134,80,16,"Instrument",200);
		guiDial(320,134,64,16,"Delete",200);
		draw_mained();
		Actualize_Sequencer();
		Actualize_Patterned();
		Actualize_Master(0);
		Actualize_Main_Ed();

		ltActualize(0);
		S->fillRect(79,10,601,14);
		S->fillRect(79,16,601,20);
		guiDial3(106,114,80,16,"Pattern Lines",200);
				
		
		if (clrc==1){
		AllocPattern();
		clrc=2;
		}
		
		LastX=C->getMouse()->getX();
		LastY=C->getMouse()->getY();
		actlogo=true;
	}
  if (Songplaying==1 && ped_line!=player_line)
  {
	  Actupated(0);
	  player_line=ped_line;
  }
 
  SelectObject(dc,old_f);DeleteObject(fnt);
  S->endDraw();  
}

if(NTKLOGO!=NULL && actlogo==true)
{
S->copy(NTKLOGO, 656, 338);actlogo=false;
S->copy(FRAMEWORK, 0, 184);
Actupated(0);
}

if(act303!=0)
{
	Actualize_303(act303);
	act303=0;
}
// Checking for mouse events ------------------------------------

  long rc;

if (gui_action==255)
{
if(!GUIMODE)
{MOUSEBACK->copy(S, 0, 0,LastX-1,LastY-1, LastX+10,LastY+18,0);
C->getMouse()->render();}

gui_action=0;
}

teac=0;

if (userscreen==2 && seditor==1 && C->getMouse()->leftButton())
{
gui_action=255;
if (zcheckMouse(78,350,16,16) && LoopStart[ped_patsam][ped_split]>0){LoopStart[ped_patsam][ped_split]--;gui_action=107;teac=5;}
if (zcheckMouse(194,350,16,16) && LoopStart[ped_patsam][ped_split]<LoopEnd[ped_patsam][ped_split]){LoopStart[ped_patsam][ped_split]++;gui_action=107;teac=5;}
if (zcheckMouse(278,350,16,16) && LoopEnd[ped_patsam][ped_split]>LoopStart[ped_patsam][ped_split]){LoopEnd[ped_patsam][ped_split]--;gui_action=101;teac=5;}
if (zcheckMouse(394,350,16,16) && LoopEnd[ped_patsam][ped_split]<SampleNumSamples[ped_patsam][ped_split]-1){LoopEnd[ped_patsam][ped_split]++;gui_action=101;teac=5;}
}

while((rc=C->getMouse()->getEvent(1,24,mlimit,CONSOLE_HEIGHT2))!=0)
  {
	  if (rc==1)// mouse moved, do something
	  {
		//Mouse Move Handling	   
		vloid();
		LastX=C->getMouse()->getX();
		LastY=C->getMouse()->getY();
		if(!GUIMODE)
		{MOUSEBACK->copy(S, 0, 0,LastX-1,LastY-1, LastX+10,LastY+18,0);
		C->getMouse()->render();}

	  }

if (C->getMouse()->leftButton())
{
	gui_action=255;

	if (zcheckMouse(395,59,16,103)){gui_action=1;}

if(userscreen==9)
{

	// Volume Knob
	if (zcheckMouse(529,357,19,88))
	{
		int breakvol=88-(LastY-357);
		if(breakvol<0)breakvol=0;
		if(breakvol>86)breakvol=86;
		tb303engine[sl3].tbVolume=breakvol*0.01136363636f;
		gui_action=73;
		teac=15;
	}

		// Tune Knob
	if (zcheckMouse(229,348,24,24))
	{
		if(fluzy==-1)fluzy=LastY+tb303[sl3].tune;

		int tempz=fluzy-LastY;
		if (tempz<0)tempz=0;
		if (tempz>127)tempz=127;
		tb303[sl3].tune=tempz;
		gui_action=73;
		teac=3;
	}
	
	// CutOff Knob
	if (zcheckMouse(262,348,24,24))
	{
		if(fluzy==-1)fluzy=LastY+tb303[sl3].cutoff;

		int tempz=fluzy-LastY;
		if (tempz<0)tempz=0;
		if (tempz>127)tempz=127;
		tb303[sl3].cutoff=tempz;
		gui_action=73;
		teac=4;
		liveparam=7+sl3;
		livevalue=tempz<<1;
	}

	// Reso knob
	if (zcheckMouse(295,348,24,24))
	{
		if(fluzy==-1)fluzy=LastY+tb303[sl3].resonance;

		int tempz=fluzy-LastY;
		if (tempz<0)tempz=0;
		if (tempz>127)tempz=127;
		tb303[sl3].resonance=tempz;
		gui_action=73;
		teac=5;
		liveparam=9+sl3;
		livevalue=tempz<<1;
	}

	// Envmod knob
	if (zcheckMouse(328,348,24,24))
	{
		if(fluzy==-1)fluzy=LastY+tb303[sl3].envmod;

		int tempz=fluzy-LastY;
		if (tempz<0)tempz=0;
		if (tempz>127)tempz=127;
		tb303[sl3].envmod=tempz;
		gui_action=73;
		teac=6;
		liveparam=11+sl3;
		livevalue=tempz<<1;
	}

	// Decay knob
	if (zcheckMouse(361,348,24,24))
	{
		if(fluzy==-1)fluzy=LastY+tb303[sl3].decay;

		int tempz=fluzy-LastY;
		if (tempz<0)tempz=0;
		if (tempz>127)tempz=127;
		tb303[sl3].decay=tempz;
		gui_action=73;
		teac=7;
		liveparam=13+sl3;
		livevalue=tempz<<1;
	}

	// Accent knob
	if (zcheckMouse(394,348,24,24))
	{
		if(fluzy==-1)fluzy=LastY+tb303[sl3].accent;

		int tempz=fluzy-LastY;
		if (tempz<0)tempz=0;
		if (tempz>127)tempz=127;
		tb303[sl3].accent=tempz;
		gui_action=73;
		teac=8;
	}
}

	if (zcheckMouse(0,458,512,CONSOLE_HEIGHT2))
	{
	if (SampleType[ped_patsam][ped_split])
	{
	axswave=SampleNumSamples[ped_patsam][ped_split]-1;
	sed_range_mode=true;
	sed_range_end=sed_display_start+((LastX-1)*sed_display_length)/512;
	teac=4;

	if (!sas)
	{
	sed_range_start=sed_range_end;
	teac=5;
	}
	
	if (sed_range_start<sed_display_start)
	{
		sed_range_start=sed_display_start;
		teac=5;
	}

	if(sed_range_end<=sed_range_start)
	{
	long erebus=sed_range_start;
	sed_range_start=sed_range_end;
	sed_range_end=erebus;
	teac=5;
	}
	
	if (sed_range_end>axswave)sed_range_end=axswave;
	gui_action=210;
	draw_sampled_wave=true;
	sas=true;
	}//SAMPLETYPE
	}//MOUSEBOX
	
	if (userscreen==2 && SampleType[ped_patsam][ped_split]!=0)
	{
	if (zcheckMouse(426,408,148,18)){gui_action=104;}
    if (zcheckMouse(436,426,128,18)){gui_action=105;}
	if (zcheckMouse(52,408,148,18)){gui_action=106;}
	if (zcheckMouse(52,426,148,18)){CustomVol[ped_patsam]=float(LastX-62)*0.0078125f;gui_action=107;teac=15;}
	}

	if (userscreen==5)
	{
	if (zcheckMouse(52,420,148,18)){gui_action=113;}
	if (zcheckMouse(252,366,148,18)){mas_vol=(LastX-262.0f)/128.0f;gui_action=114;teac=2;}
	if (zcheckMouse(252,384,48,16) && !AMIMODE){AMIMODE=true;gui_action=114;teac=5;}
	if (zcheckMouse(300,384,48,16) && AMIMODE){AMIMODE=false;gui_action=114;teac=5;}
	}

	
	if (userscreen==6)
	{
	if (zcheckMouse(406,388,148,18))
			{csynth_slv=LastX-416;
			if (csynth_slv<0)csynth_slv=0;
			if (csynth_slv>128)csynth_slv=128;
			teac=6;gui_action=150;}
	}

	if (userscreen==7)
	{
	if (zcheckMouse(74,362,148,18)){LFORATE[ped_track]=(LastX-84)/16384.0f;teac=2;gui_action=154;}
	if (zcheckMouse(74,380,148,18)){LFOAMPL[ped_track]=float(LastX-84);teac=3;gui_action=154;}
	if (zcheckMouse(74,416,148,18)){FLANGER_DEPHASE[ped_track]=(LastX-84)*0.0490873f;teac=4;gui_action=154;teac=4;}

	if (zcheckMouse(298,350,148,18)){teac=1;FLANGER_AMOUNT[ped_track]=((LastX-308.0f)/64.0f)-1.0f;gui_action=154;}
	if (zcheckMouse(298,368,148,18)){FLANGER_RATE[ped_track]=(LastX-308.0f)/939104.92f;gui_action=154;teac=5;}
	if (zcheckMouse(298,386,148,18)){FLANGER_AMPL[ped_track]=(LastX-308.0f)/12800.0f;gui_action=154;teac=6;}
	if (zcheckMouse(298,404,148,18)){FLANGER_FEEDBACK[ped_track]=((LastX-308)/64.0f)-1.0f;gui_action=154;teac=7;}
	if (zcheckMouse(298,422,148,18)){FLANGER_DELAY[ped_track]=(LastX-308)*32;gui_action=154;fld_chan=true;teac=8;}
	}
	if (userscreen==1)
	{
	if (zcheckMouse(78,360,148,18)){gui_action=50;}
	if (zcheckMouse(78,378,148,18)){gui_action=51;}
	if (zcheckMouse(78,414,148,18)){gui_action=52;}
	if (zcheckMouse(308,360,148,18)){gui_action=54;}
	if (zcheckMouse(308,378,148,18)){gui_action=55;}
	if (zcheckMouse(308,396,148,18)){gui_action=56;}
	if (zcheckMouse(308,414,148,18)){gui_action=57;}
		
	}

	if (userscreen==3)
	{
	if (zcheckMouse(78,378,148,18))
	{
		Feedback=float(float(LastX-88)/127.0);
		if (Feedback<0)Feedback=0;
		if (Feedback>0.99f)Feedback=0.99f;
		Feedback5=Feedback;
		Feedback4=Feedback;
		Feedback3=Feedback;
		Feedback2=Feedback;

		gui_action=112;
		
		teac=2;}
	if (zcheckMouse(78,414,148,18)){c_threshold=LastX-88;gui_action=112;teac=7;}
	if (zcheckMouse(78,432,148,18)){REVERBFILTER=(float)(LastX-88)/128.0f;gui_action=112;teac=9;}	
	if (zcheckMouse(308,360,148,18)){lchorus_delay=(LastX-318)*174;gui_action=112;teac=3;}
	if (zcheckMouse(308,378,148,18)){rchorus_delay=(LastX-318)*174;gui_action=112;teac=4;}
	if (zcheckMouse(308,396,148,18)){lchorus_feedback=float(LastX-318)/127;gui_action=112;teac=5;}
	if (zcheckMouse(308,414,148,18)){rchorus_feedback=float(LastX-318)/127;gui_action=112;teac=6;}

	}// userscreen

// Check Zones for GUI clicks -----------------------------------

	if (rc==2) // left mouse button (de)pressed, check state and act.
	{
	
		#include "303guicode.cpp"

		if (SampleType[ped_patsam][ped_split])
		{
		if (zcheckMouse(708,476,60,16) && sed_range_mode){
		LoopStart[ped_patsam][ped_split]=sed_range_start;
		
		if (LoopStart[ped_patsam][ped_split]>LoopEnd[ped_patsam][ped_split])
			LoopEnd[ped_patsam][ped_split]=LoopStart[ped_patsam][ped_split];
		
		if(LoopType[ped_patsam][ped_split]==0)
		{
		LoopType[ped_patsam][ped_split]=1;
		LoopEnd[ped_patsam][ped_split]=SampleNumSamples[ped_patsam][ped_split]-1;
		}

		draw_sampled_wave=true;
		if (userscreen==2){gui_action=107;teac=5;}
		}

		if (zcheckMouse(708,494,60,16) && sed_range_mode){
		LoopEnd[ped_patsam][ped_split]=sed_range_end;
		
		if (LoopEnd[ped_patsam][ped_split]<LoopStart[ped_patsam][ped_split])
			LoopStart[ped_patsam][ped_split]=LoopEnd[ped_patsam][ped_split];
				
		if(LoopType[ped_patsam][ped_split]==0)
		{
		LoopType[ped_patsam][ped_split]=1;
		LoopStart[ped_patsam][ped_split]=0;
		}

		draw_sampled_wave=true;

		if (userscreen==2){gui_action=107;teac=5;}
		}
					
		if (zcheckMouse(516,476,29,16) && sed_range_mode){teac=20;gui_action=210;}
		if (zcheckMouse(516,512,60,16) && sed_range_mode){teac=21;gui_action=210;}
		if (zcheckMouse(516,494,60,16) && sed_range_mode){teac=22;gui_action=210;}
		if (zcheckMouse(516,530,60,16) && sed_range_mode){teac=23;gui_action=210;}
		if (zcheckMouse(516,548,60,16) && sed_range_mode){teac=24;gui_action=210;}
		if (zcheckMouse(547,476,29,16) && sed_range_mode){teac=25;gui_action=210;}

		if (zcheckMouse(578,512,60,16)){
		rs_coef=32768;
		sed_display_start=0;
		sed_display_length=SampleNumSamples[ped_patsam][ped_split];
		draw_sampled_wave=true;
		teac=3;
		gui_action=210;
		}

		if (zcheckMouse(578,530,60,16)){
		rs_coef/=2;
		draw_sampled_wave=true;
		}

		if (zcheckMouse(578,548,60,16)){
		rs_coef*=2;
		draw_sampled_wave=true;
		}

		if (zcheckMouse(578,494,60,16)){
		sed_range_mode=false;
		sed_range_start=0;
		sed_range_end=0;
		draw_sampled_wave=true;
		teac=0;
		gui_action=210;}

		if (zcheckMouse(578,476,60,16)){
		sed_range_mode=true;
		sed_range_start=sed_display_start;
		sed_range_end=sed_display_start+sed_display_length-1;
		draw_sampled_wave=true;
		teac=0;
		gui_action=210;}
		
		if (zcheckMouse(646,476,60,16) && sed_range_mode){
		sed_range_mode=false;
		sed_display_start=sed_range_start;
		sed_display_length=(sed_range_end-sed_range_start)+1;
		draw_sampled_wave=true;
		teac=3;
		gui_action=210;
		}

		if (zcheckMouse(646,494,60,16)){
		sed_display_start-=sed_display_length;
		if(sed_display_start<0)sed_display_start=0;
		sed_display_length*=3;
	
		if(sed_display_length+sed_display_start>SampleNumSamples[ped_patsam][ped_split])
		sed_display_length=SampleNumSamples[ped_patsam][ped_split]-sed_display_start;

		draw_sampled_wave=true;
		teac=3;
		gui_action=210;
		}
		}
		
		if (zcheckMouse(90,134,166,16) && snamesel==0){snamesel=2;sprintf(nameins[ped_patsam],"");namesize=0;gui_action=129;}

		if (zcheckMouse(394,42,16,14)){lt_index--;gui_action=100;}
		if (zcheckMouse(394,162,16,14)){lt_index++;gui_action=100;}
		if (zcheckMouse(412,43,220,133)){gui_action=2;}
		if (zcheckMouse(188,60,16,16) && cPosition>0){gui_action=3;}
		if (zcheckMouse(232,60,16,16) && cPosition<255){gui_action=4;}
		if (zcheckMouse(188,78,16,16) && pSequence[cPosition]>0){gui_action=7;}
		if (zcheckMouse(232,78,16,16) && pSequence[cPosition]<254){gui_action=8;}
		if (zcheckMouse(188,96,16,16) && sLength>1){gui_action=9;}
		if (zcheckMouse(232,96,16,16) && sLength<255){gui_action=10;}
		if (zcheckMouse(188,114,16,16) && patternLines[pSequence[cPosition]]>1){patternLines[pSequence[cPosition]]--;gui_action=5;}
		if (zcheckMouse(232,114,16,16) && patternLines[pSequence[cPosition]]<128){patternLines[pSequence[cPosition]]++;gui_action=5;}
		
		if (zcheckMouse(258,152,16,16)){gui_action=21;}
		if (zcheckMouse(302,152,16,16)){gui_action=22;}
		if (zcheckMouse(90,152,16,16)){gui_action=23;}
		if (zcheckMouse(134,152,16,16)){gui_action=24;}
		if (zcheckMouse(258,134,16,16)){gui_action=25;}
		if (zcheckMouse(302,134,16,16)){gui_action=26;}
		if (zcheckMouse(1,184,CONSOLE_WIDTH,124)){gui_action=27;}
		if (zcheckMouse(8,60,80,16)){plx=0;gui_action=28;}
		if (zcheckMouse(8,96,80,16)){gui_action=29;}
		if (zcheckMouse(8,78,80,16))
		{
			sr_isrecording++;
			if (sr_isrecording>1)sr_isrecording=0;
			gui_action=30;
		}
		
		if (zcheckMouse(320,134,64,16)){gui_action=201;}
		
		if (zcheckMouse(324,60,16,16) && Songtracks>1){Songtracks--;gui_action=40;teac=4;}
		if (zcheckMouse(368,60,16,16) && Songtracks<16){Songtracks++;gui_action=40;teac=4;}
		
		if (zcheckMouse(324,78,16,16)){BeatsPerMin--;gui_action=40;teac=1;}
		if (zcheckMouse(368,78,16,16)){BeatsPerMin++;gui_action=40;teac=1;}
		
		if (zcheckMouse(324,96,16,16)){TicksPerBeat--;gui_action=40;teac=2;}
		if (zcheckMouse(368,96,16,16)){TicksPerBeat++;gui_action=40;teac=2;}

		if (userscreen!=8)
		{
		if (zcheckMouse(376,309,64,16) && userscreen!=0)gui_action=60;
		if (zcheckMouse(442,309,64,16) && userscreen!=1)gui_action=61;
		if (zcheckMouse(508,309,64,16) && userscreen!=2){gui_action=62;seditor=0;}
		if (zcheckMouse(574,309,64,16) && userscreen!=3){gui_action=63;teac=0;}	
		if (zcheckMouse(310,309,64,16) && userscreen!=4)gui_action=64;	
		if (zcheckMouse(244,309,64,16) && userscreen!=5)gui_action=65;
		if (zcheckMouse(178,309,64,16) && userscreen!=6)gui_action=66;
		if (zcheckMouse(112,309,64,16) && userscreen!=7)gui_action=67;
		if (zcheckMouse(706,309,64,16) && userscreen!=9)gui_action=72;
		
		if (zcheckMouse(640,309,64,16))gui_action=68;
		}
		else if (zcheckMouse(640,434,64,16))gui_action=69;
		
		if (userscreen==7)
		{
		if (zcheckMouse(74,344,20,16) && LFO_ON[ped_track]==0){LFO_ON[ped_track]=1;gui_action=154;teac=9;}
		if (zcheckMouse(96,344,20,16) && LFO_ON[ped_track]==1){LFO_ON[ped_track]=0;gui_action=154;teac=9;}
		if (zcheckMouse(184,344,20,16) && FLANGER_ON[ped_track]==0){FLANGER_ON[ped_track]=1;gui_action=154;teac=10;}
		if (zcheckMouse(206,344,20,16) && FLANGER_ON[ped_track]==1){FLANGER_ON[ped_track]=0;gui_action=154;teac=10;}
		if (zcheckMouse(532,368,42,16) && Dispan[ped_track]){Dispan[ped_track]=false;gui_action=154;teac=11;}
		if (zcheckMouse(576,368,44,16) && !Dispan[ped_track]){Dispan[ped_track]=true;gui_action=154;teac=11;}
		}

	
		if (userscreen==4)
		{
		if (zcheckMouse(144,348,80,16)){SeqFill(0,256,false);gui_action=110;}
		if (zcheckMouse(144,366,80,16)){SeqFill(cPosition,cPosition+1,false);gui_action=110;}
		if (zcheckMouse(144,384,80,16)){SeqFill(0,256,true);gui_action=110;}
		if (zcheckMouse(144,402,80,16)){SeqFill(cPosition,cPosition+1,true);gui_action=110;}
		if (zcheckMouse(178,420,22,16)){Scopish=true;gui_action=205;}
		if (zcheckMouse(202,420,22,16)){Scopish=false;gui_action=205;}
		
		if (zcheckMouse(410,348,80,16))
		{
		if(cPosition<128){pSequence[cPosition]=cPosition;Anat(cPosition);gui_action=110;}
		}
		
		if (zcheckMouse(410,364,80,16))
		{
		for(int xpos=0;xpos<128;xpos++)pSequence[xpos]=xpos;

		gui_action=110;
		}
		
		if (zcheckMouse(399,345,8,84))
		{
		int posindex=((LastY-348)/12)-3;
		posindex+=cPosition;

		if (pSequence[posindex]<127)
			{
				pSequence[posindex]++;
				Anat(posindex);

				if (posindex!=cPosition)
					gui_action=110;
				else
					gui_action=111;
			
			}
			else
					gui_action=204;
		}

		if (zcheckMouse(392,345,8,84))
		{
		int posindex=((LastY-348)/12)-3;
		posindex+=cPosition;

		if (pSequence[posindex]<127)
			{
				pSequence[posindex]+=10;	
				if(pSequence[posindex]>=128)
				pSequence[posindex]=127;
				Anat(posindex);

				if (posindex!=cPosition)
					gui_action=110;
				else
					gui_action=111;
			}
			else
			gui_action=204;
			
		}
		
		if (zcheckMouse(256,346,130,84)==1){
			int posindex=((LastY-348)/12)-3;
			posindex+=cPosition;
		
			if (posindex>-1 && posindex<256){
			int seqindex=(LastX-258)/8;
			if (seqindex<0)seqindex=0;
			if (seqindex>MAX_TRACKS-1)seqindex=MAX_TRACKS-1;
			
			if (!SACTIVE[posindex][seqindex])
			SACTIVE[posindex][seqindex]=true;
			else
			SACTIVE[posindex][seqindex]=false;
			gui_action=110;}}

			if (zcheckMouse(232,346,16,84)){
			int posindex=((LastY-348)/12)-3;
			posindex+=cPosition;
			if (posindex>-1 && posindex<256 && posindex!=cPosition)
			{cPosition=posindex;gui_action=111;}}

		}
		
		if (userscreen==0)
		{
		if (zcheckMouse(8,368,80,16)){gui_action=102;}
		if (zcheckMouse(172,350,80,16)){gui_action=70;}
		if (zcheckMouse(172,368,80,16)){gui_action=71;}
		if (zcheckMouse(8,350,80,16)){gui_action=103;}
		if (zcheckMouse(90,386,162,16) && snamesel==0){sprintf(name,"");namesize=0;snamesel=1;gui_action=108;}
		if (zcheckMouse(90,404,162,16) && snamesel==0){sprintf(artist,"");namesize=0;snamesel=4;gui_action=108;}
		if (zcheckMouse(90,422,162,16) && snamesel==0){sprintf(style,"");namesize=0;snamesel=5;gui_action=108;}
		
		if (zcheckMouse(264,368,64,16) && QUALITYPLAY!=0){QUALITYPLAY=0;gui_action=108;QUALITYCHANGE=true;}
		if (zcheckMouse(264,386,64,16) && QUALITYPLAY!=1){QUALITYPLAY=1;gui_action=108;QUALITYCHANGE=true;}
		if (zcheckMouse(264,404,64,16) && QUALITYPLAY!=2){QUALITYPLAY=2;gui_action=108;QUALITYCHANGE=true;}

		if (zcheckMouse(90,350,80,16)){gui_action=109;}
		if (zcheckMouse(482,350,16,16) && XLATENCY_TIME>10){XLATENCY_TIME-=10;if (XLATENCY_TIME<10)XLATENCY_TIME=10;Resetdevice();gui_action=108;}
		if (zcheckMouse(526,350,16,16) && XLATENCY_TIME<250){XLATENCY_TIME+=10;if (XLATENCY_TIME>250)XLATENCY_TIME=250;Resetdevice();gui_action=108;}
		if (zcheckMouse(90,368,80,16)){rawrender=true;gui_action=169;}
		}

		if (userscreen==1)
		{
		if (zcheckMouse(77,396,16,16) && FType[ped_track]>0){ResetFilters(ped_track);FType[ped_track]--;teac=4;gui_action=53;}
		if (zcheckMouse(121,396,16,16) && FType[ped_track]<MAX_FILTER){ResetFilters(ped_track);FType[ped_track]++;teac=4;gui_action=53;}
		if (zcheckMouse(508,362,16,16)){ped_track--;teac=0;gui_action=53;trkchan=true;}
		if (zcheckMouse(552,362,16,16)){ped_track++;teac=0;gui_action=53;trkchan=true;}	
		if (zcheckMouse(570,362,16,16)){CSend[ped_track]--;teac=6;gui_action=53;}
		if (zcheckMouse(614,362,16,16)){CSend[ped_track]++;teac=6;gui_action=53;}	

		if (zcheckMouse(456,360,40,16)){DThreshold[ped_track]=DClamp[ped_track];teac=7;gui_action=53;}	
		if (zcheckMouse(456,378,40,16)){DClamp[ped_track]=DThreshold[ped_track];teac=8;gui_action=53;}	
		if (zcheckMouse(456,414,40,16)){TPan[ped_track]=0.5f;gui_action=53;teac=9;}	

		if (zcheckMouse(570,406,16,16)){TRACKMIDICHANNEL[ped_track]--;gui_action=53;teac=11;}	
		if (zcheckMouse(614,406,16,16)){TRACKMIDICHANNEL[ped_track]++;gui_action=53;teac=11;}	

		// Mute track
		
		if (zcheckMouse(570,424,60,16)){Disclap[ped_track]=!Disclap[ped_track];gui_action=53;teac=12;}

		if (zcheckMouse(508,388,64,16)){
			if(TRACKSTATE[ped_track]==0)
			TRACKSTATE[ped_track]=1;
			else
			TRACKSTATE[ped_track]=0;gui_action=53;teac=10;trkchan=true;}	
		
		// Solo track

		if (zcheckMouse(508,406,64,16)){for (int solify=0;solify<MAX_TRACKS;solify++){TRACKSTATE[solify]=1;}TRACKSTATE[ped_track]=0;gui_action=53;trkchan=true;teac=10;}	
		
		// Unmute all tracks

		if (zcheckMouse(508,424,88,16)){for (int solify=0;solify<MAX_TRACKS;solify++){TRACKSTATE[solify]=0;}gui_action=53;trkchan=true;teac=10;}	
		
		}// Userscreen 1

		if (userscreen==2 && seditor==1){
		if (zcheckMouse(96,350,16,16) && LoopStart[ped_patsam][ped_split]>0){LoopStart[ped_patsam][ped_split]--;gui_action=107;teac=5;}
		if (zcheckMouse(176,350,16,16) && LoopStart[ped_patsam][ped_split]<LoopEnd[ped_patsam][ped_split]){LoopStart[ped_patsam][ped_split]++;gui_action=107;teac=5;}
		if (zcheckMouse(296,350,16,16) && LoopEnd[ped_patsam][ped_split]>LoopStart[ped_patsam][ped_split]){LoopEnd[ped_patsam][ped_split]--;gui_action=101;teac=5;}
		if (zcheckMouse(376,350,16,16) && LoopEnd[ped_patsam][ped_split]<SampleNumSamples[ped_patsam][ped_split]-1){LoopEnd[ped_patsam][ped_split]++;gui_action=101;teac=5;}	
		if (zcheckMouse(424,391,64,16)){seditor=0;gui_action=62;}
		}

		if (userscreen==2 && seditor==0){
		if (zcheckMouse(570,373,16,16) && Midiprg[ped_patsam]>-1){Midiprg[ped_patsam]--;gui_action=107;teac=10;}
		if (zcheckMouse(614,373,16,16) && Midiprg[ped_patsam]<127){Midiprg[ped_patsam]++;gui_action=107;teac=10;}
		if (zcheckMouse(570,391,28,16)){Synthprg[ped_patsam]=false;gui_action=107;teac=11;}
		if (zcheckMouse(602,391,28,16)){Synthprg[ped_patsam]=true;gui_action=107;teac=11;}
		
		if (zcheckMouse(144,372,28,16)){beatsync[ped_patsam]=false;gui_action=107;teac=12;}
		if (zcheckMouse(174,372,28,16)){beatsync[ped_patsam]=true;gui_action=107;teac=12;}
		
		if (zcheckMouse(144,390,16,16) && beatlines[ped_patsam]>1){beatlines[ped_patsam]--;gui_action=107;teac=13;}
		if (zcheckMouse(188,390,16,16) && beatlines[ped_patsam]<128){beatlines[ped_patsam]++;gui_action=107;teac=13;}
		
		if (zcheckMouse(570,337,16,16) && ped_split>0){ped_split--;gui_action=101;teac=0;NewWavy();}
		if (zcheckMouse(614,337,16,16) && ped_split<15){ped_split++;gui_action=101;teac=0;NewWavy();}
		if (zcheckMouse(268,346,88,16) && SampleType[ped_patsam][ped_split]){seditor=1;gui_action=62;}
		if (zcheckMouse(268,364,88,16) && SampleType[ped_patsam][ped_split]){gui_action=107;teac=14;}
		
		if (zcheckMouse(570,355,16,16) && Basenote[ped_patsam][ped_split]>0){Basenote[ped_patsam][ped_split]--;teac=9;gui_action=107;}
		if (zcheckMouse(614,355,16,16) && Basenote[ped_patsam][ped_split]<119){Basenote[ped_patsam][ped_split]++;teac=9;gui_action=107;}
		
		if (SampleType[ped_patsam][ped_split]!=0)
		{
		if (zcheckMouse(448,391,29,16)){LoopType[ped_patsam][ped_split]=1;teac=5;gui_action=107;draw_sampled_wave=true;}
		if (zcheckMouse(478,391,29,16)){LoopType[ped_patsam][ped_split]=0;teac=5;gui_action=107;draw_sampled_wave=true;}
		if (zcheckMouse(268,418,88,16)){} /* Range Loop Buttoon */
		if (zcheckMouse(268,400,88,16)){} /* Smooth Loop Buttoon */
		}
	
		}

		if (userscreen==3)
		{
		if (zcheckMouse(77,396,16,16)==1){DelayType--;gui_action=112;teac=1;}
		if (zcheckMouse(121,396,32,16)==1){DelayType++;gui_action=112;teac=1;}
		
		if (zcheckMouse(540,360,16,16)==1){lchorus_delay=SamplesPerTick;gui_action=112;teac=3;}
		if (zcheckMouse(540,378,16,16)==1){rchorus_delay=SamplesPerTick;gui_action=112;teac=4;}
		
		if (zcheckMouse(558,360,16,16)==1){lchorus_delay=SamplesPerTick*2;gui_action=112;teac=3;}
		if (zcheckMouse(558,378,16,16)==1){rchorus_delay=SamplesPerTick*2;gui_action=112;teac=4;}
		
		if (zcheckMouse(576,360,16,16)==1){lchorus_delay=SamplesPerTick*3;gui_action=112;teac=3;}
		if (zcheckMouse(576,378,16,16)==1){rchorus_delay=SamplesPerTick*3;gui_action=112;teac=4;}
		
		if (zcheckMouse(594,360,16,16)==1){lchorus_delay=SamplesPerTick*4;gui_action=112;teac=3;}
		if (zcheckMouse(594,378,16,16)==1){rchorus_delay=SamplesPerTick*4;gui_action=112;teac=4;}
		
		if (zcheckMouse(612,360,16,16)==1){lchorus_delay=SamplesPerTick*5;gui_action=112;teac=3;}
		if (zcheckMouse(612,378,16,16)==1){rchorus_delay=SamplesPerTick*5;gui_action=112;teac=4;}

		if (compressor==0 && zcheckMouse(78,360,32,16)==1){compressor=1;gui_action=112;teac=8;}
		if (compressor==1 && zcheckMouse(112,360,32,16)==1){compressor=0;gui_action=112;teac=8;}	
		}

		if (userscreen==6)
		{
		if (zcheckMouse(598,352,34,16)){gui_action=202;}
		if (zcheckMouse(598,370,34,16)){gui_action=206;}

		if (zcheckMouse(432,352,128,16) && snamesel==0){snamesel=3;namesize=0;sprintf(PARASynth[ped_patsam].presetname,"");teac=10;gui_action=150;}
		if (zcheckMouse(66,352,16,16)){c_midiin--;gui_action=150;midiin_changed=true;teac=8;}
		if (zcheckMouse(110,352,16,16)){c_midiin++;gui_action=150;midiin_changed=true;teac=8;}
		
		if (zcheckMouse(66,370,16,16)==1){c_midiout--;gui_action=150;midiout_changed=true;teac=9;}
		if (zcheckMouse(110,370,16,16)==1){c_midiout++;gui_action=150;midiout_changed=true;teac=9;}
		if (zcheckMouse(406,370,16,16) && ped_synthpar>1){ped_synthpar--;gui_action=150;teac=2;}
		if (zcheckMouse(450,370,16,16) && ped_synthpar<51){ped_synthpar++;gui_action=150;teac=2;}	
		if (zcheckMouse(388,406,24,16)){PARASynth[ped_patsam].osc1_waveform=0;teac=3;gui_action=150;}
		if (zcheckMouse(414,406,24,16)){PARASynth[ped_patsam].osc1_waveform=1;teac=3;gui_action=150;}
		if (zcheckMouse(440,406,24,16)){PARASynth[ped_patsam].osc1_waveform=2;teac=3;gui_action=150;}
		if (zcheckMouse(466,406,24,16)){PARASynth[ped_patsam].osc1_waveform=3;teac=3;gui_action=150;}
		if (zcheckMouse(492,406,24,16)){PARASynth[ped_patsam].osc1_waveform=4;teac=3;gui_action=150;}
		if (zcheckMouse(388,424,24,16)){PARASynth[ped_patsam].osc2_waveform=0;teac=4;gui_action=150;}
		if (zcheckMouse(414,424,24,16)){PARASynth[ped_patsam].osc2_waveform=1;teac=4;gui_action=150;}
		if (zcheckMouse(440,424,24,16)){PARASynth[ped_patsam].osc2_waveform=2;teac=4;gui_action=150;}
		if (zcheckMouse(466,424,24,16)){PARASynth[ped_patsam].osc2_waveform=3;teac=4;gui_action=150;}
		if (zcheckMouse(492,424,24,16)){PARASynth[ped_patsam].osc2_waveform=4;teac=4;gui_action=150;}
		if (zcheckMouse(572,406,24,16)){PARASynth[ped_patsam].osc3_switch=true;teac=5;gui_action=150;}
		if (zcheckMouse(596,406,24,16)){PARASynth[ped_patsam].osc3_switch=false;teac=5;gui_action=150;}
		if (zcheckMouse(572,424,24,16) && PARASynth[ped_patsam].vcf_type>0){PARASynth[ped_patsam].vcf_type--;teac=7;gui_action=150;}
		if (zcheckMouse(616,424,24,16) && PARASynth[ped_patsam].vcf_type<2){PARASynth[ped_patsam].vcf_type++;teac=7;gui_action=150;}
				
		if (zcheckMouse(8,388,124,16)==1 && c_midiout!=-1)
		{
		midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ped_track]) | (123 << 8) | (0 << 16)); 
		gui_action=151;
		}

		if (zcheckMouse(134,388,124,16)==1  && c_midiout!=-1)
		{
		MidiAllNotesOff();
		gui_action=152;
		}

		if (zcheckMouse(8,406,80,16)==1){;}
		
		}
	}//rc depressing
}// leftbutton
else
{gui_pushed=0;sas=false;}

if (C->getMouse()->rightButton() && rc==3)
{
	gui_action=255;

	if (zcheckMouse(90,152,16,16)){ped_pattad=1;gui_action=129;}
	if (zcheckMouse(134,152,16,16)){ped_pattad=16;gui_action=129;}

	if (zcheckMouse(188,114,16,16)){
		int ltp=patternLines[pSequence[cPosition]];
		ltp-=16;
		if(ltp<1)ltp=1;
	patternLines[pSequence[cPosition]]=ltp;
	gui_action=5;}
	
	if (zcheckMouse(232,114,16,16)){
		int ltp=patternLines[pSequence[cPosition]];
		ltp+=16;
		if(ltp>128)ltp=128;
	patternLines[pSequence[cPosition]]=ltp;
	gui_action=5;}
	
	if (zcheckMouse(258,134,16,16)==1){gui_action=125;}
	if (zcheckMouse(302,134,16,16)==1){gui_action=126;}
	
	if (zcheckMouse(8,60,80,16)==1){plx=1;gui_action=28;}
	if (zcheckMouse(394,42,16,14)==1){lt_index-=10;gui_action=100;}
	if (zcheckMouse(394,162,16,14)==1){lt_index+=10;gui_action=100;}
	if (zcheckMouse(188,60,16,16)==1 && cPosition!=0){int tposition=cPosition;tposition-=10;if(tposition<0)tposition=0;cPosition=tposition;gui_action=5;}
	if (zcheckMouse(232,60,16,16)==1 && cPosition!=255){int tposition=cPosition;tposition+=10;if(tposition>255)tposition=255;cPosition=tposition;gui_action=5;}

	if (zcheckMouse(188,96,16,16)==1 && sLength!=1){int tLength=sLength;tLength-=10;if (tLength<1)tLength=1;sLength=tLength;gui_action=11;}
	if (zcheckMouse(232,96,16,16)==1 && sLength!=255){int tLength=sLength;tLength+=10;if (tLength>255)tLength=255;sLength=tLength;gui_action=11;}

	if (zcheckMouse(188,78,16,16)==1){gui_action=115;}
	if (zcheckMouse(232,78,16,16)==1){gui_action=116;}

	guiDial(106,384,32,16,"On",200);
	guiDial(140,384,32,16,"Off",100);

	
	if (userscreen==1)
	{
	if (zcheckMouse(570,362,16,16)==1){CSend[ped_track]-=16;gui_action=53;}
	if (zcheckMouse(614,362,16,16)==1){CSend[ped_track]+=16;gui_action=53;}	
	}

	if (userscreen==2 && seditor==0)
	{
	if (zcheckMouse(144,391,16,16) && beatlines[ped_patsam]>1)
	{
		beatlines[ped_patsam]-=16;
		if(beatlines[ped_patsam]<1)beatlines[ped_patsam]=1;
		gui_action=107;
		teac=13;
	}

	if (zcheckMouse(188,391,16,16) && beatlines[ped_patsam]<128)
	{
		beatlines[ped_patsam]+=16;
		if(beatlines[ped_patsam]>128)beatlines[ped_patsam]=128;
		gui_action=107;
		teac=13;}
		
	if (zcheckMouse(570,373,16,16) && Midiprg[ped_patsam]>-1)
	{	if(Midiprg[ped_patsam]>14)
		Midiprg[ped_patsam]-=16;
		else
		Midiprg[ped_patsam]=-1;
		gui_action=107;
	teac=10;}
	
	if (zcheckMouse(614,373,16,16) && Midiprg[ped_patsam]<127)
	{
		if(Midiprg[ped_patsam]<111)
		Midiprg[ped_patsam]+=16;
		else
		Midiprg[ped_patsam]=127;
		gui_action=107;
	teac=10;}	

	if (zcheckMouse(570,355,16,16) && Basenote[ped_patsam][ped_split]>0)
	{	if(Basenote[ped_patsam][ped_split]>11)
		Basenote[ped_patsam][ped_split]-=12;
		else
		Basenote[ped_patsam][ped_split]=0;
		gui_action=107;
	teac=9;}
	
	if (zcheckMouse(614,355,16,16) && Basenote[ped_patsam][ped_split]<119)
	{
		if(Basenote[ped_patsam][ped_split]<107)
		Basenote[ped_patsam][ped_split]+=12;
		else
		Basenote[ped_patsam][ped_split]=119;
		gui_action=107;
	teac=9;}
	}

	if (userscreen==4)
		{

		if (zcheckMouse(399,345,8,84))
		{
		int posindex=((LastY-348)/12)-3;
		posindex+=cPosition;

		if (pSequence[posindex]>0)
			{
				pSequence[posindex]--;
				Anat(posindex);

				if (posindex!=cPosition)
					gui_action=110;
				else
					gui_action=111;
			}
		}

		if (zcheckMouse(392,345,8,84))
		{
		int posindex=((LastY-348)/12)-3;
		posindex+=cPosition;

		if (pSequence[posindex]>0)
			{
				int reak=pSequence[posindex];
				
				reak-=10;
				if (reak<0)reak=0;
				pSequence[posindex]=reak;
				Anat(posindex);

				if (posindex!=cPosition)
					gui_action=110;
				else
					gui_action=111;
			}			
		}

		if (zcheckMouse(256,346,130,84)==1){
			int posindex=((LastY-348)/12)-3;
			posindex+=cPosition;
			
			if (posindex>-1 && posindex<256){
			int seqindex=(LastX-258)/8;
			if (seqindex<0)seqindex=0;
			if (seqindex>Songtracks-1)seqindex=Songtracks-1;
			
			if (SACTIVE[posindex][seqindex])
			{
			for (int alphac=0;alphac<Songtracks;alphac++)
			SACTIVE[posindex][alphac]=false;
			}

			SACTIVE[posindex][seqindex]=true;
			gui_action=110;}}// action 110
		}//userscr4
	
	if (userscreen==6)
	{
	if (zcheckMouse(406,370,16,16) && ped_synthpar>1){ped_synthpar-=10;gui_action=150;teac=2;}
	if (zcheckMouse(450,370,16,16) && ped_synthpar<51){ped_synthpar+=10;gui_action=150;teac=2;}
	}
} // RIGHT MOUSE

}// Mouse event true
	  
// KeyBoard ShortCut Handler ------------------------------------

 Keyboard& K=*C->getKeyboard();
 K.update();

  if (K[DIK_ESCAPE] && K[DIK_LMENU]){gui_action=254;}
  if (K[DIK_F5] && !K[DIK_LSHIFT] && !K[DIK_LCONTROL] && ped_line!=0){vloid();ped_line=0;Actupated(0);}
  if (K[DIK_F6] && ped_line!=16){ped_line=16;Actupated(0);}
  if (K[DIK_F7] && ped_line!=32){ped_line=32;Actupated(0);}
  if (K[DIK_F8] && ped_line!=48){ped_line=48;Actupated(0);}
  if (K[DIK_F9] && ped_line!=63){ped_line=63;Actupated(0);}
  
  if (poslad==false && !K[DIK_SUBTRACT] && !K[DIK_ADD] && !K[DIK_MULTIPLY] && !K[DIK_DIVIDE] && !K[DIK_NEXT] && !K[DIK_PRIOR] && !K[DIK_UP] && !K[DIK_DOWN] && !K[DIK_TAB] && !K[DIK_RIGHT] && !K[DIK_LEFT]){poslad=true;}
  
  if (K[DIK_RIGHT] && poslad==true){poslad=false;ped_row++;Actupated(1);}
  if (K[DIK_LEFT] && poslad==true){poslad=false;ped_row--;Actupated(1);}
  if (!K[DIK_LSHIFT] && K[DIK_TAB] && poslad==true){ped_track++;ped_row=0;Actupated(1);poslad=false;gui_action=15;}
  if (K[DIK_LSHIFT] && K[DIK_TAB] && poslad==true){ped_track--;ped_row=0;Actupated(1);poslad=false;gui_action=16;}
  if (K[DIK_UP] && poslad==true){ped_line--;Actupated(0);poslad=false;}
  if (K[DIK_DOWN] && poslad==true){ped_line++;Actupated(0);poslad=false;}
  if (K[DIK_PRIOR] && poslad==true){;ped_line-=16;if (ped_line<0)ped_line=0;Actupated(0);poslad=false;}
  if (K[DIK_NEXT] && poslad==true && ped_line<patternLines[pSequence[cPosition]]){ped_line+=16;if (ped_line>=patternLines[pSequence[cPosition]])ped_line=patternLines[pSequence[cPosition]]-1;Actupated(0);poslad=false;}
  
  if (K[DIK_DIVIDE] && poslad==true){vloid();gui_action=21;poslad=false;}
  if (K[DIK_MULTIPLY] && poslad==true){vloid();gui_action=22;poslad=false;}

  if (K[DIK_SUBTRACT] && poslad==true && pSequence[cPosition]>0){vloid();gui_action=7;poslad=false;}
  if (K[DIK_ADD] && poslad==true && pSequence[cPosition]<254){vloid();gui_action=8;poslad=false;}
	
int retnote=120; // NO NOTE
int retvalue=120; // NOVALUE
retletter=0;

if (snamesel>0)
{
if (K[DIK_A])retletter=1;
if (K[DIK_B])retletter=2;
if (K[DIK_C])retletter=3;
if (K[DIK_D])retletter=4;
if (K[DIK_E])retletter=5;
if (K[DIK_F])retletter=6;
if (K[DIK_G])retletter=7;
if (K[DIK_H])retletter=8;
if (K[DIK_I])retletter=9;
if (K[DIK_J])retletter=10;
if (K[DIK_K])retletter=11;
if (K[DIK_L])retletter=12;
if (K[DIK_M])retletter=13;
if (K[DIK_N])retletter=14;
if (K[DIK_O])retletter=15;
if (K[DIK_P])retletter=16;
if (K[DIK_Q])retletter=17;
if (K[DIK_R])retletter=18;
if (K[DIK_S])retletter=19;
if (K[DIK_T])retletter=20;
if (K[DIK_U])retletter=21;
if (K[DIK_V])retletter=22;
if (K[DIK_W])retletter=23;
if (K[DIK_X])retletter=24;
if (K[DIK_Y])retletter=25;
if (K[DIK_Z])retletter=26;
if (K[DIK_0])retletter=27;
if (K[DIK_1])retletter=28;
if (K[DIK_2])retletter=29;
if (K[DIK_3])retletter=30;
if (K[DIK_4])retletter=31;
if (K[DIK_5])retletter=32;
if (K[DIK_6])retletter=33;
if (K[DIK_7])retletter=34;
if (K[DIK_8])retletter=35;
if (K[DIK_9])retletter=36;
if (K[DIK_BACK])retletter=37;
if (K[DIK_SPACE])retletter=38;
if (K[DIK_RETURN])retletter=39;
if (K[DIK_PERIOD])retletter=40;
if (K[DIK_COMMA])retletter=67;
if (K[DIK_SLASH])retletter=68;
if (K[DIK_MINUS])retletter=69;
if (K[DIK_EQUALS])retletter=70;
}

if ((K[DIK_LSHIFT] || K[DIK_RSHIFT])&& retletter<27 && retletter>0)
tretletter=retletter+40;
else
tretletter=retletter;

if (retletter==0)posletter=0;
if (retletter!=0 && posletter!=retletter)
{
	vloid();
	switch (snamesel)
	{
	case 1:
		Actualize_Songname(tretletter,name);
		gui_action=108;
	break;

	case 2:
		Actualize_Songname(tretletter,nameins[ped_patsam]);
		gui_action=129;
	break;
	
	case 3:
		Actualize_Songname(tretletter,PARASynth[ped_patsam].presetname);
		teac=10;
		gui_action=150;
	break;

	case 4:
		Actualize_Songname(tretletter,artist);
		gui_action=108;
	break;

	case 5:
		Actualize_Songname(tretletter,style);
		gui_action=108;
	break;

	}

	posletter=retletter;
}

if (!K[DIK_LMENU] && !K[DIK_LCONTROL] && po_ctrl==1 && snamesel==0)
{
if (K[DIK_Z])retnote=0;
if (K[DIK_S])retnote=1;
if (K[DIK_X])retnote=2;
if (K[DIK_D])retnote=3;
if (K[DIK_C])retnote=4;
if (K[DIK_V])retnote=5;
if (K[DIK_G])retnote=6;
if (K[DIK_B])retnote=7;
if (K[DIK_H])retnote=8;
if (K[DIK_N])retnote=9;
if (K[DIK_J])retnote=10;
if (K[DIK_M])retnote=11;
if (K[DIK_COMMA])retnote=12;
if (K[DIK_L])retnote=13;
if (K[DIK_PERIOD])retnote=14;
if (K[DIK_SEMICOLON])retnote=15;
if (K[DIK_SLASH])retnote=16;
if (K[DIK_Q])retnote=12;
if (K[DIK_2])retnote=13;
if (K[DIK_W])retnote=14;
if (K[DIK_3])retnote=15;
if (K[DIK_E])retnote=16;
if (K[DIK_R])retnote=17;
if (K[DIK_5])retnote=18;
if (K[DIK_T])retnote=19;
if (K[DIK_6])retnote=20;
if (K[DIK_Y])retnote=21;
if (K[DIK_7])retnote=22;
if (K[DIK_U])retnote=23;
if (K[DIK_I])retnote=24;
if (K[DIK_9])retnote=25;
if (K[DIK_O])retnote=26;
if (K[DIK_0])retnote=27;
if (K[DIK_P])retnote=28;
if (K[DIK_DELETE])retnote=-1;
if (K[DIK_INSERT])retnote=-2;
if (K[DIK_RSHIFT])retnote=-3;
if (K[DIK_BACKSPACE])retnote=-4;
if (K[DIK_0])retvalue=0;
if (K[DIK_1])retvalue=1;
if (K[DIK_2])retvalue=2;
if (K[DIK_3])retvalue=3;
if (K[DIK_4])retvalue=4;
if (K[DIK_5])retvalue=5;
if (K[DIK_6])retvalue=6;
if (K[DIK_7])retvalue=7;
if (K[DIK_8])retvalue=8;
if (K[DIK_9])retvalue=9;
if (K[DIK_A])retvalue=10;
if (K[DIK_B])retvalue=11;
if (K[DIK_C])retvalue=12;
if (K[DIK_D])retvalue=13;
if (K[DIK_E])retvalue=14;
if (K[DIK_F])retvalue=15;
if (K[DIK_DELETE])retvalue=16;
}

if (K[DIK_SPACE] && snamesel==0 && pos_space==1)
{
vloid();
gui_action=29;
if (Songplaying==0)is_editing++;
pos_space=0;
}

if (pos_space==0 && !K[DIK_SPACE])pos_space=1;

if (K[DIK_RCONTROL] && snamesel==0 && po_ctrl2){vloid();plx=0;po_ctrl2=false;gui_action=28;}
if (!K[DIK_RCONTROL] && !po_ctrl2)po_ctrl2=true;

if (!K[DIK_F1] && !K[DIK_F2] && !K[DIK_F3] && !K[DIK_F4] && !K[DIK_F5] && po_shift==0)po_shift=1;
if (!K[DIK_F5] && !K[DIK_F4] && !K[DIK_F3] && !K[DIK_C] && !K[DIK_X] && !K[DIK_V] && !K[DIK_B] && !K[DIK_E] && !K[DIK_Z] && !K[DIK_I] && !K[DIK_R] && !K[DIK_S] && po_ctrl==0)po_ctrl=1;

if (K[DIK_LCONTROL] && po_ctrl==1)
{
	if (K[DIK_S])
	{
		gui_action=102;
		vloid();
		po_ctrl=0;
	}

	if (K[DIK_F3])
	{
		for (int ipcop=0;ipcop<12288;ipcop++)
		{
		*(BuffPatt+ipcop)=*(RawPatterns+(pSequence[cPosition]*12288)+ipcop);
		}

		for (int ipcut=pSequence[cPosition]*12288;ipcut<pSequence[cPosition]*12288+12288;ipcut+=6)
		{
		*(RawPatterns+ipcut)=121;
		*(RawPatterns+ipcut+1)=255;
		*(RawPatterns+ipcut+2)=255;
		*(RawPatterns+ipcut+3)=255;
		*(RawPatterns+ipcut+4)=0;
		*(RawPatterns+ipcut+5)=0;
		}
		freakylines=patternLines[pSequence[cPosition]];		
		po_ctrl=0;
		Actupated(0);
	}
	if (K[DIK_F4])
	{
		int ipcop2=0;
		for (int ipcop=pSequence[cPosition]*12288;ipcop<pSequence[cPosition]*12288+12288;ipcop++)
		{
		*(BuffPatt+ipcop2++)=*(RawPatterns+ipcop);
		}
		vloid();po_ctrl=0;Actupated(0);
		freakylines=patternLines[pSequence[cPosition]];
	}
	if (K[DIK_F5])
	{
		int ipcop2=0;
		for (int ipcop=pSequence[cPosition]*12288;ipcop<pSequence[cPosition]*12288+12288;ipcop++)
		{
		*(RawPatterns+ipcop)=*(BuffPatt+ipcop2++);
		}
		Actupated(0);po_ctrl=0;
		patternLines[pSequence[cPosition]]=freakylines;
	}
	
	
	if (K[DIK_B])
	{
		block_start_track=ped_track;
		block_start=ped_line;
		
		if (block_end_track<block_start_track)
			block_end_track=block_start_track;
	
		if (block_end<block_start)
			block_end=block_start;
	
		if (block_end_track<0)
			block_end_track=0;

		Actupated(0);po_ctrl=0;
	}
	
	if (K[DIK_E])
	{
		block_end_track=ped_track;
		block_end=ped_line;

		if (block_start_track>block_end_track)
			block_start_track=block_end_track;

		if (block_start_track<0)
			block_start_track=0;

		if (block_start>block_end)
			block_start=block_end;
	Actupated(0);po_ctrl=0;
	}


	if (K[DIK_C] && block_start_track!=-1 && block_end_track!=-1)
	{
		int axbc=0;
		int aybc=0;

		for (int ybc=block_start;ybc<block_end+1;ybc++)
		{
			axbc=0;
			for (int xbc=block_start_track;xbc<block_end_track+1;xbc++)
			{
				*(BuffBlock+axbc*6+aybc*96)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96);
				*(BuffBlock+axbc*6+aybc*96+1)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+1);
				*(BuffBlock+axbc*6+aybc*96+2)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+2);
				*(BuffBlock+axbc*6+aybc*96+3)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+3);
				*(BuffBlock+axbc*6+aybc*96+4)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+4);
				*(BuffBlock+axbc*6+aybc*96+5)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+5);		
			axbc++;
			}
		aybc++;
		}

		b_buff_xsize=(block_end_track-block_start_track)+1;
		b_buff_ysize=(block_end-block_start)+1;

	Actupated(0);po_ctrl=0;
	}


	if (K[DIK_X] && block_start_track!=-1 && block_end_track!=-1)
	{
		int axbc=0;
		int aybc=0;

		for (int ybc=block_start;ybc<block_end+1;ybc++)
		{
			axbc=0;
			for (int xbc=block_start_track;xbc<block_end_track+1;xbc++)
			{
				*(BuffBlock+axbc*6+aybc*96)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96);
				*(BuffBlock+axbc*6+aybc*96+1)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+1);
				*(BuffBlock+axbc*6+aybc*96+2)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+2);
				*(BuffBlock+axbc*6+aybc*96+3)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+3);
				*(BuffBlock+axbc*6+aybc*96+4)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+4);
				*(BuffBlock+axbc*6+aybc*96+5)=*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+5);		
			
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96)=121;
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+1)=255;
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+2)=255;
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+3)=255;
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+4)=0;
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+5)=0;		
		
				axbc++;
			}
		aybc++;
		}

		b_buff_xsize=(block_end_track-block_start_track)+1;
		b_buff_ysize=(block_end-block_start)+1;

		block_start=0;
		block_end=0;
		block_start_track=-1;
		block_end_track=-1;

	Actupated(0);po_ctrl=0;
	}

	if (K[DIK_I] && block_start_track!=-1 && block_end_track!=-1)
	{
		int axbc=0;
		int aybc=0;

		int startcmd=*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+block_start*96+4);
		int startvalue=*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+block_start*96+5);
		int endvalue=*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+block_end*96+5);
		int ranlen=block_end-block_start;
		if(ranlen==0)ranlen=1;
		int cran=0;
		int tran=endvalue-startvalue;

		for (int ybc=block_start;ybc<block_end+1;ybc++)
		{
			int c_val=(cran*tran)/ranlen;
			*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+ybc*96+4)=startcmd;
			*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+ybc*96+5)=startvalue+c_val;			
			cran++;
		}

	Actupated(0);po_ctrl=0;
	}

		if (K[DIK_R] && block_start_track!=-1 && block_end_track!=-1)
		{
		int startcmd=*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+block_start*96+4);
		
		for (int ybc=block_start;ybc<block_end+1;ybc++)
		{
			*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+ybc*96+4)=startcmd;
			*(RawPatterns+pSequence[cPosition]*12288+block_start_track*6+ybc*96+5)=rand()/128;			
		}

	Actupated(0);po_ctrl=0;
	}

	if (K[DIK_V] && b_buff_xsize!=-1 && b_buff_ysize!=-1)
	{
		int axbc=0;
		int aybc=0;

		for (int ybc=ped_line;ybc<ped_line+b_buff_ysize;ybc++)
		{
			axbc=0;
			for (int xbc=ped_track;xbc<ped_track+b_buff_xsize;xbc++)
			{
				if (xbc<16 && ybc<128)
				{
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96)=*(BuffBlock+axbc*6+aybc*96);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+1)=*(BuffBlock+axbc*6+aybc*96+1);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+2)=*(BuffBlock+axbc*6+aybc*96+2);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+3)=*(BuffBlock+axbc*6+aybc*96+3);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+4)=*(BuffBlock+axbc*6+aybc*96+4);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+5)=*(BuffBlock+axbc*6+aybc*96+5);		
				}
				axbc++;
			}
			aybc++;
		}

	Actupated(0);po_ctrl=0;
	}

	if (K[DIK_Z] && b_buff_xsize!=-1 && b_buff_ysize!=-1)
	{
		int axbc=0;
		int aybc=0;

		for (int ybc=ped_line;ybc<ped_line+b_buff_ysize;ybc++)
		{
			axbc=0;
			for (int xbc=ped_track;xbc<ped_track+b_buff_xsize;xbc++)
			{
				if (xbc<16 && ybc<128)
				{
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+2)=*(BuffBlock+axbc*6+aybc*96+2);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+3)=*(BuffBlock+axbc*6+aybc*96+3);		
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+4)=*(BuffBlock+axbc*6+aybc*96+4);
				*(RawPatterns+pSequence[cPosition]*12288+xbc*6+ybc*96+5)=*(BuffBlock+axbc*6+aybc*96+5);		
				}
				axbc++;
			}
			aybc++;
		}

	Actupated(0);po_ctrl=0;
	}
}

if (K[DIK_LSHIFT] && po_shift==1)
{
	// Transpose 1 Seminote Down

	if (K[DIK_F1])
	{for(int lcopy=ped_line;lcopy<128;lcopy++)
	{
	int brower=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96));
	
	if (brower<120)
	{brower--;if (brower<0)brower=0;}
	
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96))=brower;
	}
	Actupated(0);po_shift=0;
	}

	// Transpose 1 Seminote Up

	if (K[DIK_F2])
	{for(int lcopy=ped_line;lcopy<128;lcopy++)
	{
	int brower=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96));
	
	if (brower<120)
	{brower++;if (brower>119)brower=119;}
	
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96))=brower;
	}
	Actupated(0);po_shift=0;
	}

	// Cut current track to Track Buffer:

	if (K[DIK_F3])
	{
	for(int lcopy=0;lcopy<128;lcopy++)
	{
	*(BuffTrack+(lcopy*6))=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96));
	*(BuffTrack+(lcopy*6)+1)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+1);
	*(BuffTrack+(lcopy*6)+2)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+2);
	*(BuffTrack+(lcopy*6)+3)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+3);
	*(BuffTrack+(lcopy*6)+4)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+4);
	*(BuffTrack+(lcopy*6)+5)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+5);
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96))=121;
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+1)=255;
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+2)=255;
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+3)=255;
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+4)=0;
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+5)=0;
	}	
	Actupated(0);
	po_shift=0;
	}

	if (K[DIK_F4])
	{for(int lcopy=0;lcopy<128;lcopy++)
	{
	*(BuffTrack+(lcopy*6))=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96));
	*(BuffTrack+(lcopy*6)+1)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+1);
	*(BuffTrack+(lcopy*6)+2)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+2);
	*(BuffTrack+(lcopy*6)+3)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+3);
	*(BuffTrack+(lcopy*6)+4)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+4);
	*(BuffTrack+(lcopy*6)+5)=*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+5);
	}
	po_shift=0;
	}

	// Paste Track Buffer To current Track:

	if (K[DIK_F5])
	{for(int lcopy=0;lcopy<128;lcopy++)
	{
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96))=*(BuffTrack+(lcopy*6));
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+1)=*(BuffTrack+(lcopy*6)+1);
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+2)=*(BuffTrack+(lcopy*6)+2);
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+3)=*(BuffTrack+(lcopy*6)+3);
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+4)=*(BuffTrack+(lcopy*6)+4);
	*(RawPatterns+pSequence[cPosition]*12288+(ped_track*6)+(lcopy*96)+5)=*(BuffTrack+(lcopy*6)+5);
	}
	Actupated(0);
	po_shift=0;
	}
}
if (K[DIK_LSHIFT]){poskeynote=1;poskeyval=1;}
if (poskeyval==0 && retvalue!=ltretvalue)poskeyval=1;

if ( (ped_row==1 || ped_row==3 || ped_row==5 || ped_row==7 || ped_row==9) && retvalue!=120 && poskeyval==1  && is_editing==1)
{

	int ped_cell=1;
	if (ped_row==3)ped_cell=2;
	if (ped_row==5)ped_cell=3;
	if (ped_row==7)ped_cell=4;
	if (ped_row==9)ped_cell=5;
	ltretvalue=retvalue;
	xoffseted=(ped_track*6)+(ped_line*96)+ped_cell;
	
	int oldval=*(RawPatterns+pSequence[cPosition]*12288+xoffseted);
	
	if (retvalue<16)
	{
	if (oldval==255 && ped_row==1)oldval=0;
	if (oldval==255 && ped_row==3)oldval=0;
	if (oldval==255 && ped_row==5)oldval=0;
	oldval=(oldval&0xf)+retvalue*16;
	}
	else
	{
	oldval=0;
	if (ped_row==1)oldval=255;
	if (ped_row==3)oldval=255;
	if (ped_row==5)oldval=255;
	}

	*(RawPatterns+pSequence[cPosition]*12288+xoffseted)=oldval;
	
	
	if(oldval!=255 && ped_row==5 && *(RawPatterns+pSequence[cPosition]*12288+xoffseted)>144)
	*(RawPatterns+pSequence[cPosition]*12288+xoffseted)=144;
	
	ped_line+=ped_pattad;
	Actupated(0);
	poskeyval=0;
}

if ( (ped_row==2 || ped_row==4 || ped_row==6 || ped_row==8 || ped_row==10) && retvalue!=120 && is_editing==1 && poskeyval==1)
{
	int ped_cell=1;
	if (ped_row==2)ped_cell=1;
	if (ped_row==4)ped_cell=2;
	if (ped_row==6)ped_cell=3;
	if (ped_row==8)ped_cell=4;
	if (ped_row==10)ped_cell=5;
	
	ltretvalue=retvalue;
	xoffseted=(ped_track*6)+(ped_line*96)+ped_cell;
	int oldval=*(RawPatterns+pSequence[cPosition]*12288+xoffseted);
	
	if (retvalue<16)
	{
	if (oldval==255 && ped_row==2)oldval=0;
	if (oldval==255 && ped_row==4)oldval=0;
	if (oldval==255 && ped_row==6)oldval=0;
	oldval=(oldval&0xf0)+retvalue;
	}
	else
	{
	oldval=0;
	if (ped_row==2)oldval=255;
	if (ped_row==4)oldval=255;
	if (ped_row==6)oldval=255;
	}

	*(RawPatterns+pSequence[cPosition]*12288+xoffseted)=oldval;
	
	if(oldval!=255 && ped_row==6 && *(RawPatterns+pSequence[cPosition]*12288+xoffseted)>144)
	*(RawPatterns+pSequence[cPosition]*12288+xoffseted)=144;

	ped_line+=ped_pattad;
	Actupated(0);
	poskeyval=0;
}

if (poskeynote==0 && retnote!=ltretnote)poskeynote=1;
  if (ped_row==0 && poskeynote==1 && retnote!=120)
  {
	 ltretnote=retnote;

    xoffseted=ped_track*6+ped_line*96+pSequence[cPosition]*12288;
	
	int tmp_note=retnote+ped_patoct*12;
	if (tmp_note>119)tmp_note=119;
	
	if (retnote>-1)
    {
	if (is_editing){
	*(RawPatterns+xoffseted)=tmp_note; //121
    *(RawPatterns+xoffseted+1)=ped_patsam; //121
	if (!Songplaying)ped_line+=ped_pattad;
	Actupated(0);
	}

	if (c_midiout!=-1)midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ped_track]) | (123 << 8) | (0 << 16)); 
	
	Sp_Playwave(ped_track, (float)tmp_note, ped_patsam,CustomVol[ped_patsam],0,0);
	}

	if (retnote==-3)
    {
	*(RawPatterns+xoffseted)=120; //121
    if (!Songplaying)ped_line+=ped_pattad;
	Actupated(0);
	
	}

	if (retnote==-1)
    {
	*(RawPatterns+xoffseted)=121; //121
    *(RawPatterns+xoffseted+1)=255; //121
	if (!Songplaying)ped_line+=ped_pattad;
	Actupated(0);
	}

	if (retnote==-2)
    {
	for (int interval=126;interval>ped_line-1;interval--)
	{
	xoffseted=ped_track*6+interval*96+pSequence[cPosition]*12288;
	*(RawPatterns+xoffseted+96)=*(RawPatterns+xoffseted);
	*(RawPatterns+xoffseted+97)=*(RawPatterns+xoffseted+1);
	*(RawPatterns+xoffseted+98)=*(RawPatterns+xoffseted+2);
	*(RawPatterns+xoffseted+99)=*(RawPatterns+xoffseted+3);
	*(RawPatterns+xoffseted+100)=*(RawPatterns+xoffseted+4);
	*(RawPatterns+xoffseted+101)=*(RawPatterns+xoffseted+5);
	
	}
	xoffseted=(ped_track*6)+(ped_line*96)+pSequence[cPosition]*12288;
	*(RawPatterns+xoffseted)=121;
    *(RawPatterns+xoffseted+1)=255;
    *(RawPatterns+xoffseted+2)=255;
    *(RawPatterns+xoffseted+3)=255;
    *(RawPatterns+xoffseted+4)=0;
    *(RawPatterns+xoffseted+5)=0;
    Actupated(0);
	}

	// BACKSPACE

	if (retnote==-4)
    {
	for (int interval=ped_line+1;interval<128;interval++)
	{
	xoffseted=ped_track*6+interval*96+pSequence[cPosition]*12288;
	int xoffseted2=pSequence[cPosition]*12288+ped_track*6+(interval*96)-96;

	*(RawPatterns+xoffseted2)=*(RawPatterns+xoffseted);
	*(RawPatterns+xoffseted2+1)=*(RawPatterns+xoffseted+1);
	*(RawPatterns+xoffseted2+2)=*(RawPatterns+xoffseted+2);
	*(RawPatterns+xoffseted2+3)=*(RawPatterns+xoffseted+3);
	*(RawPatterns+xoffseted2+4)=*(RawPatterns+xoffseted+4);
	*(RawPatterns+xoffseted2+5)=*(RawPatterns+xoffseted+5);
	}

	xoffseted=(ped_track*6)+12192+pSequence[cPosition]*12288;
	*(RawPatterns+xoffseted)=121;
    *(RawPatterns+xoffseted+1)=255;
    *(RawPatterns+xoffseted+2)=255;
    *(RawPatterns+xoffseted+3)=255;
    *(RawPatterns+xoffseted+4)=0;
    *(RawPatterns+xoffseted+5)=0;

	Actupated(0);
	}

	poskeynote=0;
  }
  if (Songplaying && player_pos!=cPosition){vloid();gui_action=200;}
  if (GUIMODE)S->flip();
  return 0;
}

void guiDial(int x,int y, int sx,int sy,const char* str,int brill)
{
	x++;
	y++;
	int tbrill1=brill-32;
	int tbrill2=brill/4;
	int tbrill3=brill/2;
	int x2=x+sx;
	int y2=y+sy;
	
	if(brill!=100){
	S->setColor(brill,brill,brill);
	S->line(x, y,x2,y);
	
	S->setColor(tbrill1,tbrill1,tbrill1);
	S->line(x,y+2,x,y2);

	S->setColor(tbrill2,tbrill2,tbrill2);
	S->line(x+2,y2,x2,y2);
	S->line(x2,y+2,x2,y2);
	}else{
	S->setColor(brill,brill,brill);
	S->line(x+2,y2,x2,y2);
	
	S->setColor(tbrill1,tbrill1,tbrill1);
	S->line(x2,y+2,x2,y2);
	
	S->setColor(tbrill2,tbrill2,tbrill2);
	S->line(x, y,x2,y);
	S->line(x,y+2,x,y2);
	}
	
	S->setColor(tbrill3,tbrill3,tbrill3);

	for (int filler=y+2;filler<y2;filler+=2)
	S->line(x+2,filler,x2-2,filler);	
	S->printXY(x+3, y+1, 0x00DDFFFF, str);	
}

void guiDial2(const char* str)
{

	S->setColor(0,72,72);

	for (int filler=330;filler<452;filler+=2)
	S->line(3,filler,637,filler);
	
	S->printXY(5, 330, 0x00112211, str);	
	S->printXY(4, 329, 0x00DDFFFF, str);	
}

void Read_SMPT(void)
{
	lt_ykar=0;
	lt_items=0;	
	int list_counter=0;
	lt_curr=0;
	lt_index=0;
	struct _finddata_t c_file;    
	long hFile;
  
	/* Find first .c file in current directory */
    
	if( (hFile = _findfirst( "*.*", &c_file )) == -1L )
    {
		sprintf(SMPT_LIST[0],"No files in current dir.");
		FILETYPE[0]=0;
		lt_items=1;
	}
	else   
	{
	// The first file
    sprintf(SMPT_LIST[list_counter],c_file.name);
	FILETYPE[list_counter]=c_file.attrib&_A_SUBDIR;
	lt_items++;
	list_counter++;
	
    /* Find the rest of the files (directorys)*/
	
	while( _findnext( hFile, &c_file ) == 0 )            
	{
		if (c_file.attrib&_A_SUBDIR)
		{
		sprintf(SMPT_LIST[list_counter],c_file.name);
		FILETYPE[list_counter]=_A_SUBDIR;
		lt_items++;
		list_counter++;
		}
	}
	// End dir
	_findclose( hFile );
	}

	if( (hFile = _findfirst( "*.*", &c_file )) == -1L )
    {
		sprintf(SMPT_LIST[0],"No files in current dir.");
		FILETYPE[0]=0;
		lt_items=1;
	}
	else   
	{
	// The first file (files)
	if (!(c_file.attrib&_A_SUBDIR))
	{
    sprintf(SMPT_LIST[list_counter],c_file.name);
	FILETYPE[list_counter]=0;
	lt_items++;
	list_counter++;
	}
    /* Find the rest of the files (files)*/
	while( _findnext( hFile, &c_file ) == 0 )            
	{
	if (!(c_file.attrib&_A_SUBDIR))
	{
	sprintf(SMPT_LIST[list_counter],c_file.name);
	FILETYPE[list_counter]=0;
	lt_items++;
	list_counter++;
	}

	} // while      
	
	_findclose( hFile );
	
	}

	/* Save current drive. */   
	int curdrive = _getdrive();

	for (int drivex=1;drivex<16;drivex++)
	{
	sprintf(SMPT_LIST[list_counter],"%c:", drivex + 'A' - 1 );
	FILETYPE[list_counter]=_A_SUBDIR;
	list_counter++;
	lt_items++;
	}
	_chdrive(curdrive);
}

void DumpList(int xr,int yr,int y)
{
	S->setColor(12,22,30);

	bjbox(xr,yr,226+restx,136);

	char buffer[64];   /* Get the current working directory: */
    
	guiDial(454,24,184+restx,16,"",100);
		
	if( _getcwd( buffer,48) == NULL )     
	S->printXY(458,26,0x00ffffff,"Cannot fit path here...");
	else
	S->printXY(458,26,0x00ffffff,buffer);

	for (int counter=0;counter<11;counter++)
	{
	int rel_val=y+counter;
	
	if (y+counter==lt_curr)
	{
	S->setColor(12,52,160);
	bjbox(xr,yr+(counter*12)+2,226+restx,12);
	}	

	if (y+counter<lt_items)
		
		if (FILETYPE[rel_val]==_A_SUBDIR)
		{
			S->printXY(xr,yr+(counter*12), 0x0044ff88,"/");		
			S->printXY(xr+8,yr+(counter*12), 0x00ffffff,SMPT_LIST[rel_val]);		
		}
		else
		{
			S->printXY(xr,yr+(counter*12), 0x00aabbcc,SMPT_LIST[rel_val]);		
		}
	}
}

void bjbox(int x,int y,int sx,int sy)
{
for (int filler=y;filler<y+sy;filler+=2)
	S->line(x,filler,x+sx,filler);
}

void ltActualize(int modeac)
{
int const brolim=lt_items-11;

if (modeac==0)
{
if (lt_ykar>70)
lt_ykar=70;

if (lt_ykar<0)
lt_ykar=0;

lt_index=(lt_ykar*brolim)/70;
}

if (lt_index>brolim)
lt_index=brolim;

if (lt_index<0)
lt_index=0;

if (modeac!=0)
lt_ykar=(lt_index*70)/brolim;

S->setColor(40,50,60);
bjbox(395,59,16,103);
guiDial(394,58+lt_ykar,16,32,"",200);
if (last_index!=lt_index){DumpList(413,43,lt_index);last_index=lt_index;}
}

char zcheckMouse(int x,int y,int xs,int ys)
{
	char scape=0;

	if (gui_pushed==0 && LastX>x && LastX<x+xs && LastY>y && LastY<y+ys)
	{
	gui_lx=x;
	gui_ly=y;
	gui_pushed=1;
	scape=1;
	fluzy=-1;
	}

	if (gui_pushed==1 && x==gui_lx && y==gui_ly)
	scape=1;

	return scape;
}

void vloid(void)
{
if(!GUIMODE)S->copy(MOUSEBACK, LastX-1, LastY-1,0,0, 10,18,0);
}
#endif // AUDACIOUS_UADE

// Bank initializer ---------------------------------------------

void init_sample_bank(void)
{
	RESET303PARAMETERS(&tb303[0]);
	RESET303PARAMETERS(&tb303[1]);

	for (int inico=0;inico<256;inico++)
	{

// All tracks actived on entire sequence (1-Active 0-Inactive)

	for (int inico2=0;inico2<MAX_TRACKS;inico2++)
		SACTIVE[inico][inico2]=true;

	pSequence[inico]=0;

	if(inico<128){
	beatsync[inico]=false;
	beatlines[inico]=16;

	sprintf(nameins[inico],"Unnamed");
	ResetSynthParameters(&PARASynth[inico]);
	
	for (int ced_split=0;ced_split<16;ced_split++)
	{
	RawSamples[inico][0][ced_split]=NULL;
	RawSamples[inico][1][ced_split]=NULL;
	SampleChannels[inico][ced_split]=0;
	SampleType[inico][ced_split]=0;
	LoopStart[inico][ced_split]=0;
	LoopEnd[inico][ced_split]=0;
	LoopType[inico][ced_split]=0;
	SampleNumSamples[inico][ced_split]=0;
	Finetune[inico][ced_split]=0;
	SampleVol[inico][ced_split]=0.0f;
	FDecay[inico][ced_split]=0.0;
	Basenote[inico][ced_split]=48;
	sprintf(SampleName[inico][ced_split],"Unnamed");
	Midiprg[inico]=-1;
	Synthprg[inico]=false;
	CustomVol[inico]=1.0f;
	}
}}
}
#ifndef AUDACIOUS_UADE
void AllocateWave(int n_index, long lenfir, int samplechans)
{

// Freeing if memory was allocated before -----------------------

	if (SampleType[n_index][ped_split]!=0)
	{
		free(RawSamples[n_index][0][ped_split]);

		if (SampleChannels[n_index][ped_split]==2)
		free(RawSamples[n_index][1][ped_split]);
	}

	SampleType[n_index][ped_split]=1;
	SampleChannels[n_index][ped_split]=samplechans;
	RawSamples[n_index][0][ped_split]=(short *)malloc(lenfir*2);
	
	if (samplechans==2)
	RawSamples[n_index][1][ped_split]=(short *)malloc(lenfir*2);		

	LoopStart[n_index][ped_split]=0;
	LoopEnd[n_index][ped_split]=lenfir;
	LoopType[n_index][ped_split]=0;
	SampleNumSamples[n_index][ped_split]=lenfir;
	Finetune[n_index][ped_split]=0;
	SampleVol[n_index][ped_split]=1.0;
}

void WavAlloc(int Freeindex,const char * str)
{
	long Datalen=0;
	int ld0=0,ld1=0,ld2=0,ld3=0;
	int Freeindex2=0;
	int idchk=0;
	FILE *in;
	
	int rate=0;
	int bits=0;
	short *csamples,*csamples2;
	char st_type=0;
	short inx=0;
	int fmtchklen=0;

	const char *Wavfile=str;
	idchk=0;

	if (Wavfile!=NULL && (in = fopen(Wavfile,"rb"))!=NULL)
	{
	char extension[10];
	char modext[5];
	char rebext[5];
	
	sprintf(modext,"NTK.");
	sprintf(rebext,"NTK.");

	fseek(in,1080,SEEK_SET);
	fread(modext, sizeof( char),4,in);
	
	fseek(in,0,SEEK_SET);
	fread(extension, sizeof( char),9,in);
	
	fseek(in,8,SEEK_SET);
	fread(rebext, sizeof( char),4,in);
	
	if (strcmp(rebext,"RB40")==0)
	{
	sprintf(name,"%s",Wavfile);
	LoadRebirthMod();
	}
	else if (strcmp(modext,"M.K.")==0 || strcmp(modext,"M!K!")==0 || strcmp(modext,"FLT4")==0  || strcmp(modext,"4CHN")==0)
	{
	sprintf(name,"%s",Wavfile);
	LoadAmigaMod();
	NewWavy();
	}
	else if (strcmp(extension,"TWNNINS0")==0)
	{
	sprintf(name,"%s",Wavfile);
	LoadInst();
	NewWavy();
	}
	else if (strcmp(extension,"TWNNSNG0")==0 || strcmp(extension,"TWNNSNG1")==0)
	{
	mess_box("Sorry, old beta versions .ntk soundformats is not supported.");
	}
	else if (strcmp(extension,"TWNNSNG2")==0)
	{
	sprintf(name,"%s",Wavfile);
	LoadMod();
	NewWavy();
	}
	else if (strcmp(extension,"TWNNSYN0")==0)
	{
	sprintf(name,"%s",Wavfile);
	LoadSynth();
	}
	else
	{
	mess_box("Attempting of loading a wav file...");	
	
	fseek(in,8,SEEK_SET);
	idchk=fgetc(in)*fgetc(in)*fgetc(in)*fgetc(in);
	
	if (idchk==33556770)
	{
		rewind(in);
		fseek(in,22,SEEK_SET);
		st_type=fgetc(in);
		
		fseek(in,24,SEEK_SET);
		rate=fgetc(in);
		rate+=fgetc(in)*256;

		fseek(in,34,SEEK_SET);
		bits=fgetc(in);
		
		fseek(in,16,SEEK_SET); /* Get length of fmtchk */
		fmtchklen=fgetc(in);
		
		// Set cursor at possible list
		fseek(in,20+fmtchklen,SEEK_SET);
		unsigned int lchk=0;
		fread(&lchk,4,1,in);
		
		if(lchk==1414744396)
		{
		fmtchklen+=8;// LIST HANDLED!

		// handling size of LIST chunk
		unsigned char lelist;
		fread(&lelist,1,1,in);
		fmtchklen+=lelist;
		}

		fseek(in,24+fmtchklen,SEEK_SET);
		ld0=fgetc(in);
		
		fseek(in,25+fmtchklen,SEEK_SET);
		ld1=fgetc(in);
		
		fseek(in,26+fmtchklen,SEEK_SET);
		ld2=fgetc(in);
		
		fseek(in,27+fmtchklen,SEEK_SET);
		ld3=fgetc(in);
		
		Datalen=(ld3*16777216)+(ld2*65536)+(ld1*256)+ld0;
		
		if (bits==16)Datalen/=2;
		
		if (st_type==2)Datalen/=2;
		
		if (st_type==2)
		{
		AllocateWave(Freeindex, Datalen,2);
		csamples=RawSamples[Freeindex][0][ped_split];
		csamples2=RawSamples[Freeindex][1][ped_split];
		}
		else
		{
		AllocateWave(Freeindex, Datalen,1);
		csamples=RawSamples[Freeindex][0][ped_split];
		}

		fseek(in,28+fmtchklen,SEEK_SET);
		
		for(long io=0;io<Datalen;io++)
		{
			if (bits==8)
			{
				inx=fgetc(in)*256;
			}
			else
			{
			inx=fgetc(in);
			inx+=fgetc(in)*256;
			}
			
			if (st_type==2 && bits==16)
			{
			*csamples2=fgetc(in);
			*csamples2+=fgetc(in)*256;
			*csamples2-=65536;
			csamples2++;
			}

			if (st_type==2 && bits==8)
			{
			*csamples2=(fgetc(in)*256)-32768;
			csamples2++;
			}
			
			if (bits==16)
			*csamples=inx-65536;
			else
			*csamples=inx-32768;
			csamples++;
		}

		sprintf(SampleName[Freeindex][ped_split],"%s",Wavfile); 
 

		if (st_type==1 && bits==8)
		mess_box("Mono 8-Bits WAV PCM loaded");	
	
		if (st_type==1 && bits==16)
		mess_box("Mono 16-Bits WAV PCM loaded");	
	
		if (st_type==2 && bits==8)
		mess_box("Stereo 8-Bits WAV PCM loaded");	
	
		if (st_type==2 && bits==16)
		mess_box("Stereo 16-Bits WAV PCM loaded");	

		
		char smpc[16];
		smpc[4]=0;
		fread(&smpc,4,1,in);
		if(strcmp("smpl",smpc)==0)
		{
			// Loop Found
		
			//	fseek(in,48,SEEK_CUR);

			fseek(in,32,SEEK_CUR);

			char pl=0;

			fread(&pl,1,1,in);
			
			if(pl==1)
			{
			fseek(in,15,SEEK_CUR);

			unsigned int ls=0;
			unsigned int le=0;
			fread(&ls,sizeof(int),1,in);
			fread(&le,sizeof(int),1,in);
			LoopStart[Freeindex][ped_split]=ls;
			LoopEnd[Freeindex][ped_split]=le;
			LoopType[Freeindex][ped_split]=1;
			}
		}
		
		Actualize_Patterned();
		Actualize_Sample_Ed(2,0);
		fclose(in);
		NewWavy();
	}
	else
	{
	mess_box("Invalid file ID format. I accept only '.wav' '.ntk' '.nti' or '.nts' or Amiga 'MOD' files.");
	fclose(in);
	}
	}
	}
	else
	{
	mess_box("File loading error. (Probably: file not found)");	
	}
	Actualize_Main_Ed();
}

// Function that search for a free wave on board ----------------

int GetFreeWave(void)
{
	int inico=0;
	int exiter=0;
	int valuer=-1;

	do{
	
	if (SampleType[inico]==0)
	{
		exiter=1;
		valuer=inico;
	}
	inico++;
	}while(exiter==0);
	
	return valuer;
}

void mess_box(char const *str)
{
guiDial3(0,CONSOLE_HEIGHT-21,fsize,18,str,220);
}
		
#include "noisetrekkerp2.cpp"

void Actualize_Sequencer(void)
{

valuer_box2(188,60,cPosition);
valuer_box2(188,78,pSequence[cPosition]);
Anat(cPosition);
valuer_box2(188,96,sLength);
valuer_box(188,114,patternLines[pSequence[cPosition]]);

if (userscreen==4)Actualize_Seq_Ed();
}
#endif // AUDACIOUS_UADE

int AllocPattern(void)
{
for (int api=0;api<128;api++)
patternLines[api]=64;

nPatterns=1;

if ((RawPatterns=(unsigned char *)malloc(PBLEN))!=NULL) // 6144 bytes per pattern
{
	for (int inicial=0;inicial<PBLEN;inicial+=6)
	{
	*(RawPatterns+inicial)=121;//121
	*(RawPatterns+inicial+1)=255;//255
	*(RawPatterns+inicial+2)=255;//255
	*(RawPatterns+inicial+3)=255;//255
	*(RawPatterns+inicial+4)=0;//0
	*(RawPatterns+inicial+5)=0;//0
	}
return 1; // Allocated
}
else
{
return 0; // Not allocated
}

} // End of alloc pattern

void FreeAll(void)
{
	// Freeing Allocated Samples

	for (int freer=0;freer<128;freer++)
	{
		for(char pedsplit=0;pedsplit<16;pedsplit++)
		{
		// mvtiaine: added NULLing
		if (SampleType[freer][pedsplit]!=0) {
			free(RawSamples[freer][0][pedsplit]);
			RawSamples[freer][0][pedsplit]=NULL;
			if (SampleChannels[freer][pedsplit]==2) {
				free(RawSamples[freer][1][pedsplit]);
				RawSamples[freer][1][pedsplit]=NULL;
			}
			}
		}
	}
}
#ifndef AUDACIOUS_UADE
void draw_pated(int pattern,int track, int line, int petrack, int row)
{
S->endDraw();

int rel=0;
int offset_t;
int dover=0;
int color=0;
int ypb1=9;
int ypb2=16;
bool multip=false;

int tvisiblecolums=visiblecolums;
if (tvisiblecolums>Songtracks)
tvisiblecolums=Songtracks;

for (int liner=0;liner<tvisiblecolums;liner++)
{
	color=liner*118;

	if(SACTIVE[cPosition][track+liner])
	Letter(82+color,187,23,0,0);
	else
	Letter(82+color,187,24,0,0);
	
	if(TRACKSTATE[track+liner])
	Letter(51+color,187,25,0,0);
	else
	Letter(51+color,187,26,0,0);
	
	Letter(72+color,186,liner+track,18,25);
	
}

int y=YVIEW+VIEWLINE2*8;
		
for (liner=VIEWLINE2;liner<VIEWLINE;liner++)
	{
	rel=liner+line;

	if (rel>-1 && rel<patternLines[pSequence[cPosition]])
		{
		
		if (liner!=0)
		{
		if(rel%4==0)
			multip=true;
		else
			multip=false;

		if(multip)
		{
		Letter(0,y,rel>>4,121,128);
		Letter(8,y,rel&0xf,121,128);
		}
		else
		{
		Letter(0,y,rel>>4,0,7);
		Letter(8,y,rel&0xf,0,7);
		}
		
		for (int tracky=0;tracky<tvisiblecolums;tracky++)
			{
			
			if (track+tracky>=block_start_track && track+tracky<=block_end_track && rel>=block_start && rel<=block_end){ypb1=9;ypb2=16;}else{ypb1=0;ypb2=7;}
			
			if(multip){ypb1+=121;ypb2+=121;}

			dover=tracky*118;
			offset_t=(rel*96+((track+tracky)*6))+pattern*12288;
			
			unsigned char p_a=*(RawPatterns+offset_t);
			unsigned char p_b=*(RawPatterns+offset_t+1);
			unsigned char p_bh=p_b&0xf;
			unsigned char p_c=*(RawPatterns+offset_t+2);
			unsigned char p_ch=p_c&0xf;
			unsigned char p_d=*(RawPatterns+offset_t+3);
			unsigned char p_dh=p_d&0xf;
			unsigned char p_e=*(RawPatterns+offset_t+4);
			unsigned char p_eh=p_e&0xf;
			unsigned char p_f=*(RawPatterns+offset_t+5);
			unsigned char p_fh=p_f&0xf;
			
			blitnote(24+dover,y,p_a,ypb1,ypb2);
			
			if(p_b!=255)
			{
			Letter(48+dover,y,p_b>>4,ypb1,ypb2);
			Letter(56+dover,y,p_bh,ypb1,ypb2);
			}
			else
			{
			Letter(48+dover,y,21,ypb1,ypb2);
			Letter(56+dover,y,21,ypb1,ypb2);
			}

			if(p_c!=255)
			{
			Letter(64+dover,y,p_c>>4,ypb1,ypb2);
			Letter(72+dover,y,p_ch,ypb1,ypb2);
			}
			else
			{
			Letter(64+dover,y,21,ypb1,ypb2);
			Letter(72+dover,y,21,ypb1,ypb2);
			}

			if(p_d!=255)
			{
			Letter(80+dover,y,p_d>>4,ypb1,ypb2);
			Letter(88+dover,y,p_dh,ypb1,ypb2);
			}
			else
			{
			Letter(80+dover,y,21,ypb1,ypb2);
			Letter(88+dover,y,21,ypb1,ypb2);
			}

			 Letter(96+dover,y,p_e>>4,ypb1,ypb2);
			Letter(104+dover,y,p_eh,ypb1,ypb2);
			Letter(112+dover,y,p_f>>4,ypb1,ypb2);
			Letter(120+dover,y,p_fh,ypb1,ypb2);

			}// Track
			}// Line!=0
		else
		{y+=8;}

		y+=8;
		}// Line
		else
		{
		Letter(0,y,20,0,7);
		Letter(8,y,20,0,7);
		
		int tvc=24;
		for(int tvcc=0;tvcc<tvisiblecolums;tvcc++)
		{
		Letter(tvc,y,22,0,0);
		tvc+=118;
		}

		y+=8;
		}
		S->setBKColor(0,0,0);
		}

}

void Actupated(int modac)
{

if (is_editing>1)is_editing=0;
int nlines=patternLines[pSequence[cPosition]];

if (ped_line<0)ped_line+=nlines;
if (ped_line>=nlines)ped_line-=nlines;
if (ped_row>10){ped_row=0;ped_track++;}
if (ped_row<0){ped_row=10;ped_track--;}
if (ped_track>Songtracks-1){ped_track=0;gui_track=0;modac=0;}
if (ped_track<0){ped_track=Songtracks-1;gui_track=Songtracks-(visiblecolums+1);if(gui_track<0)gui_track=0;modac=0;}
if (ped_track>=gui_track+visiblecolums){gui_track++;modac=0;}
if (ped_track<gui_track){gui_track--;modac=0;}

draw_pated(pSequence[cPosition],gui_track,ped_line,ped_track,ped_row);

if (is_editing)
Letter(2,186,27,0,0);
else
Letter(2,186,28,0,0);

draw_pated2(pSequence[cPosition],gui_track,ped_line,ped_track,ped_row);

}

void draw_pated2(int pattern,int track, int line, int petrack, int row)
{
S->endDraw();
int ypb1;
int ypb2;
int offset_t;
int dover=0;

		Letter(0,YVIEW,line>>4,53,68);
		Letter(8,YVIEW,line&0xf,53,68);

		int tvisiblecolums=visiblecolums;
		
		if (tvisiblecolums>Songtracks)
			tvisiblecolums=Songtracks;

		for (int tracky=0;tracky<tvisiblecolums;tracky++)
			{
			dover=tracky*118;
	
			char tri=track+tracky;

			if (tri>=block_start_track && tri<=block_end_track && line>=block_start && line<=block_end){ypb1=87;ypb2=102;}else{ypb1=53;ypb2=68;}

			offset_t=line*96+tri*6+pattern*12288;

			unsigned char p_a=*(RawPatterns+offset_t);
			unsigned char p_b=*(RawPatterns+offset_t+1);
			unsigned char p_bh=p_b&0xf;
			unsigned char p_c=*(RawPatterns+offset_t+2);
			unsigned char p_ch=p_c&0xf;
			unsigned char p_d=*(RawPatterns+offset_t+3);
			unsigned char p_dh=p_d&0xf;
			unsigned char p_e=*(RawPatterns+offset_t+4);
			unsigned char p_eh=p_e&0xf;
			unsigned char p_f=*(RawPatterns+offset_t+5);
			unsigned char p_fh=p_f&0xf;
			
			// Note

			if(row==0 && tri==petrack)
			blitnote(24+dover,YVIEW,p_a,70,85);
			else
			blitnote(24+dover,YVIEW,p_a,ypb1,ypb2);

			// Instrument

			if(row==1 && tri==petrack)
			{
				if(p_b!=255)
				Letter(48+dover,YVIEW,p_b>>4,70,85);
				else
				Letter(48+dover,YVIEW,21,70,85);
			}
			else
			{
				if(p_b!=255)
				Letter(48+dover,YVIEW,p_b>>4,ypb1,ypb2);
				else
				Letter(48+dover,YVIEW,21,ypb1,ypb2);
			}
			
			if(row==2 && tri==petrack)
			{
				if(p_b!=255)
				Letter(56+dover,YVIEW,p_bh,70,85);
				else
				Letter(56+dover,YVIEW,21,70,85);
			}
			else
			{
				if(p_b!=255)
				Letter(56+dover,YVIEW,p_bh,ypb1,ypb2);
				else
				Letter(56+dover,YVIEW,21,ypb1,ypb2);
			}

			// Volume

			if(row==3 && tri==petrack)
			{
				if(p_c!=255)
				Letter(64+dover,YVIEW,p_c>>4,70,85);
				else
				Letter(64+dover,YVIEW,21,70,85);
			}
			else
			{
				if(p_c!=255)
				Letter(64+dover,YVIEW,p_c>>4,ypb1,ypb2);
				else
				Letter(64+dover,YVIEW,21,ypb1,ypb2);
			}
			
			if(row==4 && tri==petrack)
			{
				if(p_c!=255)
				Letter(72+dover,YVIEW,p_ch,70,85);
				else
				Letter(72+dover,YVIEW,21,70,85);
			}
			else
			{
				if(p_c!=255)
				Letter(72+dover,YVIEW,p_ch,ypb1,ypb2);
				else
				Letter(72+dover,YVIEW,21,ypb1,ypb2);
			}

			// Panning!

			if(row==5 && tri==petrack)
			{
				if(p_d!=255)
				Letter(80+dover,YVIEW,p_d>>4,70,85);
				else
				Letter(80+dover,YVIEW,21,70,85);
			}
			else
			{
				if(p_d!=255)
				Letter(80+dover,YVIEW,p_d>>4,ypb1,ypb2);
				else
				Letter(80+dover,YVIEW,21,ypb1,ypb2);
			}
			
			if(row==6 && tri==petrack)
			{
				if(p_d!=255)
				Letter(88+dover,YVIEW,p_dh,70,85);
				else
				Letter(88+dover,YVIEW,21,70,85);
			}
			else
			{
				if(p_d!=255)
				Letter(88+dover,YVIEW,p_dh,ypb1,ypb2);
				else
				Letter(88+dover,YVIEW,21,ypb1,ypb2);
			}

			if(row==7 && tri==petrack)
			Letter(96+dover,YVIEW,p_e>>4,70,85);
			else
			Letter(96+dover,YVIEW,p_e>>4,ypb1,ypb2);
			
			if(row==8 && tri==petrack)
			Letter(104+dover,YVIEW,p_eh,70,85);
			else
			Letter(104+dover,YVIEW,p_eh,ypb1,ypb2);
			
			if(row==9 && tri==petrack)
			Letter(112+dover,YVIEW,p_f>>4,70,85);
			else
			Letter(112+dover,YVIEW,p_f>>4,ypb1,ypb2);
			
			if(row==10 && tri==petrack)
			Letter(120+dover,YVIEW,p_fh,70,85);
			else
			Letter(120+dover,YVIEW,p_fh,ypb1,ypb2);
			
			if(sr_isrecording)
			{
			if (liveparam>0 && ped_track==tri && Songplaying)
			{
				if (livevalue<0)livevalue=0;
				if (livevalue>255)livevalue=255;
				
				switch(liveparam){
				case 1:
				*(RawPatterns+offset_t+4)=8;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 2:
				*(RawPatterns+offset_t+4)=20;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 3:
				*(RawPatterns+offset_t+4)=17;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 4:
				*(RawPatterns+offset_t+4)=18;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 5:
				*(RawPatterns+offset_t+4)=19;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 6:
				if(livevalue>128)livevalue=128;
				*(RawPatterns+offset_t+3)=livevalue;
				break;
				case 7:
				*(RawPatterns+offset_t+4)=51;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 8:
				*(RawPatterns+offset_t+4)=52;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 9:
				*(RawPatterns+offset_t+4)=53;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 10:
				*(RawPatterns+offset_t+4)=54;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 11:
				*(RawPatterns+offset_t+4)=55;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 12:
				*(RawPatterns+offset_t+4)=56;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 13:
				*(RawPatterns+offset_t+4)=57;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				case 14:
				*(RawPatterns+offset_t+4)=58;
				*(RawPatterns+offset_t+5)=livevalue;
				break;
				
				}// Close switch
				liveparam=0;
			}// Close liveparam updated
		}// Close is recording
  }
}

void Actualize_Patterned(void)
{
if (ped_pattad<0)ped_pattad=16;
if (ped_pattad>16)ped_pattad=0;
if (ped_patoct<0)ped_patoct=0;
if (ped_patoct>8)ped_patoct=8;
if (ped_patsam<0)ped_patsam=127;
if (ped_patsam>127)ped_patsam=0;

char tcp[30];
sprintf(tcp,"%s_",nameins[ped_patsam]);

if (snamesel==2)
guiDial(90,134,166,16,tcp,100);
else
guiDial(90,134,166,16,nameins[ped_patsam],120);

valuer_box2(90,152,ped_pattad);
valuer_box(258,152,ped_patoct);
valuer_box(258,134,ped_patsam);
}
#endif // AUDACIOUS_UADE

// Main Player Routine ---------------------------------------------

void Sp_Player(void)
{
	if(Songplaying)
	{
		if (PosInTick==0)
		{
			Subicounter=0;
			
			Patbreak=127;

			for (int ct=0;ct<Songtracks;ct++)
			{
				int efactor=ct*6+ped_line*96+pSequence[cPosition]*12288;
				
				pl_note=*(RawPatterns+efactor);
				pl_sample=*(RawPatterns+efactor+1);	
				pl_vol_row=*(RawPatterns+efactor+2);
				pl_pan_row=*(RawPatterns+efactor+3);
				pl_eff_row=*(RawPatterns+efactor+4);
				pl_dat_row=*(RawPatterns+efactor+5);
			
				if (pl_vol_row<=64)sp_Tvol[ct]=(float)pl_vol_row*0.015625f; // Setting volume.
				if (pl_eff_row==3)sp_Tvol[ct]=(float)pl_dat_row*0.0039062f; // Setting volume.
				if (pl_pan_row<=128)
				{
					TPan[ct]=(float)pl_pan_row*0.0078125f; 
					ComputeStereo(ct);
				}

				live303(pl_eff_row,pl_dat_row);
				if(pl_eff_row==49){track3031=ct;Fire303(pl_dat_row,0);}
				if(pl_eff_row==50){track3032=ct;Fire303(pl_dat_row,1);}
#ifndef AUDACIOUS_UADE
				/* MidiController commands */
				if (pl_pan_row==144 && c_midiout!=-1 && pl_eff_row<128)
				midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ct]) | (pl_eff_row << 8) | (pl_dat_row << 16));
				
				if (pl_eff_row==128 && c_midiout!=-1 && pl_dat_row<128)
				midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ct]) | (0 << 8) | (pl_dat_row << 16));
#endif // AUDACIOUS_UADE
				if (pl_eff_row==13 && pl_dat_row<64)Patbreak=pl_dat_row;
				
				if (pl_note<120)
				{
			
				int toffset=0;
				glide=0;
				if (pl_eff_row==9)toffset=pl_dat_row;
				else if (pl_eff_row==5)glide=1;
	
				if (pl_vol_row<=64 || pl_eff_row==3)
				Sp_Playwave(ct, (float)pl_note, pl_sample,sp_Tvol[ct],toffset,glide);
				else
				Sp_Playwave(ct, (float)pl_note, pl_sample,CustomVol[pl_sample],toffset,glide);
				}
				
				if(pl_note==120)
				{
					// Note OFF on track ct
					if (sp_Stage[ct]==1)
					sp_Stage[ct]=2;
				
					Synthesizer[ct].SynthNoteOff();
					noteoff303(ct); // 303 Note Off...
#ifndef AUDACIOUS_UADE
					if (c_midiout!=-1)
					midiOutShortMsg(midiout_handle, (176+ct) | (123 << 8) | (0 << 16)); 					
#endif
				}

			}// For loop
			Go303();
			
			}// Pos in tick == 0

			if (!SubCounter)DoEffects();
			
			SubCounter++;
			
			if (SubCounter>SamplesPerSub-1){SubCounter=0;Subicounter++;}
			
			PosInTick++;
			if (PosInTick>SamplesPerTick+shufflestep)
			{
				shuffleswitch=-shuffleswitch;
				
				if (shuffleswitch==1)
				shufflestep=-((SamplesPerTick*shuffle)/200);
				else
				shufflestep=(SamplesPerTick*shuffle)/200;
				SubCounter=0;
				PosInTick=0;

				if (Patbreak>63)
				{ped_line++;}
				else{
				ped_line=Patbreak;
				cPosition++;
				if (cPosition>=sLength)cPosition=0;
				}

				if (ped_line==patternLines[pSequence[cPosition]])
				{
					ped_line=0;

					if (!plx)cPosition++;
					if (cPosition>=sLength)cPosition=0;
				}
			}
		}
			
	left_float=0;
	right_float=0;
	delay_left_final=1.0f;
	delay_right_final=1.0f;

	for (char c=0;c<Songtracks;c++)
	{
	grown=false;
	currsygnal=0;
	currsygnal2=0;
	bool gotsome=false;
	
	if (sp_Stage[c]==2)
		{
		// Note Stop
			sp_Tvol[c]=0.0f;
			
			if (sp_Cvol[c]<0.01)
				sp_Stage[c]=0;
		}
	
	if (sp_Stage[c]!=0)
	{	
		gotsome=true;
		if (sp_Cvol[c]>sp_Tvol[c])
			sp_Cvol[c]-=0.005f;
		else
			sp_Cvol[c]+=0.005f;
		
		// Interpolation algorithm

		Currentpointer=sp_Position[c].half.first;
		res_dec=sp_Position[c].half.last;

		currsygnal=Resampler.Work(
			*(Player_WL[c]+(Currentpointer > 0 ? Currentpointer - 1 : Currentpointer)), // mvtiaine: fixed buffer underflow
			*(Player_WL[c]+Currentpointer),
			*(Player_WL[c]+Currentpointer+1),
			*(Player_WL[c]+Currentpointer+2),res_dec,Currentpointer,Rns[c])*sp_Cvol[c]*Player_SV[c];
	
		if (Player_SC[c]==2)
		{
		grown=true;
		currsygnal2=Resampler.Work(
			*(Player_WR[c]+(Currentpointer > 0 ? Currentpointer - 1 : Currentpointer)), // mvtiaine: fixed buffer underflow
			*(Player_WR[c]+Currentpointer),
			*(Player_WR[c]+Currentpointer+1),
			*(Player_WR[c]+Currentpointer+2),res_dec,Currentpointer,Rns[c])*sp_Cvol[c]*Player_SV[c];
		}
		
		// End of Interpolation algo
		
		sp_Position[c].absolu+=Vstep1[c];

		if (Player_LT[c]==1 && sp_Position[c].half.first>=Player_LE[c])sp_Position[c].half.first-=Player_LL[c];
		if (Player_LT[c]==0 && sp_Position[c].half.first>=Player_NS[c])sp_Stage[c]=0;
		
		}//sp!!0
		
		if(track3031==c){currsygnal+=tb303engine[0].tbGetSample();gotsome=true;}
		if(track3032==c){currsygnal+=tb303engine[1].tbGetSample();gotsome=true;}
		if (Synthesizer[c].ENV1_STAGE){currsygnal+=Synthesizer[c].SynthGetSample();gotsome=true;}

		if(gotsome)
		{
		if (FType[c]!=4) /* Track filter actived */
		{
		float const dfi = TCut[c]-CCut[c];
		if (dfi<-1.0 || dfi>1.0)
			CCut[c]+=dfi*ICut[c];
	
		if (FType[c]<4){
		gco=f2i(ApplyLfo(CCut[c]-ramper[c],c));
		
		ramper[c]+=Player_FD[c]*gco*0.015625f;
		
		coef[0]=coeftab[0][gco][FRez[c]][FType[c]];
		coef[1]=coeftab[1][gco][FRez[c]][FType[c]];
		coef[2]=coeftab[2][gco][FRez[c]][FType[c]];
		coef[3]=coeftab[3][gco][FRez[c]][FType[c]];
		coef[4]=coeftab[4][gco][FRez[c]][FType[c]];
		currsygnal=Filter(currsygnal+1,c);
		}
		else
		{
		float const realcut=ApplyLfo(CCut[c]-ramper[c],c);

		ramper[c]+=Player_FD[c]*realcut*0.015625f;
		
		switch(FType[c])
		{
		case 5:
		currsygnal=filter2p(c,currsygnal+1,realcut,(float)FRez[c]);
		break;
		
		case 6:
		currsygnal=filter2p(c,currsygnal+1,realcut,(float)FRez[c]);
		currsygnal=filter2p24d(c,currsygnal+1,realcut,(float)FRez[c]);
		break;
		
		case 7:
		currsygnal=filter2p(c,currsygnal+1,realcut,(float)FRez[c]);
		currsygnal2=filter2p24d(c,currsygnal2+1,realcut,(float)FRez[c]);
		break;

		case 8:
		currsygnal=filterRingMod(c,currsygnal,realcut,(float)FRez[c]);
		break;

		case 9:
		currsygnal=filterRingMod(c,currsygnal,realcut,(float)FRez[c]);
		currsygnal2=filterRingModStereo(c,currsygnal2);
		break;

		case 10:
		currsygnal=filterWater(c,currsygnal,realcut,(float)FRez[c]);
		break;
		
		case 11:
		currsygnal=filterWater(c,currsygnal,realcut,(float)FRez[c]);
		currsygnal2=filterWaterStereo(c,currsygnal2,realcut,(float)FRez[c]);
		break;

		case 12:
		currsygnal=filterBellShaped(c,currsygnal,realcut,(float)FRez[c],-15);
		break;

		case 13:
		currsygnal=filterBellShaped(c,currsygnal,realcut,(float)FRez[c],-6);
		break;

		case 14:
		currsygnal=filterBellShaped(c,currsygnal,realcut,(float)FRez[c],6);
		break;
		
		case 15:
		currsygnal=filterBellShaped(c,currsygnal,realcut,(float)FRez[c],15);
		break;
		
		case 16:
		currsygnal=filterDelta(c,currsygnal,realcut,(float)FRez[c]);
		break;

		case 17:
		currsygnal=int_filter2p(c,currsygnal,realcut,(float)FRez[c],0.25f);
		break;
		
		case 18:
		currsygnal=int_filter2p(c,currsygnal,realcut,(float)FRez[c],0.56f);
		break;
		
		case 19:
		currsygnal=int_filter2p(c,currsygnal,realcut,(float)FRez[c],0.78f);
		break;

		case 20:
		currsygnal=int_filter2p(c,currsygnal,realcut,(float)FRez[c],0.96f);
		break;

		case 21:
		currsygnal=filterhp(c,currsygnal+1,realcut,(float)FRez[c]);
		break;

		case 22:
		currsygnal=filterhp(c,currsygnal+1,realcut,(float)FRez[c]);
		currsygnal2=filterhp2(c,currsygnal2+1,realcut,(float)FRez[c]);
		break;

		case 23:
		currsygnal=filterhp(c,currsygnal+1,realcut,(float)FRez[c]);
		currsygnal=filterhp2(c,currsygnal+1,realcut,(float)FRez[c]);
		break;
		
		}//SWITCHCASE [FILTERS]
	}		
	}	/* Filter end */

	
	if (!grown)currsygnal2=currsygnal;

	if (Disclap[c]) /* Distortion */
	{
	if (currsygnal>DThreshold[c])
	currsygnal=DClamp[c];
	else if(currsygnal<-DThreshold[c])
	currsygnal=-DClamp[c];

	if (currsygnal2>DThreshold[c])
	currsygnal2=DClamp[c];
	else if(currsygnal2<-DThreshold[c])
	currsygnal2=-DClamp[c];
	}

	currsygnal*=LVol[c];
	currsygnal2*=RVol[c];
	
	// 32-Bit HQ Interpolated System Flanger

	if (FLANGER_ON[c])
	{
	FLANGE_LEFTBUFFER[c][FLANGER_OFFSET[c]]=currsygnal*FLANGER_AMOUNT[c]+oldspawn[c]*FLANGER_FEEDBACK[c];
	FLANGE_RIGHTBUFFER[c][FLANGER_OFFSET[c]]=currsygnal2*FLANGER_AMOUNT[c]+roldspawn[c]*FLANGER_FEEDBACK[c];
	
	float const fstep1=(float)pow(2.0,sin(FLANGER_GR[c])*FLANGER_AMPL[c]);
	float const fstep2=(float)pow(2.0,sin(FLANGER_GR[c]+FLANGER_DEPHASE[c])*FLANGER_AMPL[c]);

	foff2[c]+=fstep1;
	foff1[c]+=fstep2;	
	
	if (foff2[c]>=16384.0f)foff2[c]=0.0f;
	if (foff1[c]>=16384.0f)foff1[c]=0.0f;
	if (foff2[c]<0.0f)foff2[c]=0.0f;
	if (foff1[c]<0.0f)foff1[c]=0.0f;

	oldspawn[c]=FLANGE_LEFTBUFFER[c][f2i(foff2[c])]+1.0f;
	roldspawn[c]=FLANGE_RIGHTBUFFER[c][f2i(foff1[c])]+1.0f;
	currsygnal+=oldspawn[c];
	currsygnal2+=roldspawn[c];
	
	if (++FLANGER_OFFSET[c]>16383)FLANGER_OFFSET[c]=0;
	FLANGER_GR[c]+=FLANGER_RATE[c];
	
		if (FLANGER_GR[c]>=6.283185f){
		FLANGER_GR[c]-=6.283185f;
		foff2[c]=float(FLANGER_OFFSET[c]-FLANGER_DELAY[c]);
		foff1[c]=float(FLANGER_OFFSET[c]-FLANGER_DELAY[c]);}
	}

	if(!TRACKSTATE[c])// Is the track mute or unmute ???
	{

	// Dry Send

	left_float+=currsygnal;
	right_float+=currsygnal2;

	// Sending to delay...
	
	float const DS=DSend[c];

	if (DS>0.008f)
	{
	delay_left_final+=currsygnal*DS;
	delay_right_final+=currsygnal2*DS;
	}

	// Sending to chorus

	float const DC=CCoef[c];

	if (DC>0.008f)
	{
	left_chorus+=currsygnal*DC;
	right_chorus+=currsygnal2*DC;
	}

	}//Close trackstate
	}//Gotsomething?
	}// Fortracker

}

void Sp_Playwave(int channel, float note, int sample, float vol, unsigned int offset, int glide)
{
	FADEMODE[channel]=0;
	
	if(sample==255)
	{
		sample=sp_channelsample[channel];
		vol=sp_Tvol[channel];
	}

	if (SACTIVE[cPosition][channel])
	{
	char split=0;
	int mnote=int(note);
	
	if (Synthprg[sample])
	{
	Synthesizer[channel].SynthChangeParameters(PARASynth[sample]);
	Synthesizer[channel].SynthNoteOn(note-48,vol);
	}

	for(char revo=0;revo<16;revo++)
	{
	if (note>=Basenote[sample][revo] && SampleType[sample][revo]!=0)
	split=revo;
	}

	note-=Basenote[sample][split];
	note+=float((float)Finetune[sample][split]*0.0078125f);
	
	if (sample!=sp_channelsample[channel] || !sp_Stage[channel])
		glide=0;
 
	if (SampleType[sample][split])
	{
	sp_Stage[channel]=1;

	if (!offset)
		sp_Cvol[channel]=vol;
	else
		sp_Cvol[channel]=0;
	
	sp_Tvol[channel]=vol;
	
	if(beatsync[sample])
	{
	double spreadnote=(double)(SampleNumSamples[sample][split])/((double)beatlines[sample]*(double)SamplesPerTick);
	spreadnote*=4294967296.0f;
	Vstep1[channel]=spreadnote;
	sp_Step[channel]=spreadnote;
	}
	else
	{
	double spreadnote=(double)pow(2.0f,note/12.0f);
	spreadnote*=4294967296.0f;
	
	if(glide)
	sp_Step[channel]=spreadnote;
	else
	{
	Vstep1[channel]=spreadnote;
	sp_Step[channel]=spreadnote;}
	}
	
	sp_channelsample[channel]=sample;
	sp_split[channel]=split;
	
	// Player Pointer Assignment

	Player_LS[channel]=LoopStart[sample][split];
	Player_LE[channel]=LoopEnd[sample][split];
	
	Player_LL[channel]=Player_LE[channel]-Player_LS[channel];
	
	Player_NS[channel]=SampleNumSamples[sample][split]-2;
	Rns[channel]=SampleNumSamples[sample][split];
	Player_SV[channel]=SampleVol[sample][split];
	Player_LT[channel]=LoopType[sample][split];
	Player_SC[channel]=SampleChannels[sample][split];
	Player_FD[channel]=FDecay[sample][split];
	Player_WL[channel]=RawSamples[sample][0][split];
	
	if(SampleChannels[sample][split]==2)
	Player_WR[channel]=RawSamples[sample][1][split];
	
	if (!glide)
	{
	if(!AMIMODE)
	sp_Position[channel].half.first=(offset*SampleNumSamples[sample][split])>>8;
	else
	sp_Position[channel].half.first=offset<<8;

	if(sp_Position[channel].half.first>=SampleNumSamples[sample][split])sp_Stage[channel]=0;
		
	ramper[channel]=0;
	}

	}
#ifndef AUDACIOUS_UADE
	if(TRACKSTATE[channel]==0 && c_midiout!=-1 && Midiprg[sample]!=-1)
	{
	/* haz esta linea si se cambia el programa midi */
	if (LastProgram[TRACKMIDICHANNEL[channel]]!=Midiprg[sample])
	{
	midiOutShortMsg(midiout_handle, (192+TRACKMIDICHANNEL[channel]) | (Midiprg[sample] << 8) | (127 << 16)); 
	LastProgram[TRACKMIDICHANNEL[channel]]=Midiprg[sample];
	}
	/* suelta la nota */
	float veloc=vol*mas_vol;
	midiOutShortMsg(midiout_handle, (144+TRACKMIDICHANNEL[channel]) | (mnote << 8) | (f2i(veloc*255)<< 16));
	}
#endif
	}
}

void SongPlay(void)
{
SongStop();
clipc=0;
liveparam=0;
livevalue=0;
player_pos=-1;
shufflestep=shuffle;
shuffleswitch=-1;

if (plx==0)
{
guiDial(8,60,80,16,"Playing Song",100);
mess_box("Playing song...");
}
else
{
guiDial(8,60,80,16,"Playing Pattern",100);
mess_box("Playing pattern...");
}
SamplesPerTick=(int)((60 * SamplesPerSec) / (BeatsPerMin * TicksPerBeat));  
PosInTick=0; 
SamplesPerSub=SamplesPerTick/6;
ped_line=0;
Songplaying=1;
for (int spl=0;spl<MAX_TRACKS;spl++)
CCoef[spl]=float((float)CSend[spl]/127.0);

if (shuffleswitch==1)
shufflestep=-((SamplesPerTick*shuffle)/200);
else
shufflestep=(SamplesPerTick*shuffle)/200;
}

void StartRec(void)
{
liveparam=0;
livevalue=0;
if (sr_isrecording)
guiDial(8,78,80,16,"Slider Rec: ON",100);
else
guiDial(8,78,80,16,"Slider Rec: OFF",200);
}

void SongStop(void)
{
guiDial(8,60,80,16,"Play Sng/Pttrn",200);
mess_box("Ready...");
Songplaying=0;
tb303engine[0].tbPattern=255;
tb303engine[0].tbLine=255;
tb303engine[1].tbPattern=255;
tb303engine[1].tbLine=255;
track3031=255;
track3032=255;

MidiAllNotesOff();

for (int stopper=0;stopper<MAX_TRACKS;stopper++)
{sp_Stage[stopper]=0;Synthesizer[stopper].SynthNoteOff();}
	
}

void Iniplayer(void)
{
SamplesPerTick=(int)((60 * SamplesPerSec) / (BeatsPerMin * TicksPerBeat));  
Feedback=0.5f;
rx1=rx2=ry1=ry2=rx12=rx22=ry12=ry22=0.0f;
MidiReset();
allPassInit(c_threshold);

for (int ini=0;ini<MAX_TRACKS;ini++)
	{
	FADEMODE[ini]=0;
	FADECOEF[ini]=0.0f;
	Synthesizer[ini].SynthReset();
	Player_FD[ini]=0.0f;
	ResetFilters(ini);
	sp_channelsample[ini]=0;
	sp_split[ini]=0;

	TRACKMIDICHANNEL[ini]=ini;
	TRACKSTATE[ini]=0;
	oldspawn[ini]=0.0f;
	roldspawn[ini]=0.0f;
	LFO_ON[ini]=0;
	LFORATE[ini]=0.0001f;
	LFOAMPL[ini]=0;
	LFOGR[ini]=0;
	FLANGER_AMOUNT[ini]=-0.8f;
	FLANGER_DEPHASE[ini]=0.0174532f;
	FLANGER_ON[ini]=0;
	FLANGER_RATE[ini]=0.0068125f/57.29578f;
	FLANGER_AMPL[ini]=0.0f;
	FLANGER_GR[ini]=0;
	FLANGER_FEEDBACK[ini]=-0.51f;
	FLANGER_DELAY[ini]=176;
	FLANGER_OFFSET[ini]=176;
    foff2[ini]=0.0f;
	foff1[ini]=0.0f;
	for (int ini2=0;ini2<16400;ini2++)
	{
	FLANGE_LEFTBUFFER[ini][ini2]=0.0f;
	FLANGE_RIGHTBUFFER[ini][ini2]=0.0f;
	}

	sp_Cvol[ini]=0.0f;
	sp_Tvol[ini]=0.0f;
	DSend[ini]=0.0;
	CSend[ini]=0;
	Vstep1[ini]=0;
	glidestep[ini]=0;
	TPan[ini]=0.5f;
	ComputeStereo(ini);
	sp_Position[ini].absolu=0;
	CCut[ini]=126.0;
	TCut[ini]=126.0;
	ICut[ini]=0.0039062f;
	FType[ini]=4;
	FRez[ini]=64;
	DThreshold[ini]=65535;
	Disclap[ini]=false;
	Dispan[ini]=false;

	DClamp[ini]=65535;
	CCoef[ini]=0;	
	}

	for (int dini=0;dini<131071;dini++)
	{
	lbuff_chorus[dini]=0;
	rbuff_chorus[dini]=0;
	}

for (int cutt=0;cutt<128;cutt++)
{
for (int rezz=0;rezz<128;rezz++)
{
for (int typp=0;typp<4;typp++)
{
ComputeCoefs(cutt,rezz,typp);
coeftab[0][cutt][rezz][typp]=coef[0];		
coeftab[1][cutt][rezz][typp]=coef[1];
coeftab[2][cutt][rezz][typp]=coef[2];
coeftab[3][cutt][rezz][typp]=coef[3];
coeftab[4][cutt][rezz][typp]=coef[4];
}
}
}

}
#ifndef AUDACIOUS_UADE
void Actualize_Master(char gode)
{

if (gode==0 || gode==1)
{
if (BeatsPerMin<32)BeatsPerMin=32;
if (BeatsPerMin>255)BeatsPerMin=255;
valuer_box(324,78,BeatsPerMin);
}

if (gode==0 || gode==2)
{
if (TicksPerBeat<1)TicksPerBeat=1;
if (TicksPerBeat>16)TicksPerBeat=16;
valuer_box(324,96,TicksPerBeat);
}

if (gode==0)
valuer_box2(324,60,Songtracks);

if (gode==4)
{
valuer_box2(324,60,Songtracks);
S->setColor(0,0,0);

if(userscreen==4)
{S->setColor(0,0,0);
bjbox(257,346,124,86);
Actualize_Seq_Ed();
}

if (userscreen!=8)
{
S->endDraw();
S->copy(FRAMEWORK, 0, 184);
}
else
bjbox(0,186,fsize,248);

Actupated(0);
}

SamplesPerTick=(int)((60 * SamplesPerSec) / (BeatsPerMin * TicksPerBeat));  
float SamplesPerBeat=(float)SamplesPerSec/(((float)BeatsPerMin*4)/60);
SamplesPerSub=SamplesPerTick/6;

if(userscreen==5)Actualize_Master_Ed(3);
}

void draw_tracked(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",100);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

guiDial2("Track: Properties & Fx Send");

guiDial3(508,344,60,16,"Track",200);
guiDial3(570,344,60,16,"Delay Send",200);
guiDial(508,406,60,16,"Solo Track",200);
guiDial(508,424,60,16,"Un-mute All",200);

guiDial3(8,344,224,96,"Analog Filter Emulation",128);
guiDial3(18,360,56,16,"CutOff Frq.",200);
guiDial3(18,378,56,16,"Resonance",200);
guiDial3(18,396,56,16,"Type",200);
guiDial3(18,414,56,16,"Inertia",200);

guiDial3(240,344,260,96,"Distorsion/Reverb/Pan",128);
guiDial3(248,360,56,16,"Threshold",200);
guiDial3(248,378,56,16,"Clamp",200);
guiDial3(248,396,56,16,"Reverb",200);
guiDial3(248,414,56,16,"Pan",200);
guiDial(456,360,40,16,"Flat2C",200);
guiDial(456,378,40,16,"Flat2T",200);
guiDial(456,414,40,16,"Center",200);
guiDial3(570,388,60,16,"Midi Chnl.",200);
}
#endif // AUDACIOUS_UADE
float Filter( float x, char i)
{
        float y;
		y = coef[0]*x + coef[1]*fx1[i] + coef[2]*fx2[i] + coef[3]*fy1[i] + coef[4]*fy2[i];
		fy2[i]=fy1[i];
        fy1[i]=y;
        fx2[i]=fx1[i];
        fx1[i]=x;
        return y;
}

float RevFilter(float x)
{
		float y=x;
        return y;
}

float RevFilter2(float x)
{

		float y=x;
        return y;
}

void ComputeCoefs(int freq, int r, int t)
{
    float omega =float (2*PI*Kutoff(freq)/44100);
    float sn = (float)sin( omega);
    float cs = (float)cos( omega);
    float alpha;
        
	if( t<2)
    alpha =float(sn / Reonance( r *(freq+70)/(127.0f+70)));
    else
    alpha =float (sn * sinh( Bandwidth( r) * omega/sn));

        float a0, a1, a2, b0, b1, b2;

        switch( t)
		{
    
		case 0: // LP
                b0 =  (1 - cs)/2;
                b1 =   1 - cs;
                b2 =  (1 - cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 1: // HP
                b0 =  (1 + cs)/2;
                b1 = -(1 + cs);
                b2 =  (1 + cs)/2;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 2: // BP
                b0 =   alpha;
                b1 =   0;
                b2 =  -alpha;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
        case 3: // BR
                b0 =   1;
                b1 =  -2*cs;
                b2 =   1;
                a0 =   1 + alpha;
                a1 =  -2*cs;
                a2 =   1 - alpha;
                break;
		}

        coef[0] = b0/a0;
        coef[1] = b1/a0;
        coef[2] = b2/a0;
        coef[3] = -a1/a0;
        coef[4] = -a2/a0;
}
	

float Kutoff( int v)
{
        return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
}

float Reonance( float v)
{
        return float(pow( v/127.0, 4)*150+0.1);
}

float Bandwidth( int v)
{
        return float(pow( v/127.0, 4)*4+0.1);
}
#ifndef AUDACIOUS_UADE
void Actualize_Track_Ed(char gode)
{
if (userscreen==1)
{

if (gode==0){
if (ped_track>MAX_TRACKS-1)ped_track=0;
if (ped_track<0)ped_track=MAX_TRACKS-1;
valuer_box(508,362,ped_track);}

if (gode==0 || gode==1){
if (TCut[ped_track]<0)TCut[ped_track]=0;
if (TCut[ped_track]>127)TCut[ped_track]=127;
Realslider(78,360,(int)TCut[ped_track]);}

if (gode==0 || gode==2){
if (FRez[ped_track]<0)FRez[ped_track]=0;
if (FRez[ped_track]>127)FRez[ped_track]=127;
Realslider(78,378,FRez[ped_track]);}

if (gode==0 || gode==3){
if (ICut[ped_track]>0.0078125f)ICut[ped_track]=0.0078125f;
if (ICut[ped_track]<0.00006103515625f)ICut[ped_track]=0.00006103515625f;
Realslider(78,414,f2i(ICut[ped_track]*16384.0f));}

if (gode==0 || gode==4){
switch(FType[ped_track]){
case 0:guiDial(139,396,86,16,"LowPass -12db",200);break;
case 1:guiDial(139,396,86,16,"HiPass",200);break;
case 2:guiDial(139,396,86,16,"BandPass",200);break;
case 3:guiDial(139,396,86,16,"BandReject",200);break;
case 4:guiDial(139,396,86,16,"Filter Off",200);break;
case 5:guiDial(139,396,86,16,"LowPass -24db",200);break;
case 6:guiDial(139,396,86,16,"LowPass -48db",200);break;
case 7:guiDial(139,396,86,16,"LP-24 [Stereo]",200);break;
case 8:guiDial(139,396,86,16,"A.Mod. [Mono]",200);break;
case 9:guiDial(139,396,86,16,"A.Mod. [Stereo]",200);break;
case 10:guiDial(139,396,86,16,"Single [Mono]",200);break;
case 11:guiDial(139,396,86,16,"Single [Stereo]",200);break;
case 12:guiDial(139,396,86,16,"ParaEQ -15db",200);break;
case 13:guiDial(139,396,86,16,"ParaEQ -6db",200);break;
case 14:guiDial(139,396,86,16,"ParaEQ +6db",200);break;
case 15:guiDial(139,396,86,16,"ParaEQ +15db",200);break;
case 16:guiDial(139,396,86,16,"Custom Delta",200);break;
case 17:guiDial(139,396,86,16,"Comp.Distort L",200);break;
case 18:guiDial(139,396,86,16,"Comp.Distort M",200);break;
case 19:guiDial(139,396,86,16,"Comp.Distort H",200);break;
case 20:guiDial(139,396,86,16,"Comp.Distort!!",200);break;
case 21:guiDial(139,396,86,16,"W-HP12[Mono]",200);break;
case 22:guiDial(139,396,86,16,"W-HP12[Stereo]",200);break;
case 23:guiDial(139,396,86,16,"W-HP24[Mono]",200);break;
}
valuer_box(77,396,FType[ped_track]);
}

if (gode==0 || gode==5){
if (DSend[ped_track]<0)DSend[ped_track]=0;
if (DSend[ped_track]>1.0)DSend[ped_track]=1.0;
Realslider(308,396,int(DSend[ped_track]*128.0));}

if (gode==0 || gode==6){
if (CSend[ped_track]<0)CSend[ped_track]=0;
if (CSend[ped_track]>127)CSend[ped_track]=127;
valuer_box(570,362,CSend[ped_track]);
CCoef[ped_track]=float((float)CSend[ped_track]/127.0);}

if (gode==0 || gode==7){
if (DThreshold[ped_track]<0)DThreshold[ped_track]=0;
if (DThreshold[ped_track]>65535)DThreshold[ped_track]=65535;
Realslider(308,360,(int)DThreshold[ped_track]/512);}

if (gode==0 || gode==8){
if (DClamp[ped_track]<0)DClamp[ped_track]=0;
if (DClamp[ped_track]>65535)DClamp[ped_track]=65535;
Realslider(308,378,(int)DClamp[ped_track]/512);}

if (gode==0 || gode==9){
if (TPan[ped_track]<0.0f)TPan[ped_track]=0.0f;
if (TPan[ped_track]>1.0f)TPan[ped_track]=1.0f;
ComputeStereo(ped_track);
Realslider(308,414,f2i(TPan[ped_track]*127.0f));}

if (gode==0 || gode==10){
if (TRACKSTATE[ped_track])
guiDial(508,388,60,16,"Un-Mute",100);
else
guiDial(508,388,60,16,"Mute Track",200);}

if (gode==0 || gode==11){
if (TRACKMIDICHANNEL[ped_track]>15)TRACKMIDICHANNEL[ped_track]=0;
if (TRACKMIDICHANNEL[ped_track]<0)TRACKMIDICHANNEL[ped_track]=15;
valuer_box2(570,406,TRACKMIDICHANNEL[ped_track]+1);}

if (gode==0 || gode==12)
{
if(Disclap[ped_track])
guiDial(570,424,60,16,"Distort On",100);
else
guiDial(570,424,60,16,"Distort Off",200);
}

if (trkchan==true)
{
Actupated(0);
trkchan=false;
}
	
}// Userscreen match found

}

void Realslider(int x,int y,int val)
{
S->setColor(0,192,0);
bjbox(x+2,y+4,val,12);

S->setColor(0,128,0);
S->line(x+2,y+2,x+2+val,y+2);
S->line(x+2,y+16,x+2+val,y+16);

guiDial(x+2+val,y+1,13,14,"",240);

S->setColor(0,0,0);
bjbox(x+2+val+16,y+2,128-val,16);
}

void Realslider2(int x,int y,int val)
{
S->setColor(0,192,0);
bjbox(x+2,y+4,val,12);

S->setColor(0,128,0);
S->line(x+2,y+2,x+2+val,y+2);
S->line(x+2,y+16,x+2+val,y+16);

guiDial(x+2+val,y+1,13,14,"",240);

S->setColor(0,0,0);
bjbox(x+2+val+16,y+2,64-val,16);
}
#endif // AUDACIOUS_UADE
inline int f2i(double d)
{
  const double magic = 6755399441055744.0; // 2^51 + 2^52
  double tmp = (d-0.5) + magic;
#ifdef WORDS_BIGENDIAN
  return *((int*) &tmp + 1);
#else
  return *(int*) &tmp;
#endif
}
#ifndef AUDACIOUS_UADE
void draw_fxed(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",100);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

guiDial2("FX: Setup");
guiDial3(8,344,224,104,"Reverb Setup",128);
guiDial3(18,360,56,16,"Switch",200);
guiDial3(18,378,56,16,"Feedback",200);
guiDial3(18,396,56,16,"Type",200);
guiDial3(18,414,56,16,"Room Size",200);
guiDial3(18,432,56,16,"Filter",200);

guiDial3(240,344,288,96,"Stereo Delay Settings",128);
guiDial3(248,360,56,16,"L.Delay",200);
guiDial3(248,378,56,16,"R.Delay",200);
guiDial3(248,396,56,16,"L.Decay",200);
guiDial3(248,414,56,16,"R.Decay",200);
guiDial3(530,344,104,56,"Tick Synchro",128);
guiDial(540,360,16,16,"x1",200);
guiDial(540,378,16,16,"x1",200);
guiDial(558,360,16,16,"x2",200);
guiDial(558,378,16,16,"x2",200);
guiDial(576,360,16,16,"x3",200);
guiDial(576,378,16,16,"x3",200);
guiDial(594,360,16,16,"x4",200);
guiDial(594,378,16,16,"x4",200);
guiDial(612,360,16,16,"x5",200);
guiDial(612,378,16,16,"x5",200);
}

void Actualize_Fx_Ed(char gode)
{
if (gode==0 || gode==1){
if(DelayType<0)DelayType=0;
if(DelayType>6)DelayType=6;
switch (DelayType)
{
case 0:guiDial3(139,396,86,16,"Room",200);break;
case 1:guiDial3(139,396,86,16,"Great Hall",200);break;
case 2:guiDial3(139,396,86,16,"Room 2",200);break;
case 3:guiDial3(139,396,86,16,"Echoy",200);break;
case 4:guiDial3(139,396,86,16,"1Comb",200);break;
case 5:guiDial3(139,396,86,16,"Room2",200);break;
case 6:guiDial3(139,396,86,16,"Hall3",200);break;

default:guiDial3(139,396,86,16,"Not Defined",200);break;
}

if (gode)Initreverb();
valuer_box(77,396,DelayType);
}

if (gode==0 || gode==2)
Realslider(78,378,f2i(Feedback*127.0));

if (gode==0 || gode==3){
if (lchorus_delay>22100)lchorus_delay=22100;
if (lchorus_delay<1)lchorus_delay=1;
if(gode==3){lchorus_counter=44100;lchorus_counter2=44100-lchorus_delay;}
Realslider(308,360,lchorus_delay/175);
outlong(458,360,(lchorus_delay*1000)/44100,2);}

if (gode==0 || gode==4){
if (rchorus_delay>22100)rchorus_delay=22100;
if (rchorus_delay<1)rchorus_delay=1;
if(gode==4){rchorus_counter=44100;rchorus_counter2=44100-rchorus_delay;}
Realslider(308,378,rchorus_delay/175);
outlong(458,378,(rchorus_delay*1000)/44100,2);}

if (gode==0 || gode==5){
if (lchorus_feedback>0.95f)lchorus_feedback=0.95f;
if (lchorus_feedback<0)lchorus_feedback=0;
Realslider(308,396,f2i(lchorus_feedback*127));
outlong(458,396,f2i(lchorus_feedback*100),1);}

if (gode==0 || gode==6){
if (rchorus_feedback>0.95f)rchorus_feedback=0.95f;
if (rchorus_feedback<0)rchorus_feedback=0;
Realslider(308,414,f2i(rchorus_feedback*127));
outlong(458,414,f2i(rchorus_feedback*100),1);}

if (gode==0 || gode==7)
{
if (c_threshold<10)c_threshold=10;
if (c_threshold>127)c_threshold=127;
Realslider(78,414,c_threshold);
allPassInit(c_threshold);
}

if (gode==0 || gode==8)
{
if(compressor==0)
{
guiDial(78,360,32,16,"On",200);
guiDial(112,360,32,16,"Off",100);
}
else
{
guiDial(78,360,32,16,"On",100);
guiDial(112,360,32,16,"Off",200);
}
}

if (gode==0 || gode==9)
{
if (REVERBFILTER<0.05f)REVERBFILTER=0.05f;
if (REVERBFILTER>0.99f)REVERBFILTER=0.99f;
Realslider(78,432,f2i(REVERBFILTER*128.0f));
}
}

void draw_sampleed(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",100);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

switch(seditor){
case 0:
guiDial2("Instrument Editor [Sampler]");
guiDial(268,346,88,16,"Loop Fine Editor",200);
guiDial(268,364,88,16,"Export .Wav File",200);

guiDial3(382,409,42,16,"Amplify",200);
guiDial3(382,427,42,16,"Tune",200);

guiDial3(382,337,64,16,"Length",200);
guiDial3(382,355,64,16,"Loop Start",200);
guiDial3(382,373,64,16,"Loop End",200);
guiDial3(382,391,64,16,"Loop Play",200);
guiDial3(8,408,42,16,"F.Decay",200);
guiDial3(8,426,42,16,"Def.Vol",200);
guiDial3(510,337,58,16,"Split",200);
guiDial3(510,355,58,16,"BaseNote",200);
guiDial3(510,373,58,16,"Midi Prg.",200);
guiDial3(510,391,58,16,"Synth Prg.",200);
guiDial3(8,372,134,16,"Loop Tempo Auto Synchro",200);
guiDial3(8,390,134,16,"Synchro Length (In Lines)",200);
break;

case 1:
guiDial2("Instrument Editor [Loop Editor]");
guiDial3(424,355,58,16,"LS. Value",200);
guiDial3(424,373,58,16,"LE. Value",200);
guiDial(424,391,58,16,"Exit Editor",200);
guiDial3(20,350,56,16,"Loop Start",200);
guiDial(78,350,16,16,"<<",250);
guiDial(96,350,16,16,"<",250);
guiDial(176,350,16,16,">",250);
guiDial(194,350,16,16,">>",250);
guiDial3(220,350,56,16,"Loop End",200);
guiDial(278,350,16,16,"<<",250);
guiDial(296,350,16,16,"<",250);
guiDial(376,350,16,16,">",250);
guiDial(394,350,16,16,">>",250);

break;
}//Switch Sampler Screen

}

void Actualize_Sample_Ed(int typex, char gode)
{
CheckLoops();

if(userscreen==2){

switch(seditor)
{
case 0:

if (gode==0 || gode==1){
if (SampleVol[ped_patsam][ped_split]>4.0f)SampleVol[ped_patsam][ped_split]=4.0f;
if (SampleVol[ped_patsam][ped_split]<0)SampleVol[ped_patsam][ped_split]=0;
Realslider(426,408,f2i(SampleVol[ped_patsam][ped_split]*32.0f));
outlong(575,409,f2i(SampleVol[ped_patsam][ped_split]*100.0f),1);
}

if (gode==0 || gode==2){
if (Finetune[ped_patsam][ped_split]>127)Finetune[ped_patsam][ped_split]=127;
if (Finetune[ped_patsam][ped_split]<-127)Finetune[ped_patsam][ped_split]=-127;
Realslider(426,426,64+(Finetune[ped_patsam][ped_split]/2));
outlong(575,427,(long)Finetune[ped_patsam][ped_split],0);
}

if (gode==0 || gode==3){
if (FDecay[ped_patsam][ped_split]>0.015625f)FDecay[ped_patsam][ped_split]=0.015625f;
if (FDecay[ped_patsam][ped_split]<0.0f)FDecay[ped_patsam][ped_split]=0.0f;
Realslider(52,408,f2i(FDecay[ped_patsam][ped_split]*8192.0f));
outlong(201,408,f2i(FDecay[ped_patsam][ped_split]*8192.0f),0);
}

if (gode==0)
{
if (typex==1 || typex==2)
{
if (SampleType[ped_patsam][ped_split]!=0)
{
char temprout[256];
if (SampleChannels[ped_patsam][ped_split]==2)
sprintf(temprout,"%s [Stereo]",SampleName[ped_patsam][ped_split]);
else
sprintf(temprout,"%s [Mono]",SampleName[ped_patsam][ped_split]);
guiDial3(7,347,256,16,temprout,200);
}
else
{
guiDial3(7,347,256,16,"No Sample Loaded",200);
}
}
}// typex

if (gode==0 || gode==4)
outlong(448,337,SampleNumSamples[ped_patsam][ped_split],0);

if (gode==0 || gode==5){
outlong(448,355,LoopStart[ped_patsam][ped_split],0);
outlong(448,373,LoopEnd[ped_patsam][ped_split],0);

if (LoopType[ped_patsam][ped_split])
{
	guiDial(448,391,29,16,"On",100);
	guiDial(479,391,29,16,"Off",200);
}
else
{
	guiDial(448,391,29,16,"On",200);
	guiDial(479,391,29,16,"Off",100);
}
}

if (gode==0 || gode==8)
{valuer_box(570,337,ped_split);
valuer_box3(570,355,Basenote[ped_patsam][ped_split]);}

if (gode==0 || gode==9)
valuer_box3(570,355,Basenote[ped_patsam][ped_split]);

if (gode==0 || gode==10)
valuer_box(570,373,Midiprg[ped_patsam]+1);

if (gode==0 || gode==11){
if(Synthprg[ped_patsam]){
	guiDial(570,391,28,16,"Off",200);
	guiDial(602,391,28,16,"On",100);
}else{
	guiDial(570,391,28,16,"Off",100);
	guiDial(602,391,28,16,"On",200);}
}


if (gode==0 || gode==12){
if(beatsync[ped_patsam]){
	guiDial(144,372,28,16,"Off",200);
	guiDial(174,372,28,16,"On",100);
}else{
	guiDial(144,372,28,16,"Off",100);
	guiDial(174,372,28,16,"On",200);}

}

if (gode==0 || gode==13)valuer_box2(144,390,beatlines[ped_patsam]);

if (gode==14)
{
guiDial(268,364,88,16,"Export .Wav File",100);
char buffer[64];

mess_box("Writting Wav Header And Sample Data...");

WaveFile RF;

RF.OpenForWrite(SampleName[ped_patsam][ped_split],44100,16,SampleChannels[ped_patsam][ped_split]);

bool t_stereo;

if(SampleChannels[ped_patsam][ped_split]==1)
t_stereo=false;
else 
t_stereo=true;

long woff=0;

short *eSamples=RawSamples[ped_patsam][0][ped_split];
short *erSamples=RawSamples[ped_patsam][1][ped_split];

while(woff<SampleNumSamples[ped_patsam][ped_split]){
if (t_stereo)
RF.WriteStereoSample(*eSamples++,*erSamples++);
else
RF.WriteMonoSample(*eSamples++);

woff++;
}

RF.Close();
sprintf(buffer,"File '%s' saved...",SampleName[ped_patsam][ped_split]);
mess_box(buffer);

guiDial(268,364,88,16,"Export .Wav File",200);
}

if (gode==0 || gode==15){
if (CustomVol[ped_patsam]>1.0f)CustomVol[ped_patsam]=1.0f;
if (CustomVol[ped_patsam]<0.0f)CustomVol[ped_patsam]=0.0f;
Realslider(52,426,f2i(CustomVol[ped_patsam]*128.0f));
outlong(201,426,f2i(CustomVol[ped_patsam]*100.0f),1);
}

break;

case 1:
if(typex==0)
{
outlong(484,355,*(RawSamples[ped_patsam][0][ped_split]+LoopStart[ped_patsam][ped_split]),0);
outlong(114,350,LoopStart[ped_patsam][ped_split],0);
actuloop=1;
}

if(typex==1)
{
outlong(484,373,*(RawSamples[ped_patsam][0][ped_split]+LoopEnd[ped_patsam][ped_split]),0);
outlong(314,350,LoopEnd[ped_patsam][ped_split],0);
actuloop=2;
}

if(typex==2)
{
outlong(484,355,*(RawSamples[ped_patsam][0][ped_split]+LoopStart[ped_patsam][ped_split]),0);
outlong(484,373,*(RawSamples[ped_patsam][0][ped_split]+LoopEnd[ped_patsam][ped_split]),0);
outlong(114,350,LoopStart[ped_patsam][ped_split],0);
outlong(314,350,LoopEnd[ped_patsam][ped_split],0);
actuloop=3;
}

break;
}//SWitch draw sampleeditor

}//User screen==2
}

void outlong(int x,int y, long cant,int mode)
{
char xstr[40];
switch (mode){
case 0:sprintf(xstr,"%d",cant);break;
case 1:sprintf(xstr,"%d%%",cant);break;
case 2:sprintf(xstr,"%d ms.",cant);break;
case 3:sprintf(xstr,"%d Hz.",cant);break;
case 5:sprintf(xstr,"%d K.",cant);break;
case 6:sprintf(xstr,"%d Degr.",cant);break;
case 10:sprintf(xstr,"S: %d",cant);break;
case 11:sprintf(xstr,"E: %d",cant);break;
case 12:sprintf(xstr,"L: %d",cant);break;
}
guiDial3(x,y,60,16,xstr,150);
}

void outfloat(int x,int y, float cant,int mode)
{
char xstr[40];

switch (mode){
case 0:sprintf(xstr,"%.3f",cant);break;
case 1:sprintf(xstr,"%.2f%%",cant);break;
case 2:sprintf(xstr,"%.3f ms.",cant);break;
case 3:sprintf(xstr,"%.3f Hz.",cant);break;
case 5:sprintf(xstr,"%.3f K.",cant);break;
case 8:sprintf(xstr,"%.1f Tk.",cant);break;
}

guiDial3(x,y,60,16,xstr,150);
}
#endif // AUDACIOUS_UADE

void DoEffects(void)
{

for (int trackef=0;trackef<Songtracks;trackef++)
{
	int tefactor=trackef*6+ped_line*96+pSequence[cPosition]*12288;
	int pltr_note=*(RawPatterns+tefactor);
	int pltr_sample=*(RawPatterns+tefactor+1);
	unsigned char pltr_vol_row=*(RawPatterns+tefactor+2);
	int pltr_pan_row=*(RawPatterns+tefactor+3);
	__int64 pltr_eff_row=*(RawPatterns+tefactor+4);
	__int64 pltr_dat_row=*(RawPatterns+tefactor+5);
	
	if (Subicounter==0)
	{
		if (pltr_note==121 && pltr_sample!=255)sp_Tvol[trackef]=CustomVol[pltr_sample];
		   
	}

	// AUTOFADE ROUTINE

	switch(FADEMODE[trackef])
		{
		case 1:
			sp_Tvol[trackef]+=FADECOEF[trackef];
			
			if(sp_Tvol[trackef]>1.0f)
			{	sp_Tvol[trackef]=1.0f;
			FADEMODE[trackef]=0;}
		break;

		case 2:
			sp_Tvol[trackef]-=FADECOEF[trackef];
			
			if(sp_Tvol[trackef]<0.0f)
			{	sp_Tvol[trackef]=0.0f;
			FADEMODE[trackef]=0;}
		break;
	}

	// EFFECTS

	if ((pltr_vol_row&0xf0)==0xf0) /* Note Cut: Fx */
	{
		unsigned char kinder=pltr_vol_row&0xf;
		if (Subicounter==kinder )
		{
		if(sp_Stage[trackef]!=0)
		{
			if(FType[trackef]==4)
			sp_Stage[trackef]=2;
			else
			sp_Tvol[trackef]=0.001f;
		}

		Synthesizer[trackef].SynthNoteOff();
#ifndef AUDACIOUS_UADE
		if (c_midiout!=-1)midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[trackef]) | (123 << 8) | (0 << 16)); 
#endif
		}
	}
	
	if (sp_Stage[trackef]!=0)
	{
	
		switch(pltr_eff_row)
		{
			// d01 $01 Pitch Up 
			case 1:
			if(Vstep1[trackef]<137438953472)Vstep1[trackef]+=pltr_dat_row<<21;
			break;
			
			// d02 $02 Pitch Down 
			case 2:
			Vstep1[trackef]-=pltr_dat_row<<21;
			if(Vstep1[trackef]<16)Vstep1[trackef]=16;
			break;
			
			case 3:
			sp_Tvol[trackef]=pltr_dat_row*0.0039062f;
			break;
			
			case 4: // Slicer
			if (Subicounter==0)
			sp_Tvol[trackef]=1.0f;
			
			if (Subicounter>=pltr_dat_row)
			sp_Tvol[trackef]=0;
			break;

			case 5: // Glider
			if(pltr_dat_row)glidestep[trackef]=pltr_dat_row<<22;

			if (Vstep1[trackef]<sp_Step[trackef])
			{
			Vstep1[trackef]+=glidestep[trackef];
			if (Vstep1[trackef]>sp_Step[trackef])Vstep1[trackef]=sp_Step[trackef];
			}
			else if (Vstep1[trackef]>sp_Step[trackef])
			{
			Vstep1[trackef]-=glidestep[trackef];
			if (Vstep1[trackef]<sp_Step[trackef])Vstep1[trackef]=sp_Step[trackef];
			}
			break;

		}// SWITCH CASES
	}//IF PLAYING TRACK...

		switch(pltr_eff_row)
		{
			// d08 $08 SetCutOff 
			case 8:
			TCut[trackef]=(float)pltr_dat_row/2.0f;
			break;

			// d10 $0a SetRandomCutOff 
			case 10:
				if (Subicounter==0){
			TCut[trackef]=float((float)(rand()/256)/((float)pltr_dat_row+1.0));
			
			if (TCut[trackef]<1)
			TCut[trackef]=1;

			if (TCut[trackef]>127)
				TCut[trackef]=127;}

			break;

			// d11 $0b SlideUpCutOff 
			case 11:
			if (Subicounter==0){TCut[trackef]+=pltr_dat_row;if (TCut[trackef]>127)TCut[trackef]=127;}
			break;

			// d12 $0c SlideDownCutOff 
			case 12:
			if (Subicounter==0){TCut[trackef]-=pltr_dat_row;if (TCut[trackef]<1)TCut[trackef]=1;}
			break;

			// d14 $0e Retrigger Command
			case 14: 
				
				if (pltr_dat_row>0 && (Subicounter % pltr_dat_row)==0){			
					if (pltr_vol_row<=64)
					Sp_Playwave(trackef, (float)pltr_note, pltr_sample,pltr_vol_row*0.015625f,0,0);
					else
					Sp_Playwave(trackef, (float)pltr_note, pltr_sample,CustomVol[pltr_sample],0,0);
				}
			break;

			
			// SET BPM
			case 240:
			if(pltr_dat_row>32)
			{
			BeatsPerMin=pltr_dat_row;
			SamplesPerTick=(int)((60 * SamplesPerSec) / (BeatsPerMin * TicksPerBeat));  
			SamplesPerSub=SamplesPerTick/6;
			}
			break;

			/* &15 Speed */
			case 15:
			TicksPerBeat=pltr_dat_row;
			if (TicksPerBeat<1)TicksPerBeat=1;
			if (TicksPerBeat>16)TicksPerBeat=16;
			SamplesPerTick=(int)((60 * SamplesPerSec) / (BeatsPerMin * TicksPerBeat));  
			SamplesPerSub=SamplesPerTick/6;
			if (shuffleswitch==1)
			shufflestep=-((SamplesPerTick*shuffle)/200);
			else
			shufflestep=(SamplesPerTick*shuffle)/200;
			break;
		
			// d16 $10 Send to delay Command
			case 16: 
				CCoef[trackef]=(float)pltr_dat_row/255.0f;
			break;

			// d17 $11 Send to reverb Command
			case 17: 
			DSend[trackef]=(float)pltr_dat_row/255.0f;
			break;

			// d18 $12 Set Threshold
			case 18: 
			DThreshold[trackef]=(float)pltr_dat_row*128.0f;
			break;
			
			// d19  $13 Clamp
			case 19: 
			DClamp[trackef]=(float)pltr_dat_row*128.0f;
			break;
			
			// d20  $14 Filter Reso
			case 20: 
			FRez[trackef]=pltr_dat_row/2;
			break;
			
			// d21 $15 Filter Type
			case 21: 
			if (pltr_dat_row<=MAX_FILTER)FType[trackef]=pltr_dat_row;
			break;
		
			 /* &16 Reset lfo */
			case 22:
			LFOGR[trackef]=0.0f;
			break;
		
			case 23: 
			if(pltr_dat_row>0)
			{
			FADECOEF[trackef]=0.1666667f/(float)pltr_dat_row;
			FADEMODE[trackef]=1;
			}
			break;
		
			case 24: 
			if(pltr_dat_row>0)
			{
			FADECOEF[trackef]=0.1666667f/(float)pltr_dat_row;
			FADEMODE[trackef]=2;
			}
			break;

			case 25: 
			sp_Tvol[trackef]+=pltr_dat_row*0.0039062f;
			if(sp_Tvol[trackef]>1.0f)sp_Tvol[trackef]=1.0f;
			break;
		
			case 26: 
			sp_Tvol[trackef]-=pltr_dat_row*0.0039062f;
			if(sp_Tvol[trackef]<0.0f)sp_Tvol[trackef]=0.0f;
			break;
		
		}
}//FOR BUCLE ON TRACKS

}// Function of pattern effects

#ifndef AUDACIOUS_UADE
void Actualize_Main_Ed(void)
{
if (userscreen==0)
{
	char tname[32];

	if (snamesel==1)
	{
		sprintf(tname,"%s_",name);
		guiDial(90,386,162,16,tname,100);
	}
	else
	{
		sprintf(tname,"%s",name);
		guiDial(90,386,162,16,tname,140);
	}

	if (snamesel==4)
	{
		sprintf(tname,"%s_",artist);
		guiDial(90,404,162,16,tname,100);
	}
	else
	{
		sprintf(tname,"%s",artist);
		guiDial(90,404,162,16,tname,140);
	}
	
	if (snamesel==5)
	{
		sprintf(tname,"%s_",style);
		guiDial(90,422,162,16,tname,100);
	}
	else
	{
		sprintf(tname,"%s",style);
		guiDial(90,422,162,16,tname,140);
	}

	valuer_box(482,350,XLATENCY_TIME);
	outlong(544,350,XLATENCY_TIME,2);

GlobalMemoryStatus(&memstate);
char mdis[80];
sprintf(mdis,"Memory Status. (Memory load: %d%%)",memstate.dwMemoryLoad,200);
guiDial3(344,386,246,16,mdis,150);
guiDial3(344,404,60,18,"Total Phys.",200);
outlong(344,424,memstate.dwTotalPhys/1024,5); 
guiDial3(406,404,60,18,"Avail Phys.",200);
outlong(406,424,memstate.dwAvailPhys/1024,5);
guiDial3(468,404,60,18,"Total Vrtl.",200);
outlong(468,424,memstate.dwTotalVirtual/1024,5); 
guiDial3(530,404,60,18,"Avail Vrtl.",200);
outlong(530,424,memstate.dwAvailVirtual/1024,5);

if(QUALITYCHANGE){
if(QUALITYPLAY==0)mess_box("NTK Level 2 Audio Engine [All Features Enabled, Highest CPU usage]. CSynth Enabled, all DSP units enabled.");
if(QUALITYPLAY==1)mess_box("NTK Level 1 Audio Engine [CSynth: Disabled, Flanger Engine: Off, Medium CPU usage]");
if(QUALITYPLAY==2)mess_box("NTK Level 0 Audio Engine [Standard 16 Channel Mod, No Effects, No CSynth, No Filters, Lowest CPU usage]");
QUALITYCHANGE=false;
}

switch(QUALITYPLAY)
{
case 0:
guiDial(264,368,64,16,"Highest",100);
guiDial(264,386,64,16,"Medium",200);
guiDial(264,404,64,16,"Lowest",200);
break;
case 1:
guiDial(264,368,64,16,"Highest",200);
guiDial(264,386,64,16,"Medium",100);
guiDial(264,404,64,16,"Lowest",200);
break;
case 2:
guiDial(264,368,64,16,"Highest",200);
guiDial(264,386,64,16,"Medium",200);
guiDial(264,404,64,16,"Lowest",100);
break;
}

}

}

void draw_mained(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",100);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

guiDial2("Disk Operations / Playback Latency / Song Credits");
guiDial(8,350,80,16,"Load Song",200);
guiDial(8,368,80,16,"Save Song",200);
guiDial3(8,386,80,16,"Title",190);
guiDial3(8,404,80,16,"Produced By",190);
guiDial3(8,422,80,16,"Style",190);
guiDial(90,350,80,16,"New Song",200);
guiDial(90,368,80,16,"WAV Render",200);
guiDial(172,350,80,16,"Save Inst",200);
guiDial(172,368,80,16,"Show Info",200);
guiDial3(344,350,136,16,"SoundCard Latency",200);

guiDial3(264,350,64,16,"CPU Usage",200);

S->printXY(344,368,0x00888888,"The lower latency the higher CPU and clicking probability.");
}

void SaveMod(void)
{
FILE *in;
char Temph[96];
char extension[10];
sprintf(extension,"TWNNSNG2");

sprintf (Temph,"Saving '%s.ntk' song on current directory...",name);
mess_box(Temph);	
sprintf(Temph,"%s.ntk",name);
in = fopen(Temph,"wb");
if (in!=NULL)
{
// Writing NoiseTrekker header & name...
fwrite(extension, sizeof( char),9,in);
fwrite(name, sizeof( char ), 20,in);
fwrite(&nPatterns, sizeof(unsigned char ), 1,in);
fwrite(&sLength, sizeof(unsigned char ), 1,in);
fwrite(pSequence, sizeof(unsigned char ), 256,in);
fwrite(patternLines, sizeof(short ),128,in);

for (int pwrite=0;pwrite<nPatterns;pwrite++)
fwrite(RawPatterns+pwrite*12288, 12288,1,in);

// Writing sample data
for (int swrite=0;swrite<128;swrite++)
{
fwrite(&nameins[swrite], sizeof( char ), 20,in);
fwrite(&Midiprg[swrite],sizeof(char),1,in);
fwrite(&Synthprg[swrite],sizeof(bool),1,in);
fwrite(&PARASynth[swrite], sizeof(SynthParameters),1,in);

for (int slwrite=0;slwrite<16;slwrite++)
{
fwrite(&SampleType[swrite][slwrite], sizeof(char ),1,in);
if (SampleType[swrite][slwrite]!=0)
{
fwrite(&SampleName[swrite][slwrite], sizeof(char),256,in);
fwrite(&Basenote[swrite][slwrite],sizeof(char),1,in);
fwrite(&LoopStart[swrite][slwrite], sizeof(long ),1,in);
fwrite(&LoopEnd[swrite][slwrite], sizeof(long ),1,in);
fwrite(&LoopType[swrite][slwrite], sizeof(char ),1,in);
fwrite(&SampleNumSamples[swrite][slwrite], sizeof(long ),1,in);
fwrite(&Finetune[swrite][slwrite], sizeof(char ),1,in);
fwrite(&SampleVol[swrite][slwrite], sizeof(float ),1,in);
fwrite(&FDecay[swrite][slwrite], sizeof(float ),1,in);

fwrite(RawSamples[swrite][0][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
fwrite(&SampleChannels[swrite][slwrite], sizeof(char ),1,in);
if (SampleChannels[swrite][slwrite]==2)
fwrite(RawSamples[swrite][1][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
}// Exist Sample
}
}

// Writing Track Propertiers
for (int twrite=0;twrite<MAX_TRACKS;twrite++)
{
fwrite(&TCut[twrite], sizeof(float ),1,in);
fwrite(&ICut[twrite], sizeof(float ),1,in);
fwrite(&TPan[twrite], sizeof(float ),1,in);
fwrite(&FType[twrite], sizeof(int ),1,in);
fwrite(&FRez[twrite], sizeof(int ),1,in);
fwrite(&DThreshold[twrite], sizeof(float ),1,in);
fwrite(&DClamp[twrite], sizeof(float ),1,in);
fwrite(&DSend[twrite], sizeof(float ),1,in);
fwrite(&CSend[twrite], sizeof(int ),1,in);
}

// Writing mod properties
fwrite(&compressor, sizeof(int),1,in);
fwrite(&c_threshold, sizeof(int),1,in);
fwrite(&BeatsPerMin, sizeof(int ),1,in);
fwrite(&TicksPerBeat, sizeof(int ),1,in);
fwrite(&mas_vol, sizeof(float),1,in);
fwrite(&delay_time, sizeof(int ),1,in);
fwrite(&Feedback, sizeof(float ),1,in);
fwrite(&DelayType, sizeof(int ),1,in);
fwrite(&lchorus_delay, sizeof(int ),1,in);
fwrite(&rchorus_delay, sizeof(int ),1,in);
fwrite(&lchorus_feedback, sizeof(float ),1,in);
fwrite(&rchorus_feedback, sizeof(float ),1,in);
fwrite(&shuffle, sizeof(int),1,in);

// Writing part sequence data

for (int tps_pos=0;tps_pos<256;tps_pos++)
for (int tps_trk=0;tps_trk<16;tps_trk++)
fwrite(&SACTIVE[tps_pos][tps_trk],sizeof(bool),1,in);

for (twrite=0;twrite<MAX_TRACKS;twrite++)
fwrite(&TRACKMIDICHANNEL[twrite], sizeof(int),1,in);

for (twrite=0;twrite<MAX_TRACKS;twrite++)
{
fwrite(&LFO_ON[twrite], sizeof(char),1,in);
fwrite(&LFORATE[twrite], sizeof(float),1,in);
fwrite(&LFOAMPL[twrite], sizeof(float),1,in);
}

for (twrite=0;twrite<MAX_TRACKS;twrite++){
fwrite(&FLANGER_ON[twrite], sizeof(char),1,in);
fwrite(&FLANGER_AMOUNT[twrite], sizeof(float),1,in);
fwrite(&FLANGER_DEPHASE[twrite], sizeof(float),1,in);
fwrite(&FLANGER_RATE[twrite], sizeof(float),1,in);
fwrite(&FLANGER_AMPL[twrite], sizeof(float),1,in);
fwrite(&FLANGER_FEEDBACK[twrite], sizeof(float),1,in);
fwrite(&FLANGER_DELAY[twrite], sizeof(int),1,in);
}
fwrite(&FLANGER_DEPHASE, sizeof(float),1,in);

for (char tps_trk=0;tps_trk<16;tps_trk++)
fwrite(&TRACKSTATE[tps_trk],sizeof(int),1,in);

fwrite(&Songtracks,sizeof(char),1,in);

for (tps_trk=0;tps_trk<16;tps_trk++)
{
fwrite(&Disclap[tps_trk],sizeof(bool),1,in);
fwrite(&Dispan[tps_trk],sizeof(bool),1,in);
}

fwrite(artist, sizeof( char ), 20,in);
fwrite(style, sizeof( char ), 20,in);
fwrite(&QUALITYPLAY, sizeof( char ), 1,in);

fwrite(beatsync,sizeof(bool),128,in);
fwrite(beatlines,sizeof(short),128,in);
fwrite(&REVERBFILTER,sizeof(float),1,in);
fwrite(CustomVol,sizeof(float),128,in);
fwrite(&AMIMODE,sizeof(bool),1,in);
fwrite(&tb303,sizeof(para303),2,in);
fwrite(&tb303engine[0].tbVolume,sizeof(float),1,in);
fwrite(&tb303engine[1].tbVolume,sizeof(float),1,in);
fwrite(&tb303engine[0].hpf,sizeof(bool),1,in);
fwrite(&tb303engine[1].hpf,sizeof(bool),1,in);

fclose(in);
last_index=-1;
Read_SMPT();
ltActualize(0);

mess_box("Module saved succesful...");
}
else
{
mess_box("Module save failed...");	
}
if (snamesel==1 || snamesel==4 || snamesel==5){snamesel=0;Actualize_Main_Ed();}
}
#endif // AUDACIOUS_UADE
#ifdef AUDACIOUS_UADE
bool LoadMod(FILE *in)
#else
void LoadMod(void)
#endif
{
SongStop();
mess_box("Attempting to load the song file...");
#ifndef AUDACIOUS_UADE
Sleep(1000);
FILE *in;
in = fopen(name,"rb");
#endif

if (in!=NULL)
{
// Reading and checking extension...
char extension[10];
fread(extension, sizeof( char),9,in);
if (strcmp(extension,"TWNNSNG2")==0)
{ /* Ok, extension match! */
mess_box("Loading 'NoiseTrekker' song -> Header");	

FreeAll();

for (int inicial=0;inicial<PBLEN;inicial+=6)
{
	*(RawPatterns+inicial)=121;//121
	*(RawPatterns+inicial+1)=255;//255
	*(RawPatterns+inicial+2)=255;//255
	*(RawPatterns+inicial+3)=255;//255
	*(RawPatterns+inicial+4)=0;//0
	*(RawPatterns+inicial+5)=0;//0
}

init_sample_bank();
Iniplayer();

fread(name, sizeof( char ), 20,in);
fread(&nPatterns, sizeof(unsigned char ), 1,in);
fread(&sLength, sizeof(unsigned char ), 1,in);
fread(pSequence, sizeof(unsigned char ), 256,in);
fread_swap(patternLines, sizeof(short),128,in);

for (int pwrite=0;pwrite<nPatterns;pwrite++)
fread(RawPatterns+pwrite*12288, 12288,1,in);

mess_box("Loading 'NoiseTrekker' song -> Sample data");	

for (int swrite=0;swrite<128;swrite++)
{
fread(&nameins[swrite], sizeof( char ), 20,in);
fread(&Midiprg[swrite],sizeof(char),1,in);
fread(&Synthprg[swrite],sizeof(bool),1,in);
fread(&PARASynth[swrite], sizeof(SynthParameters),1,in);
#ifdef WORDS_BIGENDIAN // mvtiaine: big endian support
PARASynth[swrite].osc1_pw = SWAP32(PARASynth[swrite].osc1_pw);
PARASynth[swrite].osc2_pw = SWAP32(PARASynth[swrite].osc2_pw);
PARASynth[swrite].env1_attack = SWAP32(PARASynth[swrite].env1_attack);
PARASynth[swrite].env1_decay = SWAP32(PARASynth[swrite].env1_decay);
PARASynth[swrite].env1_release = SWAP32(PARASynth[swrite].env1_release);
PARASynth[swrite].env2_attack = SWAP32(PARASynth[swrite].env2_attack);
PARASynth[swrite].env2_decay = SWAP32(PARASynth[swrite].env2_decay);
PARASynth[swrite].env2_release = SWAP32(PARASynth[swrite].env2_release);
PARASynth[swrite].lfo1_period = SWAP32(PARASynth[swrite].lfo1_period);
PARASynth[swrite].lfo2_period = SWAP32(PARASynth[swrite].lfo2_period);
#endif
for (int slwrite=0;slwrite<16;slwrite++)
{
fread(&SampleType[swrite][slwrite], sizeof(char ),1,in);
if (SampleType[swrite][slwrite]!=0)
{
fread(&SampleName[swrite][slwrite], sizeof(char),256,in);
fread(&Basenote[swrite][slwrite],sizeof(char),1,in);
fread_swap(&LoopStart[swrite][slwrite], sizeof(int ),1,in);
fread_swap(&LoopEnd[swrite][slwrite], sizeof(int ),1,in);
fread(&LoopType[swrite][slwrite], sizeof(char ),1,in);
fread_swap(&SampleNumSamples[swrite][slwrite], sizeof(int ),1,in);
fread(&Finetune[swrite][slwrite], sizeof(char ),1,in);
fread_swap(&SampleVol[swrite][slwrite], sizeof(float ),1,in);
fread_swap(&FDecay[swrite][slwrite], sizeof(float ),1,in);
RawSamples[swrite][0][slwrite]=(short *)malloc(SampleNumSamples[swrite][slwrite]*2 + 6); // mvtiaine: fixed buffer overflow
fread_swap(RawSamples[swrite][0][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
fread(&SampleChannels[swrite][slwrite], sizeof(char ),1,in);
if (SampleChannels[swrite][slwrite]==2)
{
RawSamples[swrite][1][slwrite]=(short *)malloc(SampleNumSamples[swrite][slwrite]*2 + 6); // mvtiaine: fixed buffer overflow
fread_swap(RawSamples[swrite][1][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
}
}// Exist Sample
}
}

mess_box("Loading 'NoiseTrekker' song -> Track info, patterns and sequence.");	

// Reading Track Propertiers
for (int twrite=0;twrite<MAX_TRACKS;twrite++)
{
fread_swap(&TCut[twrite], sizeof(float ),1,in);
fread_swap(&ICut[twrite], sizeof(float ),1,in);
if (ICut[ped_track]>0.0078125f)ICut[ped_track]=0.0078125f;
if (ICut[ped_track]<0.00006103515625f)ICut[ped_track]=0.00006103515625f;

fread_swap(&TPan[twrite], sizeof(float ),1,in);
ComputeStereo(twrite);
fread_swap(&FType[twrite], sizeof(int ),1,in);
fread_swap(&FRez[twrite], sizeof(int ),1,in);
fread_swap(&DThreshold[twrite], sizeof(float ),1,in);
fread_swap(&DClamp[twrite], sizeof(float ),1,in);
fread_swap(&DSend[twrite], sizeof(float ),1,in);
fread_swap(&CSend[twrite], sizeof(int ),1,in);
}

// Reading mod properties
fread_swap(&compressor, sizeof(int ),1,in);
fread_swap(&c_threshold, sizeof(int ),1,in);
fread_swap(&BeatsPerMin, sizeof(int ),1,in);
fread_swap(&TicksPerBeat, sizeof(int ),1,in);
fread_swap(&mas_vol, sizeof(float ),1,in);
if (mas_vol<0.01f)mas_vol=0.01f;
if (mas_vol>1.0f)mas_vol=1.0f;
fread_swap(&delay_time, sizeof(int ),1,in);
fread_swap(&Feedback, sizeof(float ),1,in);
fread_swap(&DelayType, sizeof(int ),1,in);
fread_swap(&lchorus_delay, sizeof(int),1,in);
fread_swap(&rchorus_delay, sizeof(int),1,in);
fread_swap(&lchorus_feedback, sizeof(float),1,in);
fread_swap(&rchorus_feedback, sizeof(float ),1,in);
fread_swap(&shuffle, sizeof(int),1,in);

// Reading track part sequence
for (int tps_pos=0;tps_pos<256;tps_pos++){for (int tps_trk=0;tps_trk<16;tps_trk++){fread(&SACTIVE[tps_pos][tps_trk],sizeof(bool),1,in);}}

for (int spl=0;spl<MAX_TRACKS;spl++)
	CCoef[spl]=float((float)CSend[spl]/127.0);

for (int twrite=0;twrite<MAX_TRACKS;twrite++)
fread_swap(&TRACKMIDICHANNEL[twrite], sizeof(int),1,in);

for (int twrite=0;twrite<MAX_TRACKS;twrite++)
{
fread(&LFO_ON[twrite], sizeof(char),1,in);
fread_swap(&LFORATE[twrite], sizeof(float),1,in);
fread_swap(&LFOAMPL[twrite], sizeof(float),1,in);
}

for (int twrite=0;twrite<MAX_TRACKS;twrite++){
fread(&FLANGER_ON[twrite], sizeof(char),1,in);
fread_swap(&FLANGER_AMOUNT[twrite], sizeof(float),1,in);
fread_swap(&FLANGER_DEPHASE[twrite], sizeof(float),1,in);
fread_swap(&FLANGER_RATE[twrite], sizeof(float),1,in);
fread_swap(&FLANGER_AMPL[twrite], sizeof(float),1,in);
fread_swap(&FLANGER_FEEDBACK[twrite], sizeof(float),1,in);
fread_swap(&FLANGER_DELAY[twrite], sizeof(int),1,in);
FLANGER_OFFSET[twrite]=8192;
foff2[twrite]=float(FLANGER_OFFSET[twrite]-FLANGER_DELAY[twrite]);
foff1[twrite]=float(FLANGER_OFFSET[twrite]-FLANGER_DELAY[twrite]);
}
fread_swap(&FLANGER_DEPHASE, sizeof(float),1,in);

for (char tps_trk=0;tps_trk<16;tps_trk++)
fread_swap(&TRACKSTATE[tps_trk],sizeof(int),1,in);

fread(&Songtracks,sizeof(char),1,in);

for (char tps_trk=0;tps_trk<16;tps_trk++)
{
fread(&Disclap[tps_trk],sizeof(bool),1,in);
fread(&Dispan[tps_trk],sizeof(bool),1,in);
}

fread(artist, sizeof( char ), 20,in);
fread(style, sizeof( char ), 20,in);
fread(&QUALITYPLAY, sizeof( char ), 1,in);
fread(beatsync,sizeof(bool),128,in);
fread_swap(beatlines,sizeof(short),128,in);
fread_swap(&REVERBFILTER,sizeof(float),1,in);

fread_swap(CustomVol,sizeof(float),128,in);
fread(&AMIMODE,sizeof(bool),1,in);
fread(&tb303,sizeof(para303),2,in);
fread_swap(&tb303engine[0].tbVolume,sizeof(float),1,in);
fread_swap(&tb303engine[1].tbVolume,sizeof(float),1,in);
fread(&tb303engine[0].hpf,sizeof(bool),1,in);
fread(&tb303engine[1].hpf,sizeof(bool),1,in);
fclose(in);

ped_track=0;
ped_patsam=1;
ped_row=0;
ped_line=0;
cPosition=0;
gui_track=0;
lchorus_counter=44100;
rchorus_counter=44100;
lchorus_counter2=44100-lchorus_delay;
rchorus_counter2=44100-rchorus_delay;
Initreverb();

if(userscreen!=8)
{userscreen=0;draw_mained();Actualize_Main_Ed();}
if(QUALITYPLAY==0)mess_box("NTK Level 2 Song File loaded [All Features Enabled, Hi CPU usage]");
if(QUALITYPLAY==1)mess_box("NTK Level 1 Song File loaded [CSynth: Off, Flanger Engine: Off, Medium CPU usage]");
if(QUALITYPLAY==2)mess_box("NTK Level 0 Song File loaded [Standard 16 Channel Mod, No Effects, Low CPU usage]");

Actualize_Sequencer();
Actualize_Patterned();
Actualize_Master(0);
Actualize_Master(4);
}
else
{
mess_box("That file is not a NoiseTrekker song-file...");
return false;
}
}
else
{
mess_box("Module loading failed. (Probably: file not found)");
return false;
}

if (snamesel==1 || snamesel==4 || snamesel==5)
{
	snamesel=0;
	if(userscreen!=8)Actualize_Main_Ed();
}
return true;
}

float filter2p(char ch,float input,float f,float q)
{
  f*=0.0078125f;
  q*=0.0078125f;

  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  buf0[ch] = fa * buf0[ch] + f * (input + fb * (buf0[ch] - buf1[ch])); 
  buf1[ch] = fa * buf1[ch] + f * buf0[ch];
  return buf1[ch];  
}


float filterhp2(char ch,float input,float f,float q)
{  
  f*=0.0078125f;
  q*=0.0078125f;

  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  buf024[ch] = fa * buf024[ch] + f * (input + fb * (buf024[ch] - buf124[ch])); 
  buf124[ch] = fa * buf124[ch] + f * buf024[ch];
  return input-buf124[ch];  
}

float filterhp(char ch,float input,float f,float q)
{
  f*=0.0078125f;
  q*=0.0078125f;
  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  buf0[ch] = fa * buf0[ch] + f * (input + fb * (buf0[ch] - buf1[ch])); 
  buf1[ch] = fa * buf1[ch] + f * buf0[ch];
  return input-buf1[ch];  
}

float filter2p24d(char ch,float input,float f,float q)
{
  f*=0.0078125f;
  q*=0.0078125f;
  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  buf024[ch] = fa * buf024[ch] + f * (input + fb * (buf024[ch] - buf124[ch])); 
  buf124[ch] = fa * buf124[ch] + f * buf024[ch];
  return buf124[ch];  
}

#ifndef AUDACIOUS_UADE
void Actualize_Songname(int newletter, char *nam)
{
if (newletter==39 && namesize>0){snamesel=0;return;}

if (namesize>18 && newletter!=37)return;
if (namesize==0 && newletter==37)return;

if (newletter!=37)namesize++;

switch (newletter)
{
case 1: sprintf(nam,"%sa",nam);break;
case 2: sprintf(nam,"%sb",nam);break;
case 3: sprintf(nam,"%sc",nam);break;
case 4: sprintf(nam,"%sd",nam);break;
case 5: sprintf(nam,"%se",nam);break;
case 6: sprintf(nam,"%sf",nam);break;
case 7: sprintf(nam,"%sg",nam);break;
case 8: sprintf(nam,"%sh",nam);break;
case 9: sprintf(nam,"%si",nam);break;
case 10: sprintf(nam,"%sj",nam);break;
case 11: sprintf(nam,"%sk",nam);break;
case 12: sprintf(nam,"%sl",nam);break;
case 13: sprintf(nam,"%sm",nam);break;
case 14: sprintf(nam,"%sn",nam);break;
case 15: sprintf(nam,"%so",nam);break;
case 16: sprintf(nam,"%sp",nam);break;
case 17: sprintf(nam,"%sq",nam);break;
case 18: sprintf(nam,"%sr",nam);break;
case 19: sprintf(nam,"%ss",nam);break;
case 20: sprintf(nam,"%st",nam);break;
case 21: sprintf(nam,"%su",nam);break;
case 22: sprintf(nam,"%sv",nam);break;
case 23: sprintf(nam,"%sw",nam);break;
case 24: sprintf(nam,"%sx",nam);break;
case 25: sprintf(nam,"%sy",nam);break;
case 26: sprintf(nam,"%sz",nam);break;
case 27: sprintf(nam,"%s0",nam);break;
case 28: sprintf(nam,"%s1",nam);break;
case 29: sprintf(nam,"%s2",nam);break;
case 30: sprintf(nam,"%s3",nam);break;
case 31: sprintf(nam,"%s4",nam);break;
case 32: sprintf(nam,"%s5",nam);break;
case 33: sprintf(nam,"%s6",nam);break;
case 34: sprintf(nam,"%s7",nam);break;
case 35: sprintf(nam,"%s8",nam);break;
case 36: sprintf(nam,"%s9",nam);break;
case 37: nam[strlen(nam)-1]='\0';namesize--;break;
case 38: sprintf(nam,"%s ",nam);break;
case 40: sprintf(nam,"%s.",nam);break;
case 41: sprintf(nam,"%sA",nam);break;
case 42: sprintf(nam,"%sB",nam);break;
case 43: sprintf(nam,"%sC",nam);break;
case 44: sprintf(nam,"%sD",nam);break;
case 45: sprintf(nam,"%sE",nam);break;
case 46: sprintf(nam,"%sF",nam);break;
case 47: sprintf(nam,"%sG",nam);break;
case 48: sprintf(nam,"%sH",nam);break;
case 49: sprintf(nam,"%sI",nam);break;
case 50: sprintf(nam,"%sJ",nam);break;
case 51: sprintf(nam,"%sK",nam);break;
case 52: sprintf(nam,"%sL",nam);break;
case 53: sprintf(nam,"%sM",nam);break;
case 54: sprintf(nam,"%sN",nam);break;
case 55: sprintf(nam,"%sO",nam);break;
case 56: sprintf(nam,"%sP",nam);break;
case 57: sprintf(nam,"%sQ",nam);break;
case 58: sprintf(nam,"%sR",nam);break;
case 59: sprintf(nam,"%sS",nam);break;
case 60: sprintf(nam,"%sT",nam);break;
case 61: sprintf(nam,"%sU",nam);break;
case 62: sprintf(nam,"%sV",nam);break;
case 63: sprintf(nam,"%sW",nam);break;
case 64: sprintf(nam,"%sX",nam);break;
case 65: sprintf(nam,"%sY",nam);break;
case 66: sprintf(nam,"%sZ",nam);break;
case 67: sprintf(nam,"%s,",nam);break;
case 68: sprintf(nam,"%s-",nam);break;
case 69: sprintf(nam,"%s'",nam);break;
case 70: sprintf(nam,"%s¡",nam);break;
}
}

void Newmod(void)
{
guiDial(90,350,80,16,"New Song",100);
SongStop();
mess_box("Freeing all allocated buffers and restarting...");	
Sleep(1000);

FreeAll();
init_sample_bank();

for (int api=0;api<128;api++)
patternLines[api]=64;

for (int inicial=0;inicial<PBLEN;inicial+=6)
{
*(RawPatterns+inicial)=121;//121
*(RawPatterns+inicial+1)=255;//255
*(RawPatterns+inicial+2)=255;//255
*(RawPatterns+inicial+3)=255;//255
*(RawPatterns+inicial+4)=0;//0
*(RawPatterns+inicial+5)=0;//0
}

nPatterns=1;
Iniplayer();
sprintf(name,"Untitled");
sprintf(artist,"Somebody");
sprintf(style,"Goa Trance");
namesize=8;
sLength=1;
BeatsPerMin=125;
TicksPerBeat=4;
SamplesPerSec=44100;
SubCounter=0;
PosInTick=0;
DelayType=1;
player_line=0;
mas_vol=1.0f;
ped_track=0;
ped_patsam=1;
ped_row=0;
ped_line=0;
cPosition=0;

lchorus_feedback=0.6f;
rchorus_feedback=0.5f;
lchorus_delay=10584;
rchorus_delay=15876;

lchorus_counter=44100;
rchorus_counter=44100;
lchorus_counter2=44100-lchorus_delay;
rchorus_counter2=44100-rchorus_delay;
compressor=0;
c_threshold=32;
DelayType=1;
delay_time=0;
seditor=0;

for (int spl=0;spl<MAX_TRACKS;spl++)
CCoef[spl]=float((float)CSend[spl]/127.0);

Actualize_Sequencer();
Actualize_Patterned();
Actualize_Master(0);
if (snamesel==1 || snamesel==4 || snamesel==5)snamesel=0;
Actualize_Main_Ed();

mess_box("New song started...");
Sleep(300);
guiDial(90,350,80,16,"New Song",200);
NewWavy();
S->copy(FRAMEWORK, 0, 184);
Actupated(0);
}

void draw_seqed(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",100);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

guiDial2("Track Part Sequencer / Stereo Scope");
guiDial3(4,348,134,86,"Left Scope",100);
guiDial3(496,348,134,86,"Right Scope",100);
guiDial(144,348,80,16,"Clear All",200);
guiDial(144,366,80,16,"Clear Position",200);
guiDial(144,384,80,16,"Reset All",200);
guiDial(144,402,80,16,"Reset Position",200);
guiDial(410,348,80,16,"Ptn->Pos[Cur]",200);
guiDial(410,366,80,16,"Ptn->Pos[Sng]",200);

guiDial3(144,420,32,16,"Scop.",200);
guiDial(230,343,14,88,"",200);
guiDial(390,343,14,88,"",200);
guiDial(254,343,128,88,"",200);
S->setColor(0,0,0);
bjbox(257,346,124,86);
S->setColor(0,80,0);
bjbox(257,382,124,12);
Actualize_Scopish();
}

void Actualize_Scopish(void)
{
if (Scopish)
{
guiDial(178,420,22,16,"On",100);
guiDial(202,420,22,16,"Off",200);
}
else
{
guiDial(178,420,22,16,"On",200);
guiDial(202,420,22,16,"Off",100);
}
}

void Actualize_Seq_Ed(void)
{
mess_box("Ready...");
HDC dc=(HDC)S->getDeviceContext();
SetBkMode(dc,OPAQUE);

for (int lseq=-3;lseq<4;lseq++)
	{
	                if (lseq==0)
						S->setBKColor(0,80,0);
					else
						S->setBKColor(0,10,0);

	  int rel=lseq+cPosition;
	  if (rel>-1 && rel<256)
			{
				out_decchar(232,382+lseq*12,rel,0);
				out_decchar(392,382+lseq*12,pSequence[rel],0);

				for (int rel2=0;rel2<Songtracks;rel2++)
				{
					
					if (SACTIVE[rel][rel2])
					out_nibble(256+rel2*8,382+lseq*12,0x20F040,rel2);
					else
					out_nibble(256+rel2*8,382+lseq*12,0x205030,rel2);
				
				}//sub for
			}// rel range OK
			else
			{
				S->printXY(256,382+lseq*12,0x000000,"000000000000000000000");
				S->printXY(232,382+lseq*12,0x000000,"00");
				S->printXY(392,382+lseq*12,0x000000,"00");
					
			}
	}// for end
SetBkMode(dc,TRANSPARENT);
Actupated(0);
}

void out_nibble(int x,int y,int color, int number)
{
switch(number)
{
case 0: S->printXY(x,y, color,"0");break;
case 1: S->printXY(x,y, color,"1");break;
case 2: S->printXY(x,y, color,"2");break;
case 3: S->printXY(x,y, color,"3");break;
case 4: S->printXY(x,y, color,"4");break;
case 5: S->printXY(x,y, color,"5");break;
case 6: S->printXY(x,y, color,"6");break;
case 7: S->printXY(x,y, color,"7");break;
case 8: S->printXY(x,y, color,"8");break;
case 9: S->printXY(x,y, color,"9");break;
case 10: S->printXY(x,y, color,"A");break;
case 11: S->printXY(x,y, color,"B");break;
case 12: S->printXY(x,y, color,"C");break;
case 13: S->printXY(x,y, color,"D");break;
case 14: S->printXY(x,y, color,"E");break;
case 15: S->printXY(x,y, color,"F");break;
}
}

void Resetdevice()
{
if (ss!=NULL)
 {
    if (ss->isPlaying())
    {
    ss->stop();
	ss->setStreamDelay(XLATENCY_TIME);
	ss->play();    
	}
  }
}

void draw_mastered(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",100);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

guiDial2("Master configuration / Info");

guiDial3(8,420,42,16,"Shuffle",200);
guiDial3(190,366,60,16,"Master Vol",200);
guiDial3(190,384,60,16,"09xx Mode",200);

Coolwrite(216,334,0x00FFFFFF,"Beats Per Min:");
Coolwrite(400,334,0x00FFFFFF,"1/4 Beat Time:");
Coolwrite(8,405,0x00FFFFFF,"Shuffle effect amount:");
}

void Actualize_Master_Ed(char gode)
{
if(userscreen==5)
{

if (gode==0 || gode==2)
{
if (mas_vol<0.01f)mas_vol=0.01f;
if (mas_vol>1.0f)mas_vol=1.0f;
Realslider(252,366,f2i(mas_vol*128));
outlong(401,366,f2i(mas_vol*100),1);
}

if (gode==0 || gode==3)
{
outlong(292,332,BeatsPerMin,0);
outlong(476,332,f2i(15000/(float)BeatsPerMin),2);
}

if (gode==0 || gode==4)
{
if (shuffle>100)shuffle=100;
if (shuffle<0)shuffle=0;
Realslider(52,420,shuffle);
outlong(201,420,shuffle,1);
}

if (gode==0 || gode==5)
{
if(AMIMODE)
{
guiDial(252,384,46,16,"Mod",100);
guiDial(300,384,46,16,"Ntk",200);
}else{
guiDial(252,384,46,16,"Mod",200);
guiDial(300,384,46,16,"Ntk",100);
}

}

}
}




void SaveSettings()
{
_chdir(appbuffer);

FILE *in;
in = fopen("ntk.cfg","wb");
if (in!=NULL)
{
// Writing noisetrekker configuration
fwrite(&XLATENCY_TIME, sizeof(int),1,in);
fwrite(&CONSOLE_WIDTH, sizeof(int),1,in);
fwrite(&CONSOLE_HEIGHT, sizeof(int),1,in);
fwrite(&GUIMODE, sizeof(char),1,in);
fwrite(&Scopish, sizeof(bool),1,in);
fclose(in);
}
}

void LoadSettings()
{
FILE *in;

  /* Get the current working directory: */
_getcwd(appbuffer, _MAX_PATH );

in = fopen("ntk.cfg","rb");
if (in!=NULL)
{
// Reading NoiseTrekker header & name...
fread(&XLATENCY_TIME, sizeof(int),1,in);
fseek(in,sizeof(int)*2,SEEK_CUR); // Ignore videomode;
fread(&GUIMODE, sizeof(char),1,in);
fread(&Scopish, sizeof(bool),1,in);

fclose(in);
}
}

void DrawScope(void)
{
int xl=8;
int xr=500;

for (int s=0;s<128;s++)
{
int yl=400-Lscope[s]/1024;
int yr=400-Rscope[s]/1024;

S->drawLine(xl,yl,xl,yl,0xFFFF);
S->drawLine(xr,yr,xr,yr,0xFFFF);

S->drawLine(xl,368,xl,yl-1,0);
S->drawLine(xr,368,xr,yr-1,0);

S->drawLine(xl,yl+1,xl,432,yl);
S->drawLine(xr,yr+1,xr,432,yr);
xl++;
xr++;
}

}

void MidiGetAll(void)
{
n_midiindevices=midiInGetNumDevs();
n_midioutdevices=midiOutGetNumDevs();

for (int m=0;m<n_midiindevices;m++)
	midiInGetDevCaps(m,&caps_midiin[m],sizeof(MIDIINCAPS));

for (m=0;m<n_midioutdevices;m++)
	midiOutGetDevCaps(m,&caps_midiout[m],sizeof(MIDIOUTCAPS));
}

void draw_midied(void)
{
guiDial(112,309,64,16,"Track FX",200);
guiDial(178,309,64,16,"Midi/CSynth",100);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

char middev[80];

sprintf(middev,"Midi Setup. Found: %d Midi-In devices and %d Midi-Out devices.",n_midiindevices,n_midioutdevices);
guiDial2("");
guiDial3(4,336,332,108,middev,128);

guiDial3(8,352,56,16,"Midi IN",200);
guiDial3(8,370,56,16,"Midi OUT",200);
guiDial(8,388,124,16,"All Notes Off (Track)",200);
guiDial(134,388,124,16,"All Notes Off (Song)",200);
guiDial3(340,336,296,108,"Instrument [Synthesizer Editor - CSynth]",128);
guiDial3(348,352,56,16,"Program",200);
guiDial3(348,370,56,16,"Parameter",200);
guiDial3(348,388,38,16,"Value",200);
guiDial3(348,406,38,16,"OSC1",200);
guiDial3(348,424,38,16,"OSC2",200);
guiDial3(518,406,52,16,"Sub OSC",200);
guiDial3(518,424,52,16,"VCF Type",200);
guiDial(598,352,34,16,"Save",200);
guiDial(598,370,34,16,"Rand",200);

guiDial(388,388,16,16,"<",220);
guiDial(554,388,16,16,">",220);

S->printXY(7,424,0x00000000,"Go to 'Track' section to assign Midi Channels to NoiseTrekker tracks");
S->printXY(8,423,0x00FFFFFF,"Go to 'Track' section to assign Midi Channels to NoiseTrekker tracks");
}

void Actualize_Midi_Ed(char gode)
{

if (gode==0 || gode==1)
{
valuer_box4(406,352,ped_patsam);
Actualize_SynthParSlider();
}

if (gode==6)
{
CParcha(csynth_slv);
Actualize_SynthParSlider();
}

if (gode==0 || gode==10 || gode==1)
{
char tcp[30];
sprintf(tcp,"%s_",PARASynth[ped_patsam].presetname);

if (snamesel==3)
guiDial(432,352,164,16,tcp,100);
else
guiDial(432,352,164,16,PARASynth[ped_patsam].presetname,140);
}

if (gode==0 || gode==2)
{
if (ped_synthpar<1)ped_synthpar=1;
if (ped_synthpar>51)ped_synthpar=51;
valuer_box2(406,370,ped_synthpar);
guiDial3(468,370,128,16,CS_PAR_NAME[ped_synthpar],140);
Actualize_SynthParSlider();
}

if (gode==0 || gode==3 || gode==1)
{
guiDial(388,406,24,16,"Sin",140);
guiDial(414,406,24,16,"Saw",140);
guiDial(440,406,24,16,"Pul",140);
guiDial(466,406,24,16,"Rnd",140);
guiDial(492,406,24,16,"Off",140);
switch (PARASynth[ped_patsam].osc1_waveform)
{
case 0:guiDial(388,406,24,16,"Sin",100);break;
case 1:guiDial(414,406,24,16,"Saw",100);break;
case 2:guiDial(440,406,24,16,"Pul",100);break;
case 3:guiDial(466,406,24,16,"Rnd",100);break;
case 4:guiDial(492,406,24,16,"Off",100);break;
}
}

if (gode==0 || gode==4 || gode==1)
{
guiDial(388,424,24,16,"Sin",140);
guiDial(414,424,24,16,"Saw",140);
guiDial(440,424,24,16,"Pul",140);
guiDial(466,424,24,16,"Rnd",140);
guiDial(492,424,24,16,"Off",140);
switch (PARASynth[ped_patsam].osc2_waveform)
{
case 0:guiDial(388,424,24,16,"Sin",100);break;
case 1:guiDial(414,424,24,16,"Saw",100);break;
case 2:guiDial(440,424,24,16,"Pul",100);break;
case 3:guiDial(466,424,24,16,"Rnd",100);break;
case 4:guiDial(492,424,24,16,"Off",100);break;
}
}

if (gode==0 || gode==5 || gode==1)
{
	if (PARASynth[ped_patsam].osc3_switch)
	{
	guiDial(572,406,24,16,"On",100);
	guiDial(596,406,24,16,"Off",200);
	}else{
	guiDial(572,406,24,16,"On",200);
	guiDial(596,406,24,16,"Off",100);
	}
}

if (gode==0 || gode==7 || gode==1)
{
	valuer_box(572,424,PARASynth[ped_patsam].vcf_type);
}

if (midiin_changed==true)
{
if (c_midiin<-1)c_midiin=n_midiindevices-1;
if (c_midiin==n_midiindevices)c_midiin=-1;

if (c_midiin!=-1){
if (midiin_handle!=NULL)midiInClose(midiin_handle);
if (midiInOpen(&midiin_handle,c_midiin, NULL, NULL,CALLBACK_NULL)==MMSYSERR_NOERROR)
mess_box("Midi In device actived...");
else
mess_box("Midi In device failed...");
}
else
mess_box("Midi In disconnected...");

midiin_changed=false;
}

if (midiout_changed==true)
{
MidiReset();
if (c_midiout<-1)c_midiout=n_midioutdevices-1;
if (c_midiout==n_midioutdevices)c_midiout=-1;

if (c_midiout!=-1){
if (midiout_handle!=NULL)midiOutClose(midiout_handle);
if (midiOutOpen(&midiout_handle,c_midiout, NULL, NULL,CALLBACK_NULL)==MMSYSERR_NOERROR)
mess_box("Midi Out device actived...");
else
mess_box("Midi Out device failed...");
}
else
mess_box("Midi Out disconnected...");

midiout_changed=false;
}

if (gode==0 || gode==8)
{
valuer_box(66,352,c_midiin+1);
if (c_midiin!=-1)
guiDial3(128,352,192,16,caps_midiin[c_midiin].szPname,100);
else
guiDial3(128,352,192,16,"None",100);
}

if (gode==0 || gode==9)
{
valuer_box(66,370,c_midiout+1);
if (c_midiout!=-1)
guiDial3(128,370,192,16,caps_midiout[c_midiout].szPname,100);
else
guiDial3(128,370,192,16,"None",100);
}

}
#endif // AUDACIOUS_UADE
void GetPlayerValues(float master_coef)
{
left_chorus=2.0f;
right_chorus=2.0f;
Sp_Player();
if (++lchorus_counter>88200)lchorus_counter=44100;
if (++rchorus_counter>88200)rchorus_counter=44100;
lbuff_chorus[lchorus_counter]=left_chorus+lbuff_chorus[lchorus_counter2]*lchorus_feedback;
rbuff_chorus[rchorus_counter]=right_chorus+rbuff_chorus[rchorus_counter2]*rchorus_feedback;
if (++lchorus_counter2>88200)lchorus_counter2=44100;
if (++rchorus_counter2>88200)rchorus_counter2=44100;
float rchore=lbuff_chorus[lchorus_counter2];
float lchore=rbuff_chorus[rchorus_counter2];
left_float=lchore+left_float;	
right_float=rchore+right_float;
Compressor_work();

left_value=f2i(left_float*master_coef);
right_value=f2i(right_float*master_coef);

if (left_value>32767)left_value=32767;
if (left_value<-32767)left_value=-32767;
if (right_value>32767)right_value=32767;
if (right_value<-32767)right_value=-32767;
}
#ifndef AUDACIOUS_UADE
void GetPlayerValues2(float master_coef)
{
left_chorus=2.0f;
right_chorus=2.0f;
Sp_Player2();
if (++lchorus_counter>88200)lchorus_counter=44100;
if (++rchorus_counter>88200)rchorus_counter=44100;
lbuff_chorus[lchorus_counter]=left_chorus+lbuff_chorus[lchorus_counter2]*lchorus_feedback;
rbuff_chorus[rchorus_counter]=right_chorus+rbuff_chorus[rchorus_counter2]*rchorus_feedback;
if (++lchorus_counter2>88200)lchorus_counter2=44100;
if (++rchorus_counter2>88200)rchorus_counter2=44100;
float rchore=lbuff_chorus[lchorus_counter2];
float lchore=rbuff_chorus[rchorus_counter2];
left_float=lchore+left_float;	
right_float=rchore+right_float;
Compressor_work();
left_value=f2i(left_float*master_coef);
right_value=f2i(right_float*master_coef);
if (left_value>32767)left_value=32767;
if (left_value<-32767)left_value=-32767;
if (right_value>32767)right_value=32767;
if (right_value<-32767)right_value=-32767;
}

void GetPlayerValues3(float master_coef)
{
Sp_Player3();
left_value=f2i(left_float*master_coef);
right_value=f2i(right_float*master_coef);
if (left_value>32767)left_value=32767;
if (left_value<-32767)left_value=-32767;
if (right_value>32767)right_value=32767;
if (right_value<-32767)right_value=-32767;
}

void RawRenderizer(void)
{
plx=0;
char buffer[80];
sprintf(buffer,"%s.wav",name);

if (!hd_isrecording)
{
WaveFile RF;
RF.OpenForWrite (buffer,44100,16,2);
guiDial(90,368,80,16,"WAV Render",100);
SongStop();
Sleep(500);
sprintf(buffer,"Rendering module to '%s.wav' file. Please wait...",name);
mess_box(buffer);
Sleep(500);
ped_line=0;
cPosition=0;
SongPlay();
int lastline=ped_line;
long filesize=0;
bool bru=false;
while(cPosition>0 || ped_line>0 || bru==false){
if (ped_line>0)bru=true;
GetPlayerValues(mas_vol); // <-- L INT
RF.WriteStereoSample(left_value,right_value);
filesize+=4;
if (lastline!=ped_line){
float cline=(cPosition*64.0f)+ped_line;
float tline=sLength*64.0f;
float cfr=(cline*100.0f)/tline; 
sprintf(buffer,"Rendering line %d, position %d. (%.2f%%)     File Size: %.2f Megabytes",lastline,cPosition,cfr, float(filesize/1048576.0f));
mess_box(buffer);
lastline=ped_line;
}
}

RF.Close();
SongStop();
int minutos=filesize/10584000;
int segundos=(filesize-minutos*10584000)/176400;

sprintf(buffer,"Wav Render finished. File size: %.2f Megabytes. Playback Time: %d'%d''.",float(filesize/1048576.0f),minutos,segundos);
mess_box(buffer);
ped_line=0;
cPosition=0;
Actualize_Main_Ed();

last_index=-1;
Read_SMPT();
ltActualize(0);

mess_box(buffer);
Sleep(500);
guiDial(90,368,80,16,"WAV Render",200);
Actupated(0);
}

rawrender=false;
}

void MidiAllNotesOff(void)
{
if (c_midiout!=-1)
{
for (int no_track=0;no_track<16;no_track++)
midiOutShortMsg(midiout_handle, (176+no_track) | (123 << 8) | (0 << 16)); 
}
}

void MidiReset(void)
{
MidiAllNotesOff();

for(int mreset=0;mreset<16;mreset++)
{
	LastProgram[mreset]=-1;
}

}

void draw_lfoed(void)
{
guiDial(112,309,64,16,"Track FX",100);
guiDial(178,309,64,16,"Midi/CSynth",200);
guiDial(244,309,64,16,"Master",200);
guiDial(310,309,64,16,"Sequencer",200);
guiDial(376,309,64,16,"Disk IO",200);
guiDial(442,309,64,16,"Track",200);
guiDial(508,309,64,16,"Instrument",200);
guiDial(574,309,64,16,"FX Setup",200);
guiDial(640,309,64,16,"Extend Edit",200);
guiDial(706,309,64,16,"303",200);

guiDial2("Track FX: Filter LFO And Flanger");
guiDial3(8,362,64,16,"Frequency",200);
guiDial3(8,380,64,16,"Amplitude",200);
guiDial3(230,334,288,110,"Flanger Settings",128);
guiDial3(238,350,56,16,"Amount",200);
guiDial3(238,368,56,16,"Period",200);
guiDial3(238,386,56,16,"Amplitude",200);
guiDial3(238,404,56,16,"Feedback",200);
guiDial3(238,422,56,16,"Delay",200);
guiDial3(8,398,64,16,"LFO Status",200);
guiDial3(8,416,64,16,"Flanger3D",200);
guiDial3(524,334,104,110,"Track Atributes",128);
guiDial3(532,350,88,16,"Panning Change",200);
}

void Actualize_Lfo_Ed(char gode)
{
if(userscreen==7)
{
	char tmp[16];
	if (gode==0 || gode==1){
	if (FLANGER_AMOUNT[ped_track]>1.0f)FLANGER_AMOUNT[ped_track]=1.0f;
	if (FLANGER_AMOUNT[ped_track]<-1.0f)FLANGER_AMOUNT[ped_track]=-1.0f;
	Realslider(298,350,64+f2i(FLANGER_AMOUNT[ped_track]*64.0f));
	outlong(448,350,long(FLANGER_AMOUNT[ped_track]*100.0f),1);}

	if (gode==0 || gode==7){
	if (FLANGER_FEEDBACK[ped_track]>1.0f)FLANGER_FEEDBACK[ped_track]=1.0f;
	if (FLANGER_FEEDBACK[ped_track]<-1.0f)FLANGER_FEEDBACK[ped_track]=-1.0f;
	Realslider(298,404,64+f2i(FLANGER_FEEDBACK[ped_track]*64.0f));
	outlong(448,404,long(FLANGER_FEEDBACK[ped_track]*100.0f),1);}

	if (gode==0 || gode==4){
	if (FLANGER_DEPHASE[ped_track]>3.1415927f)FLANGER_DEPHASE[ped_track]=3.1415927f;
	if (FLANGER_DEPHASE[ped_track]<0.0f)FLANGER_DEPHASE[ped_track]=0.0f;
	Realslider2(74,416,f2i(FLANGER_DEPHASE[ped_track]*20.371833f));
	outlong(158,416,f2i(FLANGER_DEPHASE[ped_track]*57.29578f),6);}

	if (gode==0 || gode==5){
	if (FLANGER_RATE[ped_track]<0.000001f)FLANGER_RATE[ped_track]=0.000001f;
	if (FLANGER_RATE[ped_track]>0.0001363f)FLANGER_RATE[ped_track]=0.0001363f;
	Realslider(298,368,f2i(FLANGER_RATE[ped_track]*939104.92f));
	outlong(448,368,long(0.1424758f/FLANGER_RATE[ped_track]),2);}

	if (gode==0 || gode==6){
	if (FLANGER_AMPL[ped_track]>0.01f)FLANGER_AMPL[ped_track]=0.01f;
	if (FLANGER_AMPL[ped_track]<0.0f)FLANGER_AMPL[ped_track]=0.0f;
	Realslider(298,386,f2i(FLANGER_AMPL[ped_track]*12800.0f));
	outlong(448,386,f2i(FLANGER_AMPL[ped_track]*10000.0f),1);
	}

	if (gode==0 || gode==2){
	if (LFORATE[ped_track]<0.0001f)LFORATE[ped_track]=0.0001f;
	if (LFORATE[ped_track]>0.0078125f)LFORATE[ped_track]=0.0078125f;
	Realslider(74,362,f2i(LFORATE[ped_track]*16384.0f));
	float tmprate=(8.1632653f/LFORATE[ped_track]);
	outlong(74,398,(long)tmprate,2);
	tmprate=1000.0f/tmprate;
	outfloat(136,398,tmprate,3);}
	
	if (gode==0 || gode==3){
	if (LFOAMPL[ped_track]<0)LFOAMPL[ped_track]=0;
	if (LFOAMPL[ped_track]>128)LFOAMPL[ped_track]=128;
	Realslider(74,380,f2i(LFOAMPL[ped_track]));}
	
	if (gode==0 || gode==9){
	sprintf(tmp,"Flt.LFO[%d]",ped_track);
	guiDial3(8,344,64,16,tmp,200);
	if(LFO_ON[ped_track]==1){
	guiDial(74,344,20,16,"On",100);
	guiDial(96,344,20,16,"Off",200);
	}else{
	guiDial(74,344,20,16,"On",200);
	guiDial(96,344,20,16,"Off",100);}}

	if (gode==0 || gode==10){
	sprintf(tmp,"Flanger[%d]",ped_track);
	guiDial3(118,344,64,16,tmp,200);
	if(FLANGER_ON[ped_track]==1){
	guiDial(184,344,20,16,"On",100);
	guiDial(206,344,20,16,"Off",200);
	}else{
	guiDial(184,344,20,16,"On",200);
	guiDial(206,344,20,16,"Off",100);}}

	if (gode==0 || gode==8){
	if (FLANGER_DELAY[ped_track]>4096)FLANGER_DELAY[ped_track]=4096;
	if (FLANGER_DELAY[ped_track]<0)FLANGER_DELAY[ped_track]=0;	
	if(fld_chan==true){
	FLANGER_OFFSET[ped_track]=8192;
	foff2[ped_track]=float(FLANGER_OFFSET[ped_track]-FLANGER_DELAY[ped_track]);
	foff1[ped_track]=float(FLANGER_OFFSET[ped_track]-FLANGER_DELAY[ped_track]);	
	fld_chan=false;}
	Realslider(298,422,FLANGER_DELAY[ped_track]/32);
	outlong(448,422,long(FLANGER_DELAY[ped_track]/44.1f),2);
	}

	if (gode==0 || gode==11)
	{
		if (Dispan[ped_track])
		{
		guiDial(532,368,42,16,"Instant.",200);
		guiDial(576,368,44,16,"Smooth",100);
		}else{
		guiDial(532,368,42,16,"Instant.",100);
		guiDial(576,368,44,16,"Smooth",200);
		}
	}

}//User gui screen match
}
#endif // AUDACIOUS_UADE
float ApplyLfo(float cy,char trcy)
{
	if (LFO_ON[trcy]==1)
	{
	cy+=SIN[f2i(LFOGR[trcy])]*LFOAMPL[trcy];
	LFOGR[trcy]+=LFORATE[trcy];
	if (LFOGR[trcy]>=360)LFOGR[trcy]-=360;
	}

	if (cy<1.0f)cy=1.0f;
	if (cy>126.0f)cy=126.0f;

	return cy;
}

float filterRingMod(char ch,float input,float f,float q)
{
  q++;
  f=float(f*0.0078125f);
  buf0[ch]+=f*(q*0.125f);
  if (buf0[ch]>=360.0f)buf0[ch]-=360.0f;
  return input*SIN[f2i(buf0[ch])];
}

float filterRingModStereo(char ch,float input)
{
  return float(input*cos(buf0[ch]*0.0174532));
}

float filterWater(char ch,float input,float f,float q)
{
  f=127.0f-f;
  float ad=input-buf0[ch];
  if (ad>1.0f || ad<-1.0f)
	  buf0[ch]+=ad/f;

  return buf0[ch];
}

float filterWaterStereo(char ch,float input,float f,float q)
{
  f=127.0f-f;
  float ad=input-buf1[ch];
  if (ad>1.0f || ad<-1.0f)
	  buf1[ch]+=ad/f;
  return buf1[ch];
}

float filterDelta(char ch,float input,float f,float q)
{
  f=127.0f-f;
  q*=0.007874f;
  
  float output=buf1[ch];
  if (buf1[ch]>1.0f || buf1[ch]<-1.0f)buf1[ch]*=q;

  buf0[ch]++;
  if(buf0[ch]>=f)
  {
  buf0[ch]=0;
  output=input;
  buf1[ch]=input;
  }
  return output;
}

float filterDeltaStereo(char ch,float input,float f,float q)
{
  f=127.0f-f;
  q*=0.007874f;
  
  float output=buf124[ch];
  if (buf124[ch]>1.0f || buf124[ch]<-1.0f)buf124[ch]*=q;

  buf024[ch]++;
  if(buf024[ch]>=f)
  {
  buf024[ch]=0;
  output=input;
  buf124[ch]=input;
  }
  return output;
}

float filterBellShaped(char ch,float input,float f,float q,float g)
{
  input++;
  q*=0.007874f;
  
  if (q<0.01f)q=0.01f;
  float freq=320+(f*127.65625f);
  float a,b,c;
  float a0, a1, a2, b1, b2; //filter coefficients
  float Wn,Wp;
  float gain=g/6.6f;
  if (freq > 22100) freq = 22100.0f; // apply Nyquist frequency
  Wn=1.0f/(6.2831853f*freq); // freq of center
  Wp=float(Wn/tan(Wn/88200.0f)); // prewarped frequency
  a=(Wn*Wn*Wp*Wp);
  float t1=Wn*Wp*q;
  b=(3+gain)*t1;
  c=(3-gain)*t1;
  t1=a+c+1;
  b2 = (1-c+a)/t1;
  a2 = (1-b+a)/t1;
  b1 = a1 = 2*(1-a)/t1;
  a0 = (a+b+1)/t1;
  xi0[ch] = input - b1*xi1[ch] - b2*xi2[ch];
  float output = a0*xi0[ch] + a1*xi1[ch] + a2*xi2[ch];
  xi2[ch]=xi1[ch];
  xi1[ch]=xi0[ch];
  return output;
}

void ResetFilters(char tr)
{buf024[tr]=0.0f;
buf124[tr]=0.0f;
buf0[tr]=0.0f;
buf1[tr]=0.0f;
fx1[tr]=0.0f;
fx2[tr]=0.0f;
fy1[tr]=0.0f;
fy2[tr]=0.0f;
xi0[tr]=xi1[tr]=xi2[tr]=0.0f;
}
#ifndef AUDACIOUS_UADE
void SeqFill(int st,int en,bool n)
{
for(int cl=st;cl<en;cl++)
for(char trk=0;trk<Songtracks;trk++)
SACTIVE[cl][trk]=n;
}		

void Afloop(void)
{
if(actuloop==1 || actuloop==3)
{for (int a=0;a<200;a++){
	long ose=a+LoopStart[ped_patsam][ped_split];
	if (ose<SampleNumSamples[ped_patsam][ped_split]){
	int v=*(RawSamples[ped_patsam][0][ped_split]+ose)/1024;
	S->drawVLine(a+220,432,400-v,0x5424);
	S->drawVLine(a+220,400-v,368,0);
	}
	else
	S->drawVLine(a+20,368,432,0x0121);
}}

if(actuloop==2 || actuloop==3)
{for (int b=0;b<200;b++){
	long ose=(LoopEnd[ped_patsam][ped_split]-200)+b;
	if (ose>-1 && ose<SampleNumSamples[ped_patsam][ped_split])
	{
	int v=*(RawSamples[ped_patsam][0][ped_split]+ose)/1024;
	S->drawVLine(b+20,432,400-v,0x6523);
	S->drawVLine(b+20,400-v,368,0x0001);
	}
	else
	S->drawVLine(b+20,368,432,0x0121);
}}
actuloop=0;
}
#endif // AUDACIOUS_UADE
float int_filter2p(char ch,float input,float f,float q, float q2)
{
q*=0.0787401f;
input=filter2px(ch,input,f,q2);
return float(32767.0f*pow(abs(f2i(input))/32767.0f,1.0f-q/11.0f));
}

float filter2px(char ch,float input,float f,float q)
{
  f*=0.0078125f;
  float fa = float(1.0 - f); 
  float fb = float(q * (1.0 + (1.0/fa)));
  buf0[ch] = fa * buf0[ch] + f * (input + fb * (buf0[ch] - buf1[ch])); 
  buf1[ch] = fa * buf1[ch] + f * buf0[ch];
  float output = buf1[ch];  
  return output;
}
#ifndef AUDACIOUS_UADE
void SaveInst(void)
{
guiDial(172,350,80,16,"Save Inst",100);
FILE *in;
char Temph[96];
char extension[10];
sprintf(extension,"TWNNINS0");
sprintf (Temph,"Saving '%s.nti' instrument on current directory...",nameins[ped_patsam]);
mess_box(Temph);
sprintf(Temph,"%s.nti",nameins[ped_patsam]);
in = fopen(Temph,"wb");
if (in!=NULL)
{
// Writing NoiseTrekker header & name...
fwrite(extension, sizeof( char),9,in);
fwrite(&nameins[ped_patsam], sizeof( char ), 20,in);

// Writing sample data
int swrite=ped_patsam;
fwrite(&Midiprg[swrite],sizeof(char),1,in);
fwrite(&Synthprg[swrite],sizeof(bool),1,in);
fwrite(&PARASynth[swrite], sizeof(SynthParameters),1,in);

for (int slwrite=0;slwrite<16;slwrite++)
{
fwrite(&SampleType[swrite][slwrite], sizeof(char ),1,in);
if (SampleType[swrite][slwrite]!=0)
{
fwrite(&SampleName[swrite][slwrite], sizeof(char),256,in);
fwrite(&Basenote[swrite][slwrite],sizeof(char),1,in);
fwrite(&LoopStart[swrite][slwrite], sizeof(long ),1,in);
fwrite(&LoopEnd[swrite][slwrite], sizeof(long ),1,in);
fwrite(&LoopType[swrite][slwrite], sizeof(char ),1,in);
fwrite(&SampleNumSamples[swrite][slwrite], sizeof(long ),1,in);
fwrite(&Finetune[swrite][slwrite], sizeof(char ),1,in);
fwrite(&SampleVol[swrite][slwrite], sizeof(float ),1,in);
fwrite(&FDecay[swrite][slwrite], sizeof(float ),1,in);
fwrite(RawSamples[swrite][0][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
fwrite(&SampleChannels[swrite][slwrite], sizeof(char ),1,in);
if (SampleChannels[swrite][slwrite]==2)
fwrite(RawSamples[swrite][1][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
}// Exist Sample
}
fclose(in);

Read_SMPT();
last_index=-1;
ltActualize(0);
Actualize_Patterned();
mess_box("Instrument saved succesful...");	
}
else
{
mess_box("Instrument save failed...");
}
if (snamesel==1 || snamesel==4 || snamesel==5){snamesel=0;Actualize_Main_Ed();}
guiDial(172,350,80,16,"Save Inst",200);
}

void LoadInst(void)
{
mess_box("Attempting to load a noisetrekker instrument file...");
Sleep(1000);	
FILE *in;
in = fopen(name,"rb");

if (in!=NULL)
{
// Reading and checking extension...
char extension[10];
fread(extension, sizeof( char),9,in);
if (strcmp(extension,"TWNNINS0")==0)
{ /* Ok, extension matched! */
KillInst();
mess_box("Loading 'NoiseTrekker' Instrument -> Header");	
fread(&nameins[ped_patsam], sizeof( char ), 20,in);

// Reading sample data
mess_box("Loading 'NoiseTrekker' Instrument -> Sample data");	

int swrite=ped_patsam;

fread(&Midiprg[swrite],sizeof(char),1,in);
fread(&Synthprg[swrite],sizeof(bool),1,in);
fread(&PARASynth[swrite], sizeof(SynthParameters),1,in);

for (int slwrite=0;slwrite<16;slwrite++)
{
fread(&SampleType[swrite][slwrite], sizeof(char ),1,in);
if (SampleType[swrite][slwrite]!=0)
{
fread(&SampleName[swrite][slwrite], sizeof(char),256,in);
fread(&Basenote[swrite][slwrite],sizeof(char),1,in);
fread(&LoopStart[swrite][slwrite], sizeof(long ),1,in);
fread(&LoopEnd[swrite][slwrite], sizeof(long ),1,in);
fread(&LoopType[swrite][slwrite], sizeof(char ),1,in);
fread(&SampleNumSamples[swrite][slwrite], sizeof(long ),1,in);
fread(&Finetune[swrite][slwrite], sizeof(char ),1,in);
fread(&SampleVol[swrite][slwrite], sizeof(float ),1,in);
fread(&FDecay[swrite][slwrite], sizeof(float ),1,in);
RawSamples[swrite][0][slwrite]=(short *)malloc(SampleNumSamples[swrite][slwrite]*2);
fread(RawSamples[swrite][0][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
fread(&SampleChannels[swrite][slwrite], sizeof(char ),1,in);
if (SampleChannels[swrite][slwrite]==2)
{
RawSamples[swrite][1][slwrite]=(short *)malloc(SampleNumSamples[swrite][slwrite]*2);
fread(RawSamples[swrite][1][slwrite], sizeof(short),SampleNumSamples[swrite][slwrite],in);
}
}// Exist Sample
}
fclose(in);
Actualize_Patterned();
Actualize_Sample_Ed(2,0);
mess_box("Instrument loaded ok.");
}
else
{
mess_box("That file is not a NoiseTrekker instrument-file...");
}
}
else
{
mess_box("Instrument loading failed. (Probably: file not found)");
}
if (snamesel==1 || snamesel==4 || snamesel==5){snamesel=0;Actualize_Main_Ed();}
}

void Coolwrite(int x, int y, int colour, char* txt)
{
S->printXY(x-1,y+1,0x00000000,txt);
S->printXY(x,y,colour,txt);
}

void KillInst()
{
for(int z=0;z<16;z++)
{
	if(SampleType[ped_patsam][z]!=0)
	{
	free(RawSamples[ped_patsam][0][z]);
	
	if(SampleChannels[ped_patsam][z]==2)
	free(RawSamples[ped_patsam][1][z]);
	
	SampleChannels[ped_patsam][z]=0;
	SampleType[ped_patsam][z]=0;
	LoopStart[ped_patsam][z]=0;
	LoopEnd[ped_patsam][z]=0;
	LoopType[ped_patsam][z]=0;
	SampleNumSamples[ped_patsam][z]=0;
	Finetune[ped_patsam][z]=0;
	SampleVol[ped_patsam][z]=0.0;
	FDecay[ped_patsam][z]=0.0;
	Basenote[ped_patsam][z]=48;
	sprintf(SampleName[ped_patsam][z],"Unnamed");
	Midiprg[ped_patsam]=-1;
	Synthprg[ped_patsam]=false;
	
	}	
}

}

void StopIfSp(void)
{
for(char u=0;u<MAX_TRACKS;u++)

if(sp_channelsample[u]==ped_patsam)
{
sp_Stage[u]=0;
Player_FD[u]=0;
ramper[u]=0;
}
}

void DeleteInstrument(void)
{
ss->stop();
guiDial(320,134,64,16,"Delete",100);
StopIfSp();
Sleep(256);
KillInst();
mess_box("Instrument deleted.");
guiDial(320,134,64,16,"Delete",200);
RefreshSample();
NewWavy();
ss->play();
}

void RefreshSample(void)
{
seditor=0;
ped_split=0;
if(userscreen==2){draw_sampleed();Actualize_Sample_Ed(2,0);}
}

void FadeToBlack(void)
{
	for (float fad=1.0f;fad>0;fad-=0.005f)
	{S->setGammaFade(fad,fad,fad);Sleep(1);}
}

void ShowInfo(void)
{
char tmp[256];
int pattsize=nPatterns*12288;
int sampsize=0;

for(int pp=0;pp<256;pp++)
{
for(int z=0;z<16;z++)
{
	if(SampleType[pp][z]!=0)
	sampsize+=SampleChannels[pp][z]*SampleNumSamples[pp][z];	
}
}
sprintf(tmp,"Sample bank size: %d bytes, pattern bank [%d patterns]: %d bytes.",sampsize,nPatterns,pattsize);
mess_box(tmp);
}

void draw_exted(void)
{
S->setColor(0,0,0);
bjbox(0,186,CONSOLE_WIDTH,248);
snamesel=0;
guiDial(640,433,64,16,"Back",200);
guiDial3(0,433,638,16,"Extended Pattern Editor Mode",200);		
if(restx>0)guiDial3(706,433,restx-2,16,"",200);
VIEWLINE=14;
VIEWLINE2=-13;
YVIEW=300;
Actupated(0);
}

void draw_back(void)
{
VIEWLINE=7;
VIEWLINE2=-6;
YVIEW=244; 
guiDial3(0,309,110,16,"User Window",200);
guiDial3(0,327,fsize,122,"",144);
actlogo=true;
userscreen=0;draw_mained();Actualize_Main_Ed();
if(restx>0)guiDial3(772,309,restx-2,16,"",200);
S->copy(FRAMEWORK, 0, 184);
Actupated(0);
}

void Anat(int posil)
{
if(pSequence[posil]>=nPatterns)
nPatterns=pSequence[posil]+1;
}


#include "CSynth_gui_pro.cpp"
#include "mod_import.cpp"
int postAction(Console* C)
{	

ss->release();
delete sw;
FreeAll();
SelectObject(dc,old_f);DeleteObject(fnt);
RELEASEINT(MOUSEBACK);
RELEASEINT(NTKLOGO);
RELEASEINT(SKIN303);
RELEASEINT(KNOB1);
RELEASEINT(PFONT);
RELEASEINT(FRAMEWORK);

// Freeing Allocated Patterns
free(RawPatterns);
SaveSettings();
return 0;
}
#else
#include "CSynth_gui_pro.cpp"
#endif // AUDACIOUS_UADE
void ComputeStereo(char channel)
{
LVol[channel]=1.0f-TPan[channel];
RVol[channel]=1.0f-LVol[channel];
if(LVol[channel]>0.5f)LVol[channel]=0.5f;
if(RVol[channel]>0.5f)RVol[channel]=0.5f;
}

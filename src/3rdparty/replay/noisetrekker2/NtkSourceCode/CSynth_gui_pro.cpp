/* The Original && Best, NoiseTrekker Main Source Code [Part 2]*/

//
// CSynth-Noisetrekker GUI functions prototypes 
//

void Actualize_SynthParSlider(void)
{
switch(ped_synthpar)
{
case 1: /* Osc1_PW */
	Realslider(406,388,PARASynth[ped_patsam].osc1_pw/4);
	outlong(572,388,PARASynth[ped_patsam].osc1_pw,0);
break;

case 2: /* Osc2_PW */
	Realslider(406,388,PARASynth[ped_patsam].osc2_pw/4);
	outlong(572,388,PARASynth[ped_patsam].osc2_pw,0);
break;

case 3: /* Osc1_Detune */
	Realslider(406,388,PARASynth[ped_patsam].osc2_detune);
	outlong(572,388,PARASynth[ped_patsam].osc2_detune-64,0);
break;

case 4: /* Osc2_Finetune */
	Realslider(406,388,PARASynth[ped_patsam].osc2_finetune);
	outlong(572,388,(PARASynth[ped_patsam].osc2_finetune*100)/128,1);
break;

case 5: /* VCF Cutoff */
	Realslider(406,388,PARASynth[ped_patsam].vcf_cutoff);
	outlong(572,388,PARASynth[ped_patsam].vcf_cutoff*172,3);
break;

case 6: /* VCF_Resonance */
	Realslider(406,388,PARASynth[ped_patsam].vcf_resonance);
	outlong(572,388,(PARASynth[ped_patsam].vcf_resonance*100)/128,1);
break;

case 7: /* ENV1_Attack */
	Realslider(406,388,PARASynth[ped_patsam].env1_attack/512);
	outlong(572,388,PARASynth[ped_patsam].env1_attack/44,2);
break;

case 8: /* ENV1_Decay */
	Realslider(406,388,PARASynth[ped_patsam].env1_decay/512);
	outlong(572,388,PARASynth[ped_patsam].env1_decay/44,2);
break;

case 9: /* ENV1_Sustain */
	Realslider(406,388,PARASynth[ped_patsam].env1_sustain);
	outlong(572,388,(PARASynth[ped_patsam].env1_sustain*100)/128,1);
break;

case 10: /* ENV1_Release */
	Realslider(406,388,PARASynth[ped_patsam].env1_release/512);
	outlong(572,388,PARASynth[ped_patsam].env1_release/44,2);
break;

case 11: /* ENV2_Attack */
	Realslider(406,388,PARASynth[ped_patsam].env2_attack/512);
	outlong(572,388,PARASynth[ped_patsam].env2_attack/44,2);
break;

case 12: /* ENV2_Decay */
	Realslider(406,388,PARASynth[ped_patsam].env2_decay/512);
	outlong(572,388,PARASynth[ped_patsam].env2_decay/44,2);
break;

case 13: /* ENV2_Sustain */
	Realslider(406,388,PARASynth[ped_patsam].env2_sustain);
	outlong(572,388,(PARASynth[ped_patsam].env2_sustain*100)/128,1);
break;

case 14: /* ENV2_Release */
	Realslider(406,388,PARASynth[ped_patsam].env2_release/512);
	outlong(572,388,PARASynth[ped_patsam].env2_release/44,2);
break;

case 15: /* LFO1_Period */
	Realslider(406,388,PARASynth[ped_patsam].lfo1_period/20);
	outfloat(572,388,(float)PARASynth[ped_patsam].lfo1_period/10,8);
break;

case 16: /* LFO2_Period */
	Realslider(406,388,PARASynth[ped_patsam].lfo2_period/20);
	outfloat(572,388,(float)PARASynth[ped_patsam].lfo2_period/10,8);
break;

case 17:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_osc1_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_osc1_pw-64.0f)*1.5625f,1);
break;

case 18:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_osc2_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_osc2_pw-64.0f)*1.5625f,1);
break;

case 19:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_osc1_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_osc1_pitch-64.0f)*1.5625f,1);
break;

case 20:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_osc2_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_osc2_pitch-64.0f)*1.5625f,1);
break;

case 21:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_osc1_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_osc1_volume-64.0f)*1.5625f,1);
break;

case 22:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_osc2_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_osc2_volume-64.0f)*1.5625f,1);
break;

case 23:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_vcf_cutoff);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_vcf_cutoff-64.0f)*1.5625f,1);
break;

case 24:
	Realslider(406,388,PARASynth[ped_patsam].lfo1_vcf_resonance);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo1_vcf_resonance-64.0f)*1.5625f,1);
break;

case 25:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_osc1_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_osc1_pw-64.0f)*1.5625f,1);
break;

case 26:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_osc2_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_osc2_pw-64.0f)*1.5625f,1);
break;

case 27:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_osc1_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_osc1_pitch-64.0f)*1.5625f,1);
break;

case 28:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_osc2_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_osc2_pitch-64.0f)*1.5625f,1);
break;

case 29:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_osc1_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_osc1_volume-64.0f)*1.5625f,1);
break;

case 30:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_osc2_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_osc2_volume-64.0f)*1.5625f,1);
break;

case 31:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_vcf_cutoff);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_vcf_cutoff-64.0f)*1.5625f,1);
break;

case 32:
	Realslider(406,388,PARASynth[ped_patsam].lfo2_vcf_resonance);
	outfloat(572,388,((float)PARASynth[ped_patsam].lfo2_vcf_resonance-64.0f)*1.5625f,1);
break;

case 33:
	Realslider(406,388,PARASynth[ped_patsam].env1_osc1_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_osc1_pw-64.0f)*1.5625f,1);
break;

case 34:
	Realslider(406,388,PARASynth[ped_patsam].env1_osc2_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_osc2_pw-64.0f)*1.5625f,1);
break;

case 35:
	Realslider(406,388,PARASynth[ped_patsam].env1_osc1_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_osc1_pitch-64.0f)*1.5625f,1);
break;

case 36:
	Realslider(406,388,PARASynth[ped_patsam].env1_osc2_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_osc2_pitch-64.0f)*1.5625f,1);
break;

case 37:
	Realslider(406,388,PARASynth[ped_patsam].env1_osc1_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_osc1_volume-64.0f)*1.5625f,1);
break;

case 38:
	Realslider(406,388,PARASynth[ped_patsam].env1_osc2_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_osc2_volume-64.0f)*1.5625f,1);
break;

case 39:
	Realslider(406,388,PARASynth[ped_patsam].env1_vcf_cutoff);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_vcf_cutoff-64.0f)*1.5625f,1);
break;

case 40:
	Realslider(406,388,PARASynth[ped_patsam].env1_vcf_resonance);
	outfloat(572,388,((float)PARASynth[ped_patsam].env1_vcf_resonance-64.0f)*1.5625f,1);
break;

case 41:
	Realslider(406,388,PARASynth[ped_patsam].env2_osc1_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_osc1_pw-64.0f)*1.5625f,1);
break;

case 42:
	Realslider(406,388,PARASynth[ped_patsam].env2_osc2_pw);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_osc2_pw-64.0f)*1.5625f,1);
break;

case 43:
	Realslider(406,388,PARASynth[ped_patsam].env2_osc1_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_osc1_pitch-64.0f)*1.5625f,1);
break;

case 44:
	Realslider(406,388,PARASynth[ped_patsam].env2_osc2_pitch);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_osc2_pitch-64.0f)*1.5625f,1);
break;

case 45:
	Realslider(406,388,PARASynth[ped_patsam].env2_osc1_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_osc1_volume-64.0f)*1.5625f,1);
break;

case 46:
	Realslider(406,388,PARASynth[ped_patsam].env2_osc2_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_osc2_volume-64.0f)*1.5625f,1);
break;

case 47:
	Realslider(406,388,PARASynth[ped_patsam].env2_vcf_cutoff);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_vcf_cutoff-64.0f)*1.5625f,1);
break;

case 48:
	Realslider(406,388,PARASynth[ped_patsam].env2_vcf_resonance);
	outfloat(572,388,((float)PARASynth[ped_patsam].env2_vcf_resonance-64.0f)*1.5625f,1);
break;

case 49:
	Realslider(406,388,PARASynth[ped_patsam].osc3_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].osc3_volume-64.0f)*1.5625f,1);
break;

case 50:
	Realslider(406,388,PARASynth[ped_patsam].ptc_glide);
	outfloat(572,388,((float)PARASynth[ped_patsam].ptc_glide)*0.78125f,1);
break;

case 51:
	Realslider(406,388,PARASynth[ped_patsam].glb_volume);
	outfloat(572,388,((float)PARASynth[ped_patsam].glb_volume)*0.78125f,1);
break;

}
}

void IniCsParNames(void)
{
sprintf(CS_PAR_NAME[1],"Osc1 Phase Distortion");
sprintf(CS_PAR_NAME[2],"Osc2 Phase Distortion");
sprintf(CS_PAR_NAME[3],"Osc2 Detune");
sprintf(CS_PAR_NAME[4],"Osc2 Finetune");
sprintf(CS_PAR_NAME[5],"Vcf CutOff");
sprintf(CS_PAR_NAME[6],"Vcf Resonance");
sprintf(CS_PAR_NAME[7],"Env1 Attack");
sprintf(CS_PAR_NAME[8],"Env1 Decay");
sprintf(CS_PAR_NAME[9],"Env1 Sustain Level");
sprintf(CS_PAR_NAME[10],"Env1 Release");
sprintf(CS_PAR_NAME[11],"Env2 Attack");
sprintf(CS_PAR_NAME[12],"Env2 Decay");
sprintf(CS_PAR_NAME[13],"Env2 Sustain Level");
sprintf(CS_PAR_NAME[14],"Env2 Release");
sprintf(CS_PAR_NAME[15],"Lfo1 Period");
sprintf(CS_PAR_NAME[16],"Lfo2 Period");
sprintf(CS_PAR_NAME[17],"Lfo1->Osc1 PD");
sprintf(CS_PAR_NAME[18],"Lfo1->Osc2 PD");
sprintf(CS_PAR_NAME[19],"Lfo1->Osc1 Pitch");
sprintf(CS_PAR_NAME[20],"Lfo1->Osc2 Pitch");
sprintf(CS_PAR_NAME[21],"Lfo1->Osc1 Volume");
sprintf(CS_PAR_NAME[22],"Lfo1->Osc2 Volume");
sprintf(CS_PAR_NAME[23],"Lfo1->Vcf CutOff");
sprintf(CS_PAR_NAME[24],"Lfo1->Vcf Resonance");
sprintf(CS_PAR_NAME[25],"Lfo2->Osc1 PD");
sprintf(CS_PAR_NAME[26],"Lfo2->Osc2 PD");
sprintf(CS_PAR_NAME[27],"Lfo2->Osc1 Pitch");
sprintf(CS_PAR_NAME[28],"Lfo2->Osc2 Pitch");
sprintf(CS_PAR_NAME[29],"Lfo2->Osc1 Volume");
sprintf(CS_PAR_NAME[30],"Lfo2->Osc2 Volume");
sprintf(CS_PAR_NAME[31],"Lfo2->Vcf CutOff");
sprintf(CS_PAR_NAME[32],"Lfo2->Vcf Resonance");
sprintf(CS_PAR_NAME[33],"Env1->Osc1 PD");
sprintf(CS_PAR_NAME[34],"Env1->Osc2 PD");
sprintf(CS_PAR_NAME[35],"Env1->Osc1 Pitch");
sprintf(CS_PAR_NAME[36],"Env1->Osc2 Pitch");
sprintf(CS_PAR_NAME[37],"Env1->Osc1 Volume");
sprintf(CS_PAR_NAME[38],"Env1->Osc2 Volume");
sprintf(CS_PAR_NAME[39],"Env1->Vcf CutOff");
sprintf(CS_PAR_NAME[40],"Env1->Vcf Resonance");
sprintf(CS_PAR_NAME[41],"Env2->Osc1 PD");
sprintf(CS_PAR_NAME[42],"Env2->Osc2 PD");
sprintf(CS_PAR_NAME[43],"Env2->Osc1 Pitch");
sprintf(CS_PAR_NAME[44],"Env2->Osc2 Pitch");
sprintf(CS_PAR_NAME[45],"Env2->Osc1 Volume");
sprintf(CS_PAR_NAME[46],"Env2->Osc2 Volume");
sprintf(CS_PAR_NAME[47],"Env2->Vcf CutOff");
sprintf(CS_PAR_NAME[48],"Env2->Vcf Resonance");
sprintf(CS_PAR_NAME[49],"Sub-Oscillator Volume");
sprintf(CS_PAR_NAME[50],"Portamento Glide");
sprintf(CS_PAR_NAME[51],"Final [global] Volume");
}

void CParcha(int cpar)
{
switch(ped_synthpar)
{
case 1: /* Osc1_PW */
PARASynth[ped_patsam].osc1_pw=cpar*4;
break;

case 2: /* Osc2_PW */
PARASynth[ped_patsam].osc2_pw=cpar*4;
break;

case 3: /* Osc1_Detune */
PARASynth[ped_patsam].osc2_detune=cpar;
break;

case 4: /* Osc2_Finetune */
PARASynth[ped_patsam].osc2_finetune=cpar;
break;

case 5: /* VCF Cutoff */
PARASynth[ped_patsam].vcf_cutoff=cpar;
break;

case 6: /* VCF_Resonance */
PARASynth[ped_patsam].vcf_resonance=cpar;
break;

case 7: /* ENV1_Attack */
PARASynth[ped_patsam].env1_attack=cpar*512;
break;

case 8: /* ENV1_Decay */
PARASynth[ped_patsam].env1_decay=cpar*512;
break;

case 9: /* ENV1_Sustain */
PARASynth[ped_patsam].env1_sustain=cpar;
break;

case 10: /* ENV1_Release */
PARASynth[ped_patsam].env1_release=cpar*512;
break;

case 11: /* ENV2_Attack */
PARASynth[ped_patsam].env2_attack=cpar*512;
break;

case 12: /* ENV2_Decay */
PARASynth[ped_patsam].env2_decay=cpar*512;
break;

case 13: /* ENV2_Sustain */
PARASynth[ped_patsam].env2_sustain=cpar;
break;

case 14: /* ENV2_Release */
PARASynth[ped_patsam].env2_release=cpar*512;
break;

case 15: /* LFO1_Period */
PARASynth[ped_patsam].lfo1_period=cpar*20;
break;

case 16: /* LFO2_Period */
PARASynth[ped_patsam].lfo2_period=cpar*20;
break;

case 17:
PARASynth[ped_patsam].lfo1_osc1_pw=cpar;
break;

case 18:
PARASynth[ped_patsam].lfo1_osc2_pw=cpar;
break;

case 19:
PARASynth[ped_patsam].lfo1_osc1_pitch=cpar;
break;

case 20:
PARASynth[ped_patsam].lfo1_osc2_pitch=cpar;
break;

case 21:
PARASynth[ped_patsam].lfo1_osc1_volume=cpar;
break;

case 22:
PARASynth[ped_patsam].lfo1_osc2_volume=cpar;
break;

case 23:
PARASynth[ped_patsam].lfo1_vcf_cutoff=cpar;
break;

case 24:
PARASynth[ped_patsam].lfo1_vcf_resonance=cpar;
break;

case 25:
PARASynth[ped_patsam].lfo2_osc1_pw=cpar;
break;

case 26:
PARASynth[ped_patsam].lfo2_osc2_pw=cpar;
break;

case 27:
PARASynth[ped_patsam].lfo2_osc1_pitch=cpar;
break;

case 28:
PARASynth[ped_patsam].lfo2_osc2_pitch=cpar;
break;

case 29:
PARASynth[ped_patsam].lfo2_osc1_volume=cpar;
break;

case 30:
PARASynth[ped_patsam].lfo2_osc2_volume=cpar;
break;

case 31:
PARASynth[ped_patsam].lfo2_vcf_cutoff=cpar;
break;

case 32:
PARASynth[ped_patsam].lfo2_vcf_resonance=cpar;
break;

case 33:
PARASynth[ped_patsam].env1_osc1_pw=cpar;
break;

case 34:
PARASynth[ped_patsam].env1_osc2_pw=cpar;
break;

case 35:
PARASynth[ped_patsam].env1_osc1_pitch=cpar;
break;

case 36:
PARASynth[ped_patsam].env1_osc2_pitch=cpar;
break;

case 37:
PARASynth[ped_patsam].env1_osc1_volume=cpar;
break;

case 38:
PARASynth[ped_patsam].env1_osc2_volume=cpar;
break;

case 39:
PARASynth[ped_patsam].env1_vcf_cutoff=cpar;
break;

case 40:
PARASynth[ped_patsam].env1_vcf_resonance=cpar;
break;

case 41:
PARASynth[ped_patsam].env2_osc1_pw=cpar;
break;

case 42:
PARASynth[ped_patsam].env2_osc2_pw=cpar;
break;

case 43:
PARASynth[ped_patsam].env2_osc1_pitch=cpar;
break;

case 44:
PARASynth[ped_patsam].env2_osc2_pitch=cpar;
break;

case 45:
PARASynth[ped_patsam].env2_osc1_volume=cpar;
break;

case 46:
PARASynth[ped_patsam].env2_osc2_volume=cpar;
break;

case 47:
PARASynth[ped_patsam].env2_vcf_cutoff=cpar;
break;

case 48:
PARASynth[ped_patsam].env2_vcf_resonance=cpar;
break;

case 49:
PARASynth[ped_patsam].osc3_volume=cpar;
break;

case 50:
PARASynth[ped_patsam].ptc_glide=cpar;
break;

case 51:
PARASynth[ped_patsam].glb_volume=cpar;
break;

}
}

void SaveSynth(void)
{
FILE *in;
char Temph[96];
char extension[10];
sprintf(extension,"TWNNSYN0");
sprintf (Temph,"Saving '%s.nts' synthesizer program on current directory...",PARASynth[ped_patsam].presetname);
mess_box(Temph);
sprintf(Temph,"%s.nts",PARASynth[ped_patsam].presetname);
in = fopen(Temph,"wb");

if (in!=NULL)
{
fwrite(extension, sizeof( char),9,in);
fwrite(&PARASynth[ped_patsam].presetname, sizeof(SynthParameters), 1,in);
fclose(in);
Read_SMPT();
last_index=-1;
ltActualize(0);
mess_box("Synthesizer program saved succesful...");	
}
else
mess_box("Synthesizer program save failed...");

if(snamesel==3)
{
guiDial(432,352,164,16,PARASynth[ped_patsam].presetname,140);
snamesel=0;
}

}

void LoadSynth(void)
{
FILE *in;
in = fopen(name,"rb");

if (in!=NULL)
{
// Reading and checking extension...
char extension[10];
fread(extension, sizeof( char),9,in);

if (strcmp(extension,"TWNNSYN0")==0)
{
/* Ok, extension matched! */
mess_box("Loading 'NoiseTrekker' Synthesizer -> structure.");	
ResetSynthParameters(&PARASynth[ped_patsam]);
fread(&PARASynth[ped_patsam], sizeof(SynthParameters), 1,in);

if(PARASynth[ped_patsam].ptc_glide<1)PARASynth[ped_patsam].ptc_glide=64;
if(PARASynth[ped_patsam].glb_volume<1)PARASynth[ped_patsam].glb_volume=64;

if (userscreen==6)Actualize_Midi_Ed(1);
Synthprg[ped_patsam]=true;
sprintf(nameins[ped_patsam],PARASynth[ped_patsam].presetname);

Actualize_Sample_Ed(0,0);
Actualize_Main_Ed();
Actualize_Patterned();

mess_box("Synthesizer program loaded ok.");
}
else
mess_box("That file is not a NoiseTrekker synthesizer program file...");

fclose(in);
}
else
mess_box("Synthesizer program loading failed. (Probably: file not found)");
}

void Draw_Sampled_Wave(void)
{
if (draw_sampled_wave)
{
	if(SampleType[ped_patsam][ped_split]>0) /* Any Sample Out There? */
	{
	int strober=SampleChannels[ped_patsam][ped_split]*2;
	int s_ey=458+(resty/strober);
	int s_ey2=s_ey+((resty/strober)*2);
	int s_size=resty/strober;
	int s_coef=32768/s_size;

	if (rs_coef>s_coef)rs_coef=s_coef;
	if (rs_coef<1)rs_coef=1;

	int rcolor1=0xCCCC;
	int rcolor2=0x0;
	int	rcolor3=0x1234;
	
	if(SampleChannels[ped_patsam][ped_split]==1)
	{
	for (int s_ex=0;s_ex<512;s_ex++)
	{
	long const s_offset=(s_ex*sed_display_length)/512+sed_display_start;
	int h=*(RawSamples[ped_patsam][0][ped_split]+s_offset)/rs_coef;
	if(h>s_size)h=s_size;
	if(h<-s_size)h=-s_size;

	int s_y=s_ey-h;	
	if (sed_range_mode && s_offset>=sed_range_start && s_offset<=sed_range_end)
	{
	rcolor2=0x1232;
	rcolor1=0xDDDD;
	}else{
	rcolor1=0xCCCC;
	rcolor2=0x0;
	}

	S->drawVLine(s_ex,458,CONSOLE_HEIGHT2,rcolor2);
	S->drawVLine(s_ex,s_ey,s_y,rcolor1);
	S->drawVLine(s_ex,s_ey,s_ey,0xCCC0);
}
}// If

// STEREO DISPLAY
	
	if(SampleChannels[ped_patsam][ped_split]==2)
	{
	for (int s_ex=0;s_ex<512;s_ex++)
	{
	long const s_offset=(s_ex*sed_display_length)/512+sed_display_start;
	int h=*(RawSamples[ped_patsam][0][ped_split]+s_offset)/rs_coef;
	int h2=*(RawSamples[ped_patsam][1][ped_split]+s_offset)/rs_coef;
	if(h>s_size)h=s_size;
	if(h<-s_size)h=-s_size;
	if(h2>s_size)h2=s_size;
	if(h2<-s_size)h2=-s_size;
	int s_y=s_ey-h;
	int s_y2=s_ey2-h2;
	
	if (sed_range_mode && s_offset>=sed_range_start && s_offset<=sed_range_end)
	{
	rcolor2=0x1232;
	rcolor1=0xDDDD;
	}else{
	rcolor1=0xCCCC;
	rcolor2=0x0;
	}
	
	S->drawVLine(s_ex,458,CONSOLE_HEIGHT2,rcolor2);
	S->drawVLine(s_ex,s_ey,s_y,rcolor1);
	S->drawVLine(s_ex,s_ey2,s_y2,rcolor1);
	S->drawVLine(s_ex,s_ey,s_ey,0xCCC0);
	S->drawVLine(s_ex,s_ey2,s_ey2,0xCCC0);
}
}// If Stereo

if (LoopType[ped_patsam][ped_split])
{
	int LSX=LoopStart[ped_patsam][ped_split]-sed_display_start;
	int LEX=LoopEnd[ped_patsam][ped_split]-sed_display_start;

	LSX=(LSX*512)/sed_display_length;
	LEX=(LEX*512)/sed_display_length;

	if (LSX>=0 && LSX<512)
	{
	S->drawVLine(LSX,458,CONSOLE_HEIGHT2,0xEEEE);
	S->drawHLine(460,LSX+2,LSX+5,0xFFFF);
	S->drawHLine(461,LSX+2,LSX+2,0xFFFF);
	S->drawHLine(462,LSX+2,LSX+5,0xFFFF);
	S->drawHLine(463,LSX+5,LSX+5,0xFFFF);
	S->drawHLine(464,LSX+2,LSX+5,0xFFFF);
	}

	if (LEX>=0 && LEX<512)
	{
	S->drawVLine(LEX,458,CONSOLE_HEIGHT2,0xEEEF);	
	S->drawHLine(460,LEX-5,LEX-2,0xFFFF);
	S->drawHLine(461,LEX-5,LEX-5,0xFFFF);
	S->drawHLine(462,LEX-5,LEX-2,0xFFFF);
	S->drawHLine(463,LEX-5,LEX-5,0xFFFF);
	S->drawHLine(464,LEX-5,LEX-2,0xFFFF);
	}
}

}// IF CHECK SAMPLE
else
{
for (int s_ex=0;s_ex<512;s_ex++)
S->drawVLine(s_ex,458,CONSOLE_HEIGHT2,0x0004);
}

draw_sampled_wave=false;}
}

void NewWavy(void)
{
draw_sampled_wave=true;
sed_display_start=0;
sed_display_length=SampleNumSamples[ped_patsam][ped_split];
sed_range_start=0;
sed_range_end=0;
sed_range_mode=false;
Actualize_Wave_Ed(0);
}

void Draw_Sampled_Wave2(void)
{
if (draw_sampled_wave2)
{
	if(SampleType[ped_patsam][ped_split]>0) /* Any Sample Out There? */
	{
	int strober=SampleChannels[ped_patsam][ped_split]*2;
	int s_ey=458+(resty/strober);
	int s_ey2=s_ey+((resty/strober)*2);
	int	rcolor3=0x1234;
	
	if(SampleChannels[ped_patsam][ped_split]==1)
	{
	for (int s_ex=0;s_ex<512;s_ex++)
	{
	long const s_offset=(s_ex*sed_display_length)/512+sed_display_start;
	
	if (sp_Position[ped_track].half.first>s_offset)
		rcolor3=0xFFFF;
		else
		rcolor3=0x4321;

	S->drawVLine(s_ex,s_ey,s_ey,rcolor3);
	}
}// If

// STEREO DISPLAY
	
	if(SampleChannels[ped_patsam][ped_split]==2)
	{
	for (int s_ex=0;s_ex<512;s_ex++)
	{
	long const s_offset=(s_ex*sed_display_length)/512+sed_display_start;
		
	if (sp_Position[ped_track].half.first>s_offset)
		rcolor3=0xFFFF;
		else
		rcolor3=0x4321;

	S->drawVLine(s_ex,s_ey,s_ey,rcolor3);
	S->drawVLine(s_ex,s_ey2,s_ey2,rcolor3);}

}// If Stereo

}// IF CHECK SAMPLE

draw_sampled_wave2=false;}
}

void Draw_Sampled_Wave3(void)
{

if (draw_sampled_wave3)
	{
	if(SampleType[ped_patsam][ped_split]>0) /* Any Sample Out There? */
	{
	int strober=SampleChannels[ped_patsam][ped_split]*2;
	int s_ey=458+(resty/strober);
	int s_ey2=s_ey+((resty/strober)*2);
	int	rcolor3=0xCCC0;
	
	if(SampleChannels[ped_patsam][ped_split]==1)S->drawHLine(s_ey,0,511,rcolor3);	
	
	if(SampleChannels[ped_patsam][ped_split]==2){S->drawHLine(s_ey,0,511,rcolor3);S->drawHLine(s_ey2,0,511,rcolor3);}// If Stereo
	}// IF CHECK SAMPLE
	draw_sampled_wave3=false;
	}
}

void Actualize_Wave_Ed(char gode)
{

if(SampleType[ped_patsam][ped_split])
{
if (gode==3 || gode==0)
{
outlong(708,538,sed_display_start,10);
outlong(708,556,sed_display_length,12);
}

if (gode==5 || gode==0)
outlong(646,538,sed_range_start,10);

if (gode==4 || gode==5 || gode==0)
outlong(646,556,sed_range_end,11);

if (gode==0 || gode==6)mess_box("Full Vertical View...");

/* Sample Processing plant here */

// Sample editor functions code are implemented in this
// function.

/* Cut Sample */

if (gode==20)
{
guiDial(516,476,29,16,"Cut",100);

long cutsize=(sed_range_end-sed_range_start)+1;
long newsize=SampleNumSamples[ped_patsam][ped_split]-cutsize;

if (newsize!=0)
{
mess_box("Cutting sample...");
StopIfSp();
ss->stop();
Sleep(256);
char nc=SampleChannels[ped_patsam][ped_split];

short *Mono=(short *)malloc(newsize*2);

short *Stereo;
if (nc==2)Stereo=(short *)malloc(newsize*2);

long p_s=0;

if (sed_range_start>0)
{
for(long wao=0;wao<sed_range_start;wao++)
{
*(Mono+p_s)=*(RawSamples[ped_patsam][0][ped_split]+wao);

if (nc==2)
*(Stereo+p_s)=*(RawSamples[ped_patsam][1][ped_split]+wao);

p_s++;
}
}

if (SampleNumSamples[ped_patsam][ped_split]-sed_range_end>1)
{
for(int wao=sed_range_end+1;wao<SampleNumSamples[ped_patsam][ped_split];wao++)
{
*(Mono+p_s)=*(RawSamples[ped_patsam][0][ped_split]+wao);

if (nc==2)
*(Stereo+p_s)=*(RawSamples[ped_patsam][1][ped_split]+wao);

p_s++;
}
}

free(RawSamples[ped_patsam][0][ped_split]);
RawSamples[ped_patsam][0][ped_split]=Mono;

if (nc==2)
{
free(RawSamples[ped_patsam][1][ped_split]);
RawSamples[ped_patsam][1][ped_split]=Stereo;
}

SampleNumSamples[ped_patsam][ped_split]=newsize;

if(sed_display_length+sed_display_start>SampleNumSamples[ped_patsam][ped_split])
sed_display_length=SampleNumSamples[ped_patsam][ped_split]-sed_display_start;

sed_range_mode=false;
sed_range_start=0;
sed_range_end=0;
draw_sampled_wave=true;
outlong(708,538,sed_display_start,10);
outlong(708,556,sed_display_length,12);
outlong(646,538,sed_range_start,10);
outlong(646,556,sed_range_end,11);
mess_box("Selection cut ok.");

CheckLoops();
if(userscreen==2)Actualize_Sample_Ed(0,4);

ss->play();
}
else
{
mess_box("You cannot cut entire sample, use 'delete' on instrument instead.");
}
guiDial(516,476,29,16,"Cut",200);
}

if (gode==21)
{

guiDial(516,512,60,16,"DC Adjust",100);
mess_box("Calculating shifting-factor...");Sleep(100);

char nc=SampleChannels[ped_patsam][ped_split];

float l_shift=0;
float r_shift=0;

for(long wao=sed_range_start;wao<sed_range_end+1;wao++)
{
l_shift+=*(RawSamples[ped_patsam][0][ped_split]+wao);

if (nc==2)
r_shift+=*(RawSamples[ped_patsam][1][ped_split]+wao);
}

l_shift/=(sed_range_end+1)-sed_range_start;
r_shift/=(sed_range_end+1)-sed_range_start;

mess_box("Re-building waves...");Sleep(100);

for(wao=sed_range_start;wao<sed_range_end+1;wao++)
{
float bleak=*(RawSamples[ped_patsam][0][ped_split]+wao);
bleak-=l_shift;

if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;
*(RawSamples[ped_patsam][0][ped_split]+wao)=(short)bleak;

if (nc==2){
bleak=*(RawSamples[ped_patsam][1][ped_split]+wao);
bleak-=r_shift;

if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;
*(RawSamples[ped_patsam][1][ped_split]+wao)=(short)bleak;}
}

draw_sampled_wave=true;

guiDial(516,512,60,16,"DC Adjust",200);
mess_box("Selection calibrated...");
}

/* Normalizing */

if (gode==22)
{
guiDial(516,494,60,16,"Maximize",100);

mess_box("Searching highest peak...");Sleep(100);

char nc=SampleChannels[ped_patsam][ped_split];

float l_shift=0;

for(long wao=sed_range_start;wao<sed_range_end+1;wao++)
{
if (abs(*(RawSamples[ped_patsam][0][ped_split]+wao))>l_shift)
l_shift=*(RawSamples[ped_patsam][0][ped_split]+wao);

if (nc==2)
{
if (abs(*(RawSamples[ped_patsam][1][ped_split]+wao))>l_shift)
l_shift=*(RawSamples[ped_patsam][1][ped_split]+wao);
}

}

l_shift=32768.0f/l_shift;

mess_box("Amplifying...");Sleep(100);

for(wao=sed_range_start;wao<sed_range_end+1;wao++)
{
float bleak=*(RawSamples[ped_patsam][0][ped_split]+wao);
bleak*=l_shift;

if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;
*(RawSamples[ped_patsam][0][ped_split]+wao)=(short)bleak;

if (nc==2){
bleak=*(RawSamples[ped_patsam][1][ped_split]+wao);
bleak*=l_shift;

if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;
*(RawSamples[ped_patsam][1][ped_split]+wao)=(short)bleak;}
}

draw_sampled_wave=true;

guiDial(516,494,60,16,"Maximize",200);
mess_box("Selection maximized...");
}

/* Fade In */

if (gode==23)
{
guiDial(516,530,60,16,"Fade In",100);
mess_box("Fade In Selection...");Sleep(100);

char nc=SampleChannels[ped_patsam][ped_split];

float c_vol=0.0f;
float const coef_vol=1.0f/((sed_range_end+1)-sed_range_start);

for(long wao=sed_range_start;wao<sed_range_end+1;wao++)
{
float bleak=*(RawSamples[ped_patsam][0][ped_split]+wao);
bleak*=c_vol;
if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;

*(RawSamples[ped_patsam][0][ped_split]+wao)=(short)bleak;

if (nc==2){
bleak=*(RawSamples[ped_patsam][1][ped_split]+wao);
bleak*=c_vol;

if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;
*(RawSamples[ped_patsam][1][ped_split]+wao)=(short)bleak;}

c_vol+=coef_vol;
}

draw_sampled_wave=true;

guiDial(516,530,60,16,"Fade In",200);

mess_box("Finished...");
}

/* Fade Out */

if (gode==24)
{
guiDial(516,548,60,16,"Fade Out",100);
mess_box("Fade Out Selection...");Sleep(100);

char nc=SampleChannels[ped_patsam][ped_split];

float c_vol=1.0f;
float const coef_vol=1.0f/((sed_range_end+1)-sed_range_start);

for(long wao=sed_range_start;wao<sed_range_end+1;wao++)
{
float bleak=*(RawSamples[ped_patsam][0][ped_split]+wao);
bleak*=c_vol;
if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;

*(RawSamples[ped_patsam][0][ped_split]+wao)=(short)bleak;

if (nc==2){
bleak=*(RawSamples[ped_patsam][1][ped_split]+wao);
bleak*=c_vol;

if(bleak>32767)bleak=32767;
if(bleak<-32767)bleak=-32767;
*(RawSamples[ped_patsam][1][ped_split]+wao)=(short)bleak;}

c_vol-=coef_vol;
}

draw_sampled_wave=true;

guiDial(516,548,60,16,"Fade Out",200);

mess_box("Finished...");
}

/* Half */

if (gode==25)
{
guiDial(547,476,29,16,"Half",100);

mess_box("Halving Selection Volume...");Sleep(100);

char nc=SampleChannels[ped_patsam][ped_split];

float c_vol=0.5f;

for(long wao=sed_range_start;wao<sed_range_end+1;wao++)
{
float bleak=*(RawSamples[ped_patsam][0][ped_split]+wao);
bleak*=c_vol;

*(RawSamples[ped_patsam][0][ped_split]+wao)=(short)bleak;

if (nc==2){
bleak=*(RawSamples[ped_patsam][1][ped_split]+wao);
bleak*=c_vol;
*(RawSamples[ped_patsam][1][ped_split]+wao)=(short)bleak;}
}

draw_sampled_wave=true;
guiDial(547,476,29,16,"Half",200);
mess_box("Finished...");
}

/* SAMPLE EDITOR FUNCTIONS */
}
else
guiDial(646,538,122,34,"No Sample Loaded",108);
}

void guiDial3(int x,int y, int sx,int sy,const char* str,int brill)
{
	x++;
	y++;
	int tbrill1=brill-32;
	int tbrill2=brill/4;
	int tbrill3=brill/2;
	int x2=x+sx;
	int y2=y+sy;
	
	S->setColor(0,brill,brill);
	S->line(x, y,x2,y);
	
	S->setColor(0,tbrill1,tbrill1);
	S->line(x,y+2,x,y2);

	S->setColor(0,tbrill2,tbrill2);
	S->line(x+1,y2,x2,y2);
	S->line(x2,y+1,x2,y2);
	
	S->setColor(0,tbrill3,tbrill3);

	for (int filler=y+1;filler<y2;filler+=2)
	S->line(x+1,filler,x2-1,filler);	
	
	S->printXY(x+3, y+1, 0x00BBDDDD, str);	
}



void Initreverb()
{
	switch (DelayType)
	{
	case 0: // Deep Night II
	decays[0][0]= 20; decays[0][1]=  0;
	decays[1][0]=  0; decays[1][1]= 15;
	decays[2][0]= 15; decays[2][1]=  0;
	decays[3][0]=  0; decays[3][1]= 10;
	decays[4][0]=  9; decays[4][1]=  0;
	decays[5][0]=  1; decays[5][1]=  8;
	decays[6][0]=  8; decays[6][1]=  1;
	decays[7][0]=  1; decays[7][1]=  4;
	decays[8][0]=  4; decays[8][1]=  0;
	decays[9][0]=  1; decays[9][1]=  2;
	
	delays[0] = 1000;
	delays[1] = 1100;	
	delays[2] = 1200;
	delays[3] = 1300;
	delays[4] = 1400;
	delays[5] = 1800;
	delays[6] = 1900;
	delays[7] = 2000;
	delays[8] = 2100;
	delays[9] = 2200;

	num_echoes=10;
	break;

	case 1: // Deep Night III
	decays[0][0]=  7; decays[0][1]=  7;
	decays[1][0]=-13; decays[1][1]=-15;
	decays[2][0]= 25; decays[2][1]= 32;
	decays[3][0]= 31; decays[3][1]= 26;
	decays[4][0]= 20; decays[4][1]=-30;
	decays[5][0]= 28; decays[5][1]= 24;
	decays[6][0]=-21; decays[6][1]=-18;
	decays[7][0]= 18; decays[7][1]= 14;
	decays[8][0]=-13; decays[8][1]=-12;
	decays[9][0]=  9; decays[9][1]=  7;
	
	delays[0] = 1000;
	delays[1] = 1600;	
	delays[2] = 2100;
	delays[3] = 2400;
	delays[4] = 2290;
	delays[5] = 2350;
	delays[6] = 2400;
	delays[7] = 2500;
	delays[8] = 2680;
	delays[9] = 3410;

	num_echoes=10;
	break;

	case 2: // Deep Night III
	decays[0][0]=  1; decays[0][1]=  2;
	decays[1][0]=  1; decays[1][1]= -4;
	decays[2][0]=  9; decays[2][1]=  1;
	decays[3][0]= 12; decays[3][1]= 11;
	decays[4][0]= 22; decays[4][1]= -1;
	decays[5][0]=  1; decays[5][1]= 19;
	decays[6][0]= 15; decays[6][1]= -1;
	decays[7][0]=  1; decays[7][1]= 12;
	decays[8][0]=  7; decays[8][1]=- 1;
	decays[9][0]=  2; decays[9][1]=  3;
	
	delays[0] =  100;
	delays[1] =  200;	
	delays[2] =  300;
	delays[3] = 1000;
	delays[4] = 1190;
	delays[5] = 1250;
	delays[6] = 1300;
	delays[7] = 1400;
	delays[8] = 1580;
	delays[9] = 1610;

	num_echoes=10;
	break;

	
	case 3: // Deep Night II
	decays[0][0]= 22; decays[0][1]=  3;
	decays[1][0]=  5; decays[1][1]= 12;
	decays[2][0]= 12; decays[2][1]=  1;
	decays[3][0]=  3; decays[3][1]=  5;
	
	delays[0] = 2000;
	delays[1] = 4400;	
	delays[2] = 5000;
	delays[3] = 6200;
	
	num_echoes=4;
	break;

	case 4: // Deep Night II
	decays[0][0]= 11; decays[0][1]= 0;
	decays[1][0]= 0 ; decays[1][1]= 21;
	decays[2][0]= 31; decays[2][1]= 0;
	decays[3][0]= 0 ; decays[3][1]= 41;
	
	delays[0] = 3012;
	delays[1] = 4012;
	delays[2] = 4022;
	delays[3] = 5232;
	
	num_echoes=4;
	break;

	case 5: // Deep Night III
	decays[0][0]=  7; decays[0][1]=  7;
	decays[1][0]=-13; decays[1][1]=-15;
	decays[2][0]= 25; decays[2][1]= 32;
	decays[3][0]= 31; decays[3][1]= 26;
	decays[4][0]= 20; decays[4][1]=-30;
	decays[5][0]= 28; decays[5][1]= 24;
	decays[6][0]=-21; decays[6][1]=-18;
	decays[7][0]= 18; decays[7][1]= 14;
	decays[8][0]=-13; decays[8][1]=-12;
	decays[9][0]=  9; decays[9][1]=  7;
	
	delays[0] = 20;
	delays[1] = 600;	
	delays[2] = 100;
	delays[3] = 400;
	delays[4] = 290;
	delays[5] = 1350;
	delays[6] = 400;
	delays[7] = 1500;
	delays[8] = 1680;
	delays[9] = 1410;

	num_echoes=10;
	break;

	case 6: // Deep Night III
	decays[0][0]=  7; decays[0][1]=  7;
	decays[1][0]=-13; decays[1][1]=-15;
	decays[2][0]= 25; decays[2][1]= 32;
	decays[3][0]= 31; decays[3][1]= 26;
	decays[4][0]= 20; decays[4][1]=-30;
	decays[5][0]= 28; decays[5][1]= 24;
	decays[6][0]=-21; decays[6][1]=-18;
	decays[7][0]= 18; decays[7][1]= 14;
	decays[8][0]=-13; decays[8][1]=-12;
	decays[9][0]= 12; decays[9][1]=  8;
	
	delays[0] = 20;
	delays[1] = 600;	
	delays[2] = 700;
	delays[3] = 800;
	delays[4] = 990;
	delays[5] = 1350;
	delays[6] = 1400;
	delays[7] = 1500;
	delays[8] = 1680;
	delays[9] = 1910;

	num_echoes=10;
	break;

}

	for (int i=0;i<num_echoes;i++)
	{
	int mlrw=99999-(delay_time+delays[i])*4;
	if (mlrw<0)mlrw=0;

	decays[i][0]=decays[i][0]*0.015625f;
	decays[i][1]=decays[i][1]*0.015625f;
	
	counters[i]=mlrw;
	}
	rev_counter=99999;
}

float sign(float entry)
{
	if (entry>=0.0f)
	return 1.0f;
	else
	return -1.0f;
}
void Compressor_work(void)
{
	if (compressor)
	{
	
	float l_rout=0;
	float r_rout=0;

	// COMB FILTER:

	for (int i=0;i<num_echoes;i++)
	{
	
		delay_left_buffer[i][rev_counter]  = (delay_left_final + delay_right_final + delay_left_buffer [i][counters[i]])*decays[i][0];
		l_rout += delay_left_buffer[i][counters[i]];
		if(++counters[i]>99999)counters[i]=0;
	}
	
	if(++rev_counter>99999)rev_counter=0;
	
	// STEREO ALLPASS FILTER:

	float temp1;
	
	temp1 = LFP_L.fWork(l_rout++,REVERBFILTER);
	
	l_rout  = (-Feedback * temp1)+allBuffer_L[delayedCounter];
   	allBuffer_L[currentCounter]=(l_rout*Feedback)+temp1;
	
	temp1 = l_rout;
	l_rout  = (-Feedback2 * temp1)+allBuffer_L2[delayedCounter2];
   	allBuffer_L2[currentCounter]=(l_rout*Feedback2)+temp1;
	
	temp1 = l_rout;
	l_rout  = (-Feedback3 * temp1)+allBuffer_L3[delayedCounter3];
   	allBuffer_L3[currentCounter]=(l_rout*Feedback3)+temp1;
		
	temp1 = l_rout;
	l_rout  = (-Feedback4 * temp1)+allBuffer_L4[delayedCounter4];
 	allBuffer_L4[currentCounter]=(l_rout*Feedback4)+temp1;
	
	temp1 = l_rout;
	l_rout  = (-Feedback5 * temp1)+allBuffer_L5[delayedCounter5];
 	allBuffer_L5[currentCounter]=(l_rout*Feedback5)+temp1;
	
	// Stereo diffussion

	right_float+=l_rout;
	
	temp1 = l_rout;
	l_rout  = (-Feedback6 * temp1)+allBuffer_L6[delayedCounter6];
 	allBuffer_L6[currentCounter]=(l_rout*Feedback6)+temp1;
	
	// Updating delayedCointers
	
	if(++delayedCounter>5759)delayedCounter=0;
	if(++delayedCounter2>5759)delayedCounter2=0;
	if(++delayedCounter3>5759)delayedCounter3=0;
	if(++delayedCounter4>5759)delayedCounter4=0;
	if(++delayedCounter5>5759)delayedCounter5=0;
	if(++delayedCounter6>5759)delayedCounter6=0;

	// Updating currentCointer
	currentCounter++;
	if(currentCounter>5759)currentCounter=0;

	left_float+=l_rout;
	}
}

void allPassInit(float miliSecs)
{
currentCounter=5759;
delayedCounter=5759-int(miliSecs*44.1f);
delayedCounter2=5759-int(miliSecs*50.1f);
delayedCounter3=5759-int(miliSecs*60.1f);
delayedCounter4=5759-int(miliSecs*70.1f);
delayedCounter5=5759-int(miliSecs*73.1f);
delayedCounter6=5759-int(miliSecs*79.1f);

if (delayedCounter<0)delayedCounter=0;
if (delayedCounter2<0)delayedCounter2=0;
if (delayedCounter3<0)delayedCounter3=0;
if (delayedCounter4<0)delayedCounter4=0;
if (delayedCounter5<0)delayedCounter5=0;
if (delayedCounter6<0)delayedCounter6=0;

for (int yb=0;yb<5760;yb++)
{
allBuffer_L[yb]=0.0f;
allBuffer_L2[yb]=0.0f;
allBuffer_L3[yb]=0.0f;
allBuffer_L4[yb]=0.0f;
allBuffer_L5[yb]=0.0f;
allBuffer_L6[yb]=0.0f;
}

}

void CheckLoops(void)
{
if (LoopStart[ped_patsam][ped_split]<0)LoopStart[ped_patsam][ped_split]=0;
if (LoopStart[ped_patsam][ped_split]>=SampleNumSamples[ped_patsam][ped_split])LoopStart[ped_patsam][ped_split]=SampleNumSamples[ped_patsam][ped_split]-1;
if (LoopEnd[ped_patsam][ped_split]<0)LoopEnd[ped_patsam][ped_split]=0;
if (LoopEnd[ped_patsam][ped_split]>=SampleNumSamples[ped_patsam][ped_split])LoopEnd[ped_patsam][ped_split]=SampleNumSamples[ped_patsam][ped_split]-1;

if (LoopStart[ped_patsam][ped_split]==LoopEnd[ped_patsam][ped_split])
{
LoopStart[ped_patsam][ped_split]=0;
LoopEnd[ped_patsam][ped_split]=SampleNumSamples[ped_patsam][ped_split]-1;
}

}

void Sp_Player2(void)
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
			
				/* MidiController commands */
				if (pl_pan_row==144 && c_midiout!=-1 && pl_eff_row<128)
				midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ct]) | (pl_eff_row << 8) | (pl_dat_row << 16));
		
				if (pl_eff_row==128 && c_midiout!=-1 && pl_dat_row<128)
				midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ct]) | (0 << 8) | (pl_dat_row << 16));
			
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
					if (sp_Stage[ct]==1)
					sp_Stage[ct]=2;
					noteoff303(ct);
				}

			}// For loop
			Go303();	// Let's shake the 303s!
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
			sp_Tvol[c]=0.0f;	
			if (sp_Cvol[c]<0.01)sp_Stage[c]=0;
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
			*(Player_WL[c]+Currentpointer-1),
			*(Player_WL[c]+Currentpointer),
			*(Player_WL[c]+Currentpointer+1),
			*(Player_WL[c]+Currentpointer+2),res_dec,Currentpointer,Rns[c])*sp_Cvol[c]*Player_SV[c];
	
		if (Player_SC[c]==2)
		{
		grown=true;
		currsygnal2=Resampler.Work(
			*(Player_WR[c]+Currentpointer-1),
			*(Player_WR[c]+Currentpointer),
			*(Player_WR[c]+Currentpointer+1),
			*(Player_WR[c]+Currentpointer+2),res_dec,Currentpointer,Rns[c])*sp_Cvol[c]*Player_SV[c];
		}
		

		// End of Interpolation algo

		sp_Position[c].absolu+=Vstep1[c];
		if (Player_LT[c]==1 && sp_Position[c].half.first>=Player_LE[c])sp_Position[c].half.first-=Player_LL[c];
		if (Player_LT[c]==0 && sp_Position[c].half.first>=Player_NS[c])sp_Stage[c]=0;
		}//sp!!0

		if(track3031==c)
		{
			gotsome=true;
			currsygnal+=tb303engine[0].tbGetSample();
		}

		if(track3032==c)
		{
			currsygnal+=tb303engine[1].tbGetSample();
			gotsome=true;
		}

		if(gotsome)
		{
		if (FType[c]!=4) /* Track filter actived */
		{
		float const dfi = TCut[c]-CCut[c];
		if (dfi<-1.0 || dfi>1.0)
			CCut[c]+=dfi*ICut[c];
	
		if (FType[c]<4){
		gco=f2i(ApplyLfo(CCut[c]-ramper[c],c));
		
		ramper[c]+=Player_FD[c]*(float)gco*0.015625f;
		
		coef[0]=coeftab[0][gco][FRez[c]][FType[c]];
		coef[1]=coeftab[1][gco][FRez[c]][FType[c]];
		coef[2]=coeftab[2][gco][FRez[c]][FType[c]];
		coef[3]=coeftab[3][gco][FRez[c]][FType[c]];
		coef[4]=coeftab[4][gco][FRez[c]][FType[c]];
		currsygnal=Filter(currsygnal,c);
		}
		else
		{
		float const realcut=ApplyLfo(CCut[c]-ramper[c],c);

		ramper[c]+=Player_FD[c]*realcut*0.015625f;

		switch(FType[c])
		{
		case 5:
		currsygnal=filter2p(c,currsygnal,realcut,(float)FRez[c]);
		break;
		
		case 6:
		currsygnal=filter2p(c,currsygnal,realcut,(float)FRez[c]);
		currsygnal=filter2p24d(c,currsygnal,realcut,(float)FRez[c]);
		break;
		
		case 7:
		currsygnal=filter2p(c,currsygnal,realcut,(float)FRez[c]);
		currsygnal2=filter2p24d(c,currsygnal2,realcut,(float)FRez[c]);
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
		currsygnal=filterhp(c,currsygnal,realcut,(float)FRez[c]);
		break;

		case 22:
		currsygnal=filterhp(c,currsygnal,realcut,(float)FRez[c]);
		currsygnal2=filterhp2(c,currsygnal2,realcut,(float)FRez[c]);
		break;

		case 23:
		currsygnal=filterhp(c,currsygnal,realcut,(float)FRez[c]);
		currsygnal=filterhp2(c,currsygnal,realcut,(float)FRez[c]);
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
	
	} //TRACKSTATE CLOSE
	} // GOTSOME
	}// Fortracker

}

void Sp_Player3(void) /* Lowest Quality Player , Standard Mod Properties.*/
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
			
				/* MidiController commands */
				if (pl_pan_row==144 && c_midiout!=-1 && pl_eff_row<128)
				midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ct]) | (pl_eff_row << 8) | (pl_dat_row << 16));
				
				if (pl_eff_row==128 && c_midiout!=-1 && pl_dat_row<128)
				midiOutShortMsg(midiout_handle, (176+TRACKMIDICHANNEL[ct]) | (0 << 8) | (pl_dat_row << 16));
			
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
					if (sp_Stage[ct]==1)
					sp_Stage[ct]=2;
					noteoff303(ct);
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

	for (char c=0;c<Songtracks;c++)
	{
	bool gotsome=false;
	grown=false;
	currsygnal=0;
	currsygnal2=0;
	
	if (sp_Stage[c]==2)
		{
		// Note Stop
			sp_Tvol[c]=0.0f;
			
			if (sp_Cvol[c]<0.01)sp_Stage[c]=0;
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
			*(Player_WL[c]+Currentpointer-1),
			*(Player_WL[c]+Currentpointer),
			*(Player_WL[c]+Currentpointer+1),
			*(Player_WL[c]+Currentpointer+2),res_dec,Currentpointer,Rns[c])*sp_Cvol[c]*Player_SV[c];
	
		if (Player_SC[c]==2)
		{
		grown=true;
		currsygnal2=Resampler.Work(
			*(Player_WR[c]+Currentpointer-1),
			*(Player_WR[c]+Currentpointer),
			*(Player_WR[c]+Currentpointer+1),
			*(Player_WR[c]+Currentpointer+2),res_dec,Currentpointer,Rns[c])*sp_Cvol[c]*Player_SV[c];
		}
		
		// End of Interpolation algo

		sp_Position[c].absolu+=Vstep1[c];

		if (Player_LT[c]==1 && sp_Position[c].half.first>=Player_LE[c])sp_Position[c].half.first-=Player_LL[c];
		if (Player_LT[c]==0 && sp_Position[c].half.first>=Player_NS[c])sp_Stage[c]=0;
	
	} // SP
	
	if(track3031==c)
	{
		gotsome=true;
		currsygnal+=tb303engine[0].tbGetSample();
	}

	if(track3032==c)
	{
		gotsome=true;
		currsygnal+=tb303engine[1].tbGetSample();
	}

	if(gotsome)
	{
	if (!grown)currsygnal2=currsygnal;

	currsygnal*=LVol[c];
	currsygnal2*=RVol[c];
	
	if(!TRACKSTATE[c])// Is the track mute or unmute ???
	{
	left_float+=currsygnal;
	right_float+=currsygnal2;
	}// TRACKSTATE
	}// GOTSOME
	
	}// Fortracker

}


void Go303(void)
{
if (tb303engine[0].tbPattern!=255)
{
	tb303engine[0].tbNoteOn(tb303[0].tone[tb303engine[0].tbPattern][tb303engine[0].tbLine],&tb303[0]);

	tb303engine[0].tbLine++;
	
	if(tb303engine[0].tbLine==tb303[0].patternlength[tb303engine[0].tbPattern])tb303engine[0].tbLine=0;
}

if (tb303engine[1].tbPattern!=255)
{
	tb303engine[1].tbNoteOn(tb303[1].tone[tb303engine[1].tbPattern][tb303engine[1].tbLine],&tb303[1]);

	tb303engine[1].tbLine++;
	
	if(tb303engine[1].tbLine==tb303[1].patternlength[tb303engine[1].tbPattern])tb303engine[1].tbLine=0;	
}
}


void Fire303(unsigned char number,char unit)
{
	tb303engine[unit].tbLine=0;
	
	switch(number)
	{
	case 0x00:tb303engine[unit].tbPattern=tb303[unit].selectedpattern;break;
	case 0xA1:tb303engine[unit].tbPattern=0;break;
	case 0xA2:tb303engine[unit].tbPattern=1;break;
	case 0xA3:tb303engine[unit].tbPattern=2;break;
	case 0xA4:tb303engine[unit].tbPattern=3;break;
	case 0xA5:tb303engine[unit].tbPattern=4;break;
	case 0xA6:tb303engine[unit].tbPattern=5;break;
	case 0xA7:tb303engine[unit].tbPattern=6;break;
	case 0xA8:tb303engine[unit].tbPattern=7;break;
	case 0xB1:tb303engine[unit].tbPattern=8;break;
	case 0xB2:tb303engine[unit].tbPattern=9;break;
	case 0xB3:tb303engine[unit].tbPattern=10;break;
	case 0xB4:tb303engine[unit].tbPattern=11;break;
	case 0xB5:tb303engine[unit].tbPattern=12;break;
	case 0xB6:tb303engine[unit].tbPattern=13;break;
	case 0xB7:tb303engine[unit].tbPattern=14;break;
	case 0xB8:tb303engine[unit].tbPattern=15;break;
	case 0xC1:tb303engine[unit].tbPattern=16;break;
	case 0xC2:tb303engine[unit].tbPattern=17;break;
	case 0xC3:tb303engine[unit].tbPattern=18;break;
	case 0xC4:tb303engine[unit].tbPattern=19;break;
	case 0xC5:tb303engine[unit].tbPattern=20;break;
	case 0xC6:tb303engine[unit].tbPattern=21;break;
	case 0xC7:tb303engine[unit].tbPattern=22;break;
	case 0xC8:tb303engine[unit].tbPattern=23;break;
	case 0xD1:tb303engine[unit].tbPattern=24;break;
	case 0xD2:tb303engine[unit].tbPattern=25;break;
	case 0xD3:tb303engine[unit].tbPattern=26;break;
	case 0xD4:tb303engine[unit].tbPattern=27;break;
	case 0xD5:tb303engine[unit].tbPattern=28;break;
	case 0xD6:tb303engine[unit].tbPattern=29;break;
	case 0xD7:tb303engine[unit].tbPattern=30;break;
	case 0xD8:tb303engine[unit].tbPattern=31;break;

	default: /* No Fire */
	tb303engine[unit].tbLine=255;
	break;

	}
}

void noteoff303(char strack)
{
if(strack==track3031)
{
tb303engine[0].tbLine=255;
tb303engine[0].tbPattern=255;
track3031=255;
tb303engine[0].tbBuf0=0.0f;
tb303engine[0].tbBuf1=0.0f;
}

if(strack==track3032)
{
tb303engine[1].tbLine=255;
tb303engine[1].tbPattern=255;
track3032=255;
tb303engine[1].tbBuf0=0.0f;
tb303engine[1].tbBuf1=0.0f;
}
}

void live303(int pltr_eff_row,int pltr_dat_row)
{
	// 303 Pattern commands Parameter changers...

	switch(pltr_eff_row)
	{
	case 51:tb303[0].cutoff=pltr_dat_row/2;break;
	case 52:tb303[1].cutoff=pltr_dat_row/2;break;
	case 53:tb303[0].resonance=pltr_dat_row/2;break;
	case 54:tb303[1].resonance=pltr_dat_row/2;break;
	case 55:tb303[0].envmod=pltr_dat_row/2;break;
	case 56:tb303[1].envmod=pltr_dat_row/2;break;
	case 57:tb303[0].decay=pltr_dat_row/2;break;
	case 58:tb303[1].decay=pltr_dat_row/2;break;
	case 59:tb303[0].tune=pltr_dat_row/2;break;
	case 60:tb303[1].tune=pltr_dat_row/2;break;
	}
}

void Letter(int x,int y,char ltr,int ys,int y2)
{
	switch(ltr)
	{
	case 0: S->copy(PFONT,x,y,72 ,ys,79,y2);break;
	case 1: S->copy(PFONT,x,y,80 ,ys,87,y2);break;
	case 2: S->copy(PFONT,x,y,88 ,ys,95,y2);break;
	case 3: S->copy(PFONT,x,y,96 ,ys,103,y2);break;
	case 4: S->copy(PFONT,x,y,104,ys,111,y2);break;
	case 5: S->copy(PFONT,x,y,112,ys,119,y2);break;
	case 6: S->copy(PFONT,x,y,120,ys,127,y2);break;
	case 7: S->copy(PFONT,x,y,128,ys,135,y2);break;
	case 8: S->copy(PFONT,x,y,136,ys,143,y2);break;
	case 9: S->copy(PFONT,x,y,144,ys,151,y2);break;
	case 10: S->copy(PFONT,x,y,0,ys,7,y2);break; // A
	case 11: S->copy(PFONT,x,y,8,ys,15,y2);break;// B
	case 12: S->copy(PFONT,x,y,16,ys,23,y2);break;// C
	case 13: S->copy(PFONT,x,y,24,ys,31,y2);break;// D
	case 14: S->copy(PFONT,x,y,32,ys,39,y2);break;// E
	case 15: S->copy(PFONT,x,y,40,ys,47,y2);break;// F
	case 16: S->copy(PFONT,x,y,48,ys,55,y2);break;// G
	case 17: S->copy(PFONT,x,y,64,ys,71,y2);break; // #
	case 18: S->copy(PFONT,x,y,176,ys,183,y2);break; // -
	case 19: S->copy(PFONT,x,y,152,ys,175,y2);break; // Off
	case 20: S->copy(PFONT,x,y,56,ys,63,y2);break; // Blank
	case 21: S->copy(PFONT,x,y,184,ys,191,y2);break; // .
	case 22: S->copy(PFONT,x,y,0,104,103,111);break; // .
	case 23: S->copy(PFONT,x,y,39,113,57,119);break; // ON!
	case 24: S->copy(PFONT,x,y,58,113,76,119);break; // OFF
	case 25: S->copy(PFONT,x,y,0,113,18,119);break; // MUTE!
	case 26: S->copy(PFONT,x,y,19,113,37,119);break; // UNMUTE
	case 27: S->copy(PFONT,x,y,78,113,109,119);break; // EDIT ON!
	case 28: S->copy(PFONT,x,y,111,113,142,119);break; // EDIT OFF!

	}
}

void blitnote(int x,int y,int note, int y1,int y2)
{
switch(note)
{

case 120:Letter(x,y,19,y1,y2);break;
case 121:Letter(x,y,18,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,18,y1,y2);break;

case 0: Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 1: Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 2: Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 3: Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 4: Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 5: Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 6: Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 7: Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 8: Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 9: Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 10:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 11:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,0,y1,y2);break;
case 12:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 13:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 14:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 15:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 16:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 17:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 18:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 19:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 20:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 21:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 22:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 23:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,1,y1,y2);break;
case 24:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 25:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 26:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 27:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 28:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 29:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 30:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 31:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 32:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 33:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 34:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 35:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,2,y1,y2);break;
case 36:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 37:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 38:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 39:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 40:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 41:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 42:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 43:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 44:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 45:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 46:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 47:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,3,y1,y2);break;
case 48:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 49:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 50:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 51:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 52:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 53:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 54:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 55:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 56:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 57:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 58:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 59:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,4,y1,y2);break;
case 60:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 61:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 62:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 63:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 64:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 65:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 66:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 67:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 68:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 69:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 70:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 71:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,5,y1,y2);break;
case 72:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 73:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 74:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 75:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 76:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 77:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 78:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 79:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 80:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 81:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 82:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 83:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,6,y1,y2);break;
case 84:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 85:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 86:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 87:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 88:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 89:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 90:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 91:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 92:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 93:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 94:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 95:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,7,y1,y2);break;
case 96:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 97:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 98:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 99:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 100:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 101:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 102:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 103:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 104:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 105:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 106:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 107:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,8,y1,y2);break;
case 108:Letter(x,y,12,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 109:Letter(x,y,12,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 110:Letter(x,y,13,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 111:Letter(x,y,13,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 112:Letter(x,y,14,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 113:Letter(x,y,15,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 114:Letter(x,y,15,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 115:Letter(x,y,16,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 116:Letter(x,y,16,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 117:Letter(x,y,10,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 118:Letter(x,y,10,y1,y2);Letter(x+8,y,17,y1,y2);Letter(x+16,y,9,y1,y2);break;
case 119:Letter(x,y,11,y1,y2);Letter(x+8,y,18,y1,y2);Letter(x+16,y,9,y1,y2);break;
}
}

void RandSynth()
{
PARASynth[ped_patsam].osc1_waveform=rand()/8192;
PARASynth[ped_patsam].osc2_waveform=rand()/8192;
PARASynth[ped_patsam].vcf_type=rand()/10923;
PARASynth[ped_patsam].osc1_pw=rand()/64;
PARASynth[ped_patsam].osc2_pw=rand()/64;
PARASynth[ped_patsam].osc2_finetune=rand()/256;
PARASynth[ped_patsam].vcf_cutoff=rand()/256;
PARASynth[ped_patsam].vcf_resonance=rand()/256;
PARASynth[ped_patsam].env1_attack=rand();
PARASynth[ped_patsam].env1_decay=rand();
PARASynth[ped_patsam].env1_sustain=rand()/256;
PARASynth[ped_patsam].env1_release=rand();
PARASynth[ped_patsam].env2_attack=rand();
PARASynth[ped_patsam].env2_decay=rand();
PARASynth[ped_patsam].env2_sustain=rand()/256;
PARASynth[ped_patsam].env2_release=rand();
PARASynth[ped_patsam].lfo1_period=rand()/16;
PARASynth[ped_patsam].lfo2_period=rand()/16;
PARASynth[ped_patsam].lfo1_osc1_pw=rand()/256;
PARASynth[ped_patsam].lfo1_osc2_pw=rand()/256;
PARASynth[ped_patsam].lfo1_osc1_volume=rand()/256;
PARASynth[ped_patsam].lfo1_osc2_volume=rand()/256;
PARASynth[ped_patsam].lfo1_vcf_cutoff=rand()/256;
PARASynth[ped_patsam].lfo1_vcf_resonance=rand()/256;
PARASynth[ped_patsam].lfo2_osc1_pw=rand()/256;
PARASynth[ped_patsam].lfo2_osc2_pw=rand()/256;
PARASynth[ped_patsam].lfo2_osc1_volume=rand()/256;
PARASynth[ped_patsam].lfo2_osc2_volume=rand()/256;
PARASynth[ped_patsam].lfo2_vcf_cutoff=rand()/256;
PARASynth[ped_patsam].lfo2_vcf_resonance=rand()/256;
PARASynth[ped_patsam].env1_osc1_pw=rand()/256;
PARASynth[ped_patsam].env1_osc2_pw=rand()/256;
PARASynth[ped_patsam].env1_osc1_volume=rand()/256;
PARASynth[ped_patsam].env1_osc2_volume=rand()/256;
PARASynth[ped_patsam].env1_vcf_cutoff=rand()/256;
PARASynth[ped_patsam].env1_vcf_resonance=rand()/256;
PARASynth[ped_patsam].env2_osc1_pw=rand()/256;
PARASynth[ped_patsam].env2_osc2_pw=rand()/256;
PARASynth[ped_patsam].env2_osc1_volume=rand()/256;
PARASynth[ped_patsam].env2_osc2_volume=rand()/256;
PARASynth[ped_patsam].env2_vcf_cutoff=rand()/256;
PARASynth[ped_patsam].env2_vcf_resonance=rand()/256;
PARASynth[ped_patsam].osc3_volume=rand()/256;
PARASynth[ped_patsam].ptc_glide=rand()/256;
PARASynth[ped_patsam].glb_volume=100;


if(rand()>16384)
{
PARASynth[ped_patsam].osc2_detune=rand()/256;
PARASynth[ped_patsam].lfo1_osc1_pitch=rand()/256;
PARASynth[ped_patsam].lfo1_osc2_pitch=rand()/256;
PARASynth[ped_patsam].lfo2_osc1_pitch=rand()/256;
PARASynth[ped_patsam].lfo2_osc2_pitch=rand()/256;
PARASynth[ped_patsam].env1_osc1_pitch=rand()/256;
PARASynth[ped_patsam].env1_osc2_pitch=rand()/256;
PARASynth[ped_patsam].env2_osc1_pitch=rand()/256;
PARASynth[ped_patsam].env2_osc2_pitch=rand()/256;
}
else
{
PARASynth[ped_patsam].osc2_detune=64;
PARASynth[ped_patsam].lfo1_osc1_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].lfo1_osc2_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].lfo2_osc1_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].lfo2_osc2_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].env1_osc1_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].env1_osc2_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].env2_osc1_pitch=64+((rand()-16384)/10923);
PARASynth[ped_patsam].env2_osc2_pitch=64+((rand()-16384)/10923);
}

if (userscreen==6)Actualize_Midi_Ed(1);
}
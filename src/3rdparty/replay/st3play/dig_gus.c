/***********************************************************************
 **
 **  Gravis Ultrasound driver
 **
 ** 8bitbubsy: This driver (as with original ST3) is buggy when close to
 **            all (16) ST3 PCM voices are used. :(
 **
 ***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dig.h"
#include "mixer/gus_gf1.h"

static uint8_t stchannelpan[ACHANNELS]; // st channels pan settings
static int8_t voiceused[32];
static uint8_t channeltrig[32];
static uint8_t somevoice;
static uint8_t voicetry; // which voice to try next
static uint8_t g_maxvoices;

static const uint16_t gusvoltable[64+1] =
{
	 4096,36848,40944,43008,45040,46080,47104,48128,49136,49664,50176,
	50688,51200,51712,52224,52736,53232,53504,53760,54016,54272,54528,
	54784,55040,55296,55552,55808,56064,56320,56576,56832,57088,57328,
	57472,57600,57728,57856,57984,58112,58240,58368,58496,58624,58752,
	58880,59008,59136,59264,59392,59520,59648,59776,59904,60032,60160,
	60288,60416,60544,60672,60800,60928,61056,61184,61312,

	256 // 8bb: non-audible gain, aka. "morezero"
};

static void shutupgus(void)
{
	for (int32_t i = 0; i < g_maxvoices; i++)
	{
		GUS_VoiceSelect((uint8_t)i);
		GUS_SetVoiceCtrl(0b00000011);
		GUS_SetCurrVolume(0);
		GUS_SetBalance(7);
		GUS_SetVolumeCtrl(0b00000011);

		voiceused[i] = 0;
	}
}

void gcmd_inittables(void)
{
	shutupgus();

	for (int32_t i = 0; i < 32; i++)
	{
		voiceused[i] = 0;
		channeltrig[i] = 255;
	}

	somevoice = voicetry = 0;
}

void gcmd_setvoices(uint8_t numVoices)
{
	g_maxvoices = numVoices;
	shutupgus();

	for (int32_t i = 0; i < 32; i++)
	{
		GUS_VoiceSelect((uint8_t)i);
		GUS_SetVoiceCtrl(3);
		GUS_SetVolumeCtrl(3);
	}
}

void gcmd_setstereo(void) // sets for 16 first dma channels
{
	memset(stchannelpan, 0, sizeof (stchannelpan));
	for (int32_t i = 0; i < 16; i++)
	{
		uint8_t pan;
		if (song.stereomode)
		{
			if (i < 8)
				pan = 0x3;
			else
				pan = 0xC;
		}
		else
		{
			pan = 7;
		}

		stchannelpan[i] = pan;

		GUS_VoiceSelect((uint8_t)i);
		GUS_SetBalance(pan);
	}
}

static void setvolslide(zchn_t *ch, uint8_t currVol, uint8_t targetVol)
{
	if (currVol != targetVol)
	{
		ch->m_oldvol = targetVol;

		const uint16_t currLogVol = gusvoltable[currVol];
		const uint16_t targetLogVol = gusvoltable[targetVol];

		GUS_SetVolumeRate(15);
		GUS_SetCurrVolume(currLogVol);

		if (currLogVol < targetLogVol) // slide up
		{
			GUS_SetStartVolume(currLogVol >> 8);
			GUS_SetEndVolume(targetLogVol >> 8);
			GUS_SetVolumeCtrl(0); // increasing ramp
		}
		else // slide down
		{
			GUS_SetStartVolume(targetLogVol >> 8);
			GUS_SetEndVolume(currLogVol >> 8);
			GUS_SetVolumeCtrl(64); // decreasing ramp
		}
	}
}

static void freevoices(void) // finds free voices
{
	// try first to find voices with almost zero volume or stopped
	bool voicesfreed = false;
	for (int32_t i = 0; i < g_maxvoices; i++)
	{
		if (voiceused[i] > 0)
			continue; // playing, don't do

		GUS_VoiceSelect((uint8_t)i);
		if (GUS_GetVoiceCtrl() & 1) // voice stopped
		{
			// voice volume about zero. Shut down
			voiceused[i] = 0;
			voicesfreed = true;
		}
	}

	if (voicesfreed)
		return; // voices freed! done.

	// find voice with lowest notused count - try finding voices that are sliding down
	int8_t voice = -1, notusedcount = -103;
	for (int32_t i = 0; i < g_maxvoices; i++)
	{
		if (voiceused[i] <= 0 && voiceused[i] >= notusedcount)
		{
			voice = (int8_t)i;
			notusedcount = voiceused[i];
		}
	}

	if (voice == -1)
	{
		// WEIRDFATAL. Not found, force one
		if (++somevoice >= g_maxvoices)
			somevoice = 0;

		voiceused[somevoice] = 0;
	}
	else
	{
		voiceused[voice] = 0;
	}
}

void gcmd_update(zchn_t *ch)
{
	if (ch == NULL) // 8bb: handle GUS triggers
	{
		for (int32_t i = 0; i < 32; i++)
		{
			if (voiceused[i] < 0)
				voiceused[i]++;

			if (channeltrig[i] != 255)
			{
				GUS_VoiceSelect((uint8_t)i);
				GUS_SetVoiceCtrl(channeltrig[i]);
				channeltrig[i] = 255;
			}
		}

		return;
	}

	if (ch->aguschannel >= 0)
	{
		GUS_VoiceSelect(ch->aguschannel);

		if (ch->m_oldpos == ch->m_pos)
		{
			// update m_pos & m_oldpos

			if (ch->m_speed != 0)
			{
				uint32_t pos = (uint32_t)(GUS_GetCurrAddress() - ch->m_base);
				if (pos >= 65536)
					pos = 0;

				ch->m_oldpos = ch->m_pos = pos;
			}

			// hz
			if (g_maxvoices == 16)
				GUS_SetFrequency((uint16_t)(ch->m_speed >> 6));
			else if (g_maxvoices == 24)
				GUS_SetFrequency((uint16_t)((ch->m_speed + ch->m_speed) / 85));
			else if (g_maxvoices == 32)
				GUS_SetFrequency((uint16_t)(ch->m_speed >> 5));
			else
				GUS_SetFrequency((uint16_t)ch->m_speed);

			setvolslide(ch, ch->m_oldvol, ch->m_vol);

			return;
		}

		// slide old channel to zero
		setvolslide(ch, ch->m_oldvol, 64); // 8bb: 64 (aka. "morezero") is quieter than 0 in gusvoltable

		// flip channel
		voiceused[ch->aguschannel] = -4; // shutting down
	}

	// 8bb: find available GUS voice
	bool nochannel = true;
	while (nochannel)
	{
		ch->m_oldpos = ch->m_pos;
		for (int32_t i = 0; i < g_maxvoices; i++)
		{
			if (++voicetry >= g_maxvoices)
				voicetry = 0;

			if (voiceused[voicetry] == 0)
			{
				// 8bb: free GUS voice found!
				nochannel = false;
				break;
			}
		}

		if (nochannel)
			freevoices(); // changes setvoice
	}

	if ((uint16_t)ch->m_end == 0) // 8bb: test lower word here
	{
		voiceused[voicetry] = 0;
		ch->aguschannel = -1;
		return;
	}

	// 8bb: assign channel

	voiceused[voicetry] = 1;
	ch->aguschannel = voicetry;
	GUS_VoiceSelect(voicetry);

	if (ch->m_end == 0)
	{
		// channel quiet, don't mark it used
		
		voiceused[ch->aguschannel] = -1;
		ch->aguschannel = -1;
		ch->m_oldvol = ch->m_vol = 0;
	}

	// clear volume & stop voice
	GUS_SetVoiceCtrl(0b00000010); // stop
	GUS_SetCurrVolume(0);

	// set pan
	if (song.stereomode && ch->apanpos >= 0xF0)
		GUS_SetBalance(ch->apanpos & 0x0F);
	else
		GUS_SetBalance(stchannelpan[ch->channelnum]);

	//GUS_SetVoiceCtrl(0b00000010); // stop (8bb: Again. Why?)

	GUS_SetEndAddress(ch->m_base + ch->m_end); // loop end

	// loop start
	if ((uint16_t)ch->m_loop == 65535) // 8bb: no loop
		GUS_SetStartAddress(ch->m_base);
	else
		GUS_SetStartAddress(ch->m_base + ch->m_loop);

	GUS_SetCurrAddress(ch->m_base + ch->m_pos); // begin/curpos

	// hz
	GUS_SetFrequency((uint16_t)(ch->m_speed >> 6));

	if (ch->aguschannel >= 0) // 8bb: added protection (yes, <0 can happen!)
	{
		if (ch->m_end == 0)
			channeltrig[ch->aguschannel] = 0b00000010; // stop
		else if ((uint16_t)ch->m_loop == 65535)
			channeltrig[ch->aguschannel] = 0b00000000; // noloop
		else
			channeltrig[ch->aguschannel] = 0b00001000; // loop
	}

	// finally slide volumeon (8bb: what's this 1->2 volume logic..?)
	setvolslide(ch, (ch->m_vol == 1) ? 2 : 1, ch->m_vol);
}

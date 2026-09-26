/***********************************************************************
 **
 **  Amiga note handling
 **
 ***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "dig.h"
#include "digdata.h"

void doamiga(zchn_t *ch)
{
	// ***INSTRUMENT***
	if (ch->ins > 0)
	{
		ch->astartoffset = 0;

		if (ch->ins < MAX_INSTRUMENTS) // 8bb: added for safety reasons
		{
			ch->lastins = ch->ins;

			const ds_smp *ins = &song.ins[ch->ins-1];
			if (ins->type != 0)
			{
				if (ins->type == 1) // sample
				{
					ch->ac2spd = (uint16_t)ins->c2spd; // 8bb: clamped to 0..65535 in sample loader
					ch->avol = CLAMP((int8_t)ins->vol, 0, 63);
					ch->aorgvol = ch->avol;
					setvol(ch);

					ch->m_base = ins->baseptr;

					uint16_t lend = (uint16_t)ins->lend;
					if ((ins->flags & 1) && lend != 0) // 8bb: loop enabled?
					{
						// loop, expand if <500 bytes long
						const uint16_t lbeg = (uint16_t)ins->lbeg; // loop start
						ch->m_loop = lbeg;

						if (lend <= 500)
						{
							int16_t a = lbeg - lend; // -looplen
							do
							{
								lend -= a;
							}
							while (lend < 500);

							lend += a;
						}

						ch->m_end = lend;
					}
					else // no loop
					{
						uint16_t length = (uint16_t)ins->length;
						length += 32; // extra fadeout for GUS (8bb: also applies to SB, and can also overflow)

						ch->m_end = length;
						ch->m_loop = 65535; // 8bb: disable loop
					}
				}
				else // 8bb: not a PCM sample
				{
					ch->lastins = 0;
				}
			}
		}
	}

	// continue only if we have an active instrument on this channel
	if (ch->lastins == 0)
		return;

	if (ch->cmd == 'O'-64)
	{
		if (ch->info == 0)
			ch->astartoffset = ch->astartoffset00;
		else
			ch->astartoffset00 = ch->astartoffset = ch->info << 8;
	}

	// ***NOTE***
	if (ch->note != 255)
	{
		if (ch->note == 254) // ^^
		{
			// end sample

			ch->m_pos = 1;
			ch->m_poslow = 0; // 8bb: also clear position frac

			ch->aspd = 0;
			setspd(ch);

			ch->avol = 0;
			setvol(ch);

			ch->m_end = 0;
			ch->m_loop = 65535; // 8bb: disable loop

			ch->asldspd = 65535; // 8bb: accidental label-jump typo in asm code causes this
		}
		else
		{
			// restart sample
			if (ch->cmd != 'G'-64 && ch->cmd != 'L'-64)
			{
				ch->m_pos = ch->astartoffset;
				ch->m_poslow = 0; // 8bb: also clear position frac

				ch->m_oldpos = 0x12345678; // 8bb: this forces a GUS retrigger
			}

			// calc note speed

			ch->lastnote = ch->note;

			int16_t newspd = scalec2spd(ch, stnote2herz(ch->note));
			if (ch->aorgspd == 0 || (ch->cmd != 'G'-64 && ch->cmd != 'L'-64))
			{
				ch->aspd = newspd;
				setspd(ch);
				ch->avibcnt = 0;
				ch->aorgspd = newspd; // original speed if true one changed with vibrato etc.
			}

			ch->asldspd = newspd; // destination for toneslide (G/L)
		}
	}

	// ***VOLUME***
	if (ch->vol != 255)
	{
		ch->avol = ch->vol;
		setvol(ch);
		ch->aorgvol = ch->vol;
	}
}

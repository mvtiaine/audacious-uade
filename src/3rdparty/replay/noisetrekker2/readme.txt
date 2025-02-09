=================================================================================================
NOISETREKKER DIGITAL COMPOSER RELEASE 12 DOCUMENTATION:
=================================================================================================

The Team
--------

Program coded by J. Arguelles Rius [arguru@vermail.net]
Additional code / libCON.dll: Amir Geva [photon@photoneffect.com]
WEB Support + Logo grafx: Felix Petrescu aka waKa x [wakax@yahoo.com]
Program quality control + testing: LOCOHLASGALASIAS[titan@vermail.net]
IFF2NTI v0.2 by krokpitr/AGGES [zscsoka@hotmail.com]

Contents
--------
Intro
FAQ/Troubleshooting
Keyboard Shortcuts

				Agarrense Team 7/February/2000

=================================================================================================
INTRODUCTION
=================================================================================================
Noisetrekker original concept / created by J. Arguelles/Fco. Portillo
in the attempt to get a powerful allinone music application that give
a solid and fast mode to make proffessional electronic/trance styles.
Of couse, we did, at least for us.

We didnt create Noisetrekker to get fame / make people happy / try
to enter a new standard, simply, is the tool that full our music
requiriments.
Noisetrekker is a FREEWARE tool, this means that you can use it
for free, anyway, if you're plaining to release any commercial stuff
with it, you MUST contact us for a commercial license of the program.
Also, we DONT GUARATEE that this program will run on your platform,
read the FAQ here anyway.
If u like the program and find it useful, congratulations, enjoy the
program and let's make some experimental shit. =]

What the HELL is it?

Noisetrekker is a music workstation composed of:
-16 Track sampler
-Sample editor
-16 based pattern tracker style track sequencer.
-16 Track synthesizer
-2 303s
-Multieffects engine with reverb, distortion, filters, delay, EQs, flanger, etc...

=================================================================================================
NTK R5 F.A.Q./TROUBLESHOOTING
=================================================================================================
-------------------------------------------------------------------------------------------------
Q> No Envelopes for samples, No Fasttracker support, no 53194 tracks?
   No etc...?, "m... i dont like the gui, the keyshortcuts,
   you should have to..."
-------------------------------------------------------------------------------------------------
A1> Get a compiler and code the best tracker ever. I'm sure u'll.
A2> Use fastracker, impulse tracker, buzz or whatever stuff you want.
A3> Put your internet explorer and search for some stuff...

-------------------------------------------------------------------------------------------------
Q> I Cannot run the program! Any idea?
-------------------------------------------------------------------------------------------------
A> If u have got a SBLive, try disabling EMU driver on your soundcard.
Be sure you have got lastest DirectX drivers for your stuff [gfx/sfx card] and DirectX6.1 or
above installed. [Microsoft DirectX 6.1 or above is required to run NTK]
Ntk wont run on Windows NT, it will run on Windows 95,98 and 2k.
Also, might not work with some gfx/sfx cards.

-------------------------------------------------------------------------------------------------
Q> Well, the program runs ok but ... how I can quit it???
-------------------------------------------------------------------------------------------------
Press ALT+ESC. Or just close the window if you're running ntk on
windows frame mode.

-------------------------------------------------------------------------------------------------
Q> I load Csynth presets but they doesnt sound...?
-------------------------------------------------------------------------------------------------
A> Have u set CPU mode [on DISK IO section] on HIGHER?
Medium and Low cpu modes have turned off the Csynth engine... you've
to put Higher usage to turn CSynth On.

-------------------------------------------------------------------------------------------------
Q> Ehh Everything sounds clicks, distorted and weird!!?
-------------------------------------------------------------------------------------------------
A> Put HIGHER LATENCY ON DISKIO section. This means that the CPU is not
having enough time to render/fill the audio buffer data, and this
produce clicking...
A2> Buy a Pentium 800Mhz if you want better performance in all your
soft.

-------------------------------------------------------------------------------------------------
Q> How I play 303 patterns on ntk?
-------------------------------------------------------------------------------------------------
A> Use NTk command 31xy and 32xy. 31xy will trigger pattern 'y' (1-8)
from bank 'x' (A-D) from the first 303 unit 
[ntk has got 2 303 units, like Rebirth]

Example
-------
|     00       |
|              |
|---......31A3 |
|---......0000 |
|---......0000 |

This will start to play the pattern '3' in the bank 'A', from the FIRST
303 unit. The 303 unit 1 will be assigned to the track 0 stream.
This means, that panning, fx setup, filter settings of the track will
be aplied to the sound of the 303 unit 1. 
To play the second 303 unit, is the same method but using 32xy, command
instead 31. To stop 303 playing [patterns are played "looped" continuously]
just put a note off on the track were was triggered.

Also, using -3100- or -3200- will replay the patterns that are currently selected on the 303
editor. Good while composing basslines.

Btw: 303 engine will not "eat" the track sampler/csynth engine, so
u can have both playing ex. a bassdrum and a 303 line on the 
same pattern. Also, both 303 units can be triggered in the
same track. Well, just play with them :].

-------------------------------------------------------------------------------------------------
Q> Oh!, the 303 sounds great, will you code a 'buzz' port???
-------------------------------------------------------------------------------------------------
A> Nope yet!.

-------------------------------------------------------------------------------------------------
Q> mmm, Rebirth patterns doesnt sound the same on ntk, why?
-------------------------------------------------------------------------------------------------
A> If u think you can code a better 303 emulator, please let me know.
   Or ask to Propellerheads guys for the source code. Or buy Rebirth :P

-------------------------------------------------------------------------------------------------
Q> How I can record 303 tweakings on the fly?
-------------------------------------------------------------------------------------------------
A> Press the button where says "Slider REC:OFF" [top/left screeen]
   and, voilá, the 303 tweakings are autofilled on pattern.

-------------------------------------------------------------------------------------------------
Q> How I can alter 303s CutOff,Resonance, etc.. on the pattern while
playing?
-------------------------------------------------------------------------------------------------
A> Easy, just use the 303 special pattern commands. They're very useful to
automatize 303 stuff.

-------------------------------------------------------------------------------------------------
Q1> What're the pattern commands effects on ntk? Are the same as .xm or .mod?
-------------------------------------------------------------------------------------------------
This is the complete list of the pattern commands on noisetrekker:

	General
	-------

	Command '00': No Effect :]
	Command '01': Pitch Up
	Command '02': Pitch Down
	Command '03': Volume
	Command '04': Trance Slicer
	Command '05': Glide
	Command '08': Set Filter CutOff
	Command '09': Set Sample Play Offset
	Command '0A': Randomize Filter CutOff"
	Command '0B': Filter CutOff Slide Up
	Command '0C': Filter CutOff Slide Down
	Command '0D': Jump To Next Sequencer Position
	Command '0E': Note Retrigger
	Command '0F': Set Number Of Ticks Per Beat
	Command '10': Set Delay/Echo Send
	Command '11': Set Reverb Send
	Command '12': Set Distortion Threshold
	Command '13': Set Distortion Clamp
	Command '14': Set Filter Resonance
	Command '15': Set Filter Type"
	Command '16': Reset Filter LFO
	Command '17xx': Auto Fade In in 'xx' ticks
	Command '18xx': Auto Fade Out in 'xx' ticks
	Command '19': Volume Up
	Command '20': Volume Down
	
	303 Triggering
	--------------
	Command '31xy': Trigger 303 Bassline [Unit 1] Bank 'x' (A-D) Pattern 'y' (1-8), also '00' will trigger current selected pattern.
	Command '32xy': Trigger 303 Bassline [Unit 2] Bank 'x' (A-D) Pattern 'y' (1-8), also '00' will trigger current selected pattern.


	303 Controllers
	---------------
	Where 'xx' is a number between $00 and $FF [hex]
	
	Command '33xx': Set 303 [Unit 1] Filter Cutoff
	Command '34xx': Set 303 [Unit 2] Filter Cutoff
	Command '35xx': Set 303 [Unit 1] Filter Resonance
	Command '36xx': Set 303 [Unit 2] Filter Resonance
	Command '37xx': Set 303 [Unit 1] Filter Env Mod
	Command '38xx': Set 303 [Unit 2] Filter Env Mod
	Command '39xx': Set 303 [Unit 1] Filter Decay
	Command '40xx': Set 303 [Unit 2] Filter Decay
	Command '41xx': Set 303 [Unit 1] Tune
	Command '42xx': Set 303 [Unit 2] Tune

	Misc
	----
	Command '80': Set Patch Bank [MIDI] (might not work on all midi equipment)
	Command 'F0': Set BPM speed

	Read below how you can send MidiOut controller messages
	and read the manual of your gear to how send RRPN messages
	using this midiOut messages, also, you might find here the
	midi implementation table of your hardware.

-------------------------------------------------------------------------------------------------
Q> Midi Out??? how to use???
-------------------------------------------------------------------------------------------------
A> First of all, midi out capabilities of this program are not full. But follow these steps:

0 - Put HIGHER CPU use on disk Io section. The other modes has got
    MIDI disabled.

1 - Go to Midi/CSynth section and select a midi out device to use [ntk only supports once at
    the same time].

2 - Go to instrument section, and select a MIDI PRG [by default is 00, that means, no midi
    program selected == no midi sound]

3 - Go to track section and here u can assign a midi channel to each track of ntk.

4 - Play notes :]. Note off works. F'x' note cut command also works too, and note-volume command
    [speed] is supported.

Also, you can change midicontrollers in the tracker, using '90' in the panning row:

ex:
	00

  C-302....0000  
  ---....90xxyy <-- This will change the value of the controller n.'xx' to 'yy' [both in hex]
  ---......0000

So "---....902040" will change the controller number $20(32) to $40(64).

U will need the midi implementation table of your gear to know what u can change with midi
controller messages. Probably, it's at end pages of the manual =].

-------------------------------------------------------------------------------------------------
Q> Audio & Midi are not synchronized, what I can do???
-------------------------------------------------------------------------------------------------
A1> Buy cubase.
A2> Buy logic.
A3> Pay for sex.
A4> Well, there is a nasty trick to synchronize both. It's a bit hardcore but work with me.

Simply put one line down to all midi notes on your pattern [use Insert key]and go to
'Disk Io', adjust the latency and just search a value that will make sound sync both audio/midi.

-------------------------------------------------------------------------------------------------
Q> The program is eating my CPU!!! What I can do?
-------------------------------------------------------------------------------------------------
A> Sorry, ntk needs a great CPU. I'm not enough good coder. The decent CPU for ntk is a P2 300Mhz
   and 64 mb ram. Try Low CPU usage mode [Disk Io section], but anyway, dont be stupid, buy
   a new one [cpu are cheap now!]. Btw: the tracker uses 4-point
   spline interpolation to get intermediate sample points during
   sampling engine. This means hi CPU consuming but you get pro quality
   with this.

=================================================================================================
KEYBOARD SHORTCUTS
=================================================================================================

Playing
-------

Right Control Key: Play song
Left clicking on 'Play Sng/Pttrn' button: Play song.
Right clicking on 'Play Sng/Pttrn' button: Play pattern.


Editing
-------

TAB: Go to next track
LSHIFT + TAB: Go to prev. track
LSHIFT while EDITING: Will "keyrepeate" fast
RSHIFT while EDITING: Will insert Note Off command
SPACE: Toggle Edit mode On & Off. Also stop the player if the song is being played
DOWN ARROW: 1 Line down
UP ARROW: 1 Line up
LEFT ARROW: 1 Row left
RIGHT ARROW: 1 Row right =]
PREV. PAGE: 16 Arrows Up
NEXT PAGE: 16 Arrorws Down
F5, F6, F7, F8, F9: Jump to 0, 16, 32, 48, 63 line
LSHIFT + F1: Transpose all notes below the current line [the line included too] of the current track -1 semitone
LSHIFT + F2: Transpose all notes below the current line [the line included too] of the current track +1 semitone
LSHIFT + F3, F4, F5: Cut, Copy, Paste track
CTRL + F3, F4, F5: Cut, Copy, Paster pattern


Tracking
--------

zsxdcvgbhnjm,l.ñ-: Lower octave
q2w3er5t6y7ui9o0p: Upper octave
/ and * on the Numeric keyboard: -1 or +1 octave
+ and - on the Numeric keyboard: Selects/Edits the prev or next pattern on the current sequencer position
Insert / BackSlash: Inserts or Delete an empty note
Delete [key below Insert on the keyboard]: Overwrite an empty note


Block
-----
CTRL + B: Set begin of the current block
CTRL + E: Set end of the current block
CTRL + X: Cut the select block and copy it to the block-buffer
CTRL + V: Paste the data on the block buffer in pattern
CTRL + Z: Paste the data on the block buffer in pattern, but only will insert the effect colums data, it wont overwrite notes
CTRL + I: Make effect-interpolation between the data on the first and last note on the block

example: 

C-3  04  ..  ..  09  00  CTRL+I - Interpolation --->  C-3  04  ..  ..  09  00  
C-3  04  ..  ..  09  00  			      C-3  04  ..  ..  09  02  
C-3  04  ..  ..  09  00  			      C-3  04  ..  ..  09  05  
C-3  04  ..  ..  09  00  			      C-3  04  ..  ..  09  07  
C-3  04  ..  ..  09  00  			      C-3  04  ..  ..  09  0A  
C-3  04  ..  ..  09  00  			      C-3  04  ..  ..  09  0D  
C-3  04  ..  ..  09  00  			      C-3  04  ..  ..  09  10  

Cool to make cutoff transitions, etc... [anyway, you can switch on the Slider Rec to On, and perform parameter-live-recording, such as cutoff, resonance or panning tweaking, etc..]

CTRL + R: Randomize the effect data on the selection, works similar to CTRL + I, but it randomizes values instead of interpolation.

=================================================================================================
-------------------------------------------------------------------------------------------------
Juan Antonio Arguelles Rius <arguru@vermail.net>
-------------------------------------------------------------------------------------------------
=================================================================================================









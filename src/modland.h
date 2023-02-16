/*
    Copyright (C) 2014-2023  Matti Tiainen <mvtiaine@cc.hut.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef MODLAND_H_
#define MODLAND_H_

typedef struct {
    String format, author, album;
} modland_data_t;

int modland_init(void);
void modland_cleanup(void);
modland_data_t *modland_lookup(const char *md5);

// These must be in alphabetical order
static const char * const modland_amiga_formats[] = {
    "AHX",
    "AM Composer",
    "AProSys",
    "Actionamics",
    "Activision Pro",
    "Anders Oland",
    "Arpeggiator",
    "Art And Magic",
    "Art Of Noise",
    "Audio Sculpture",
    "BP SoundMon 2",
    "BP SoundMon 3",
    "Beathoven Synthesizer",
    "Ben Daglish",
    "Ben Daglish SID", // C64
    "Core Design",
    "CustomMade",
    "Cybertracker",
    "Darius Zendeh",
    "Dave Lowe",
    "Dave Lowe New",
    "David Hanney",
    "David Whittaker",
    "Delitracker Custom",
    "Delta Music",
    "Delta Music 2",
    "Delta Packer",
    "Desire",
    "Digibooster",
    "Digibooster Pro",
    "Digital Mugician",
    "Digital Mugician 2",
    "Digital Sonix And Chrome",
    "Digital Sound Studio",
    "Dirk Bialluch",
    "Dynamic Studio Professional",
    "Dynamic Synthesizer",
    "EarAche",
    "Electronic Music System",
    "Electronic Music System v6",
    "Face The Music",
    "Fashion Tracker",
    "Follin Player II",
    "Forgotten Worlds",
    "Fred Gray",
    "FredMon",
    "FuchsTracker",
    "Future Composer 1.3",
    "Future Composer 1.4",
    "Future Composer BSI",
    "Future Player",
    "GT Game Systems",
    "Game Music Creator",
    "GlueMon",
    "Hippel",
    "Hippel 7V",
    "Hippel COSO",
    "Hippel ST", // ST
    "Hippel ST COSO", // ST
    "HivelyTracker",
    "Howie Davies",
    "Images Music System",
    "Infogrames",
    "InStereo!",
    "InStereo! 2.0",
    "JamCracker",
    "Janko Mrsic-Flogel",
    "Jason Brooke",
    "Jason Page",
    "Jason Page Old",
    "Jeroen Tel",
    "Jesper Olsen",
    "Kris Hatlelid",
    "Leggless Music Editor",
    "Lionheart",
    "MCMD",
    "Magnetic Fields Packer",
    "Maniacs Of Noise",
    "Mark Cooksey",
    "Mark Cooksey Old",
    "Mark II",
    "MaxTrax",
    "Medley",
    "Mike Davies",
    "MultiMedia Sound",
    "Music Assembler",
    "Music Editor",
    "MusicMaker V8",
    "MusicMaker V8 Old",
    "Musicline Editor",
    "NovoTrade Packer",
    "OctaMED MMD0",
    "OctaMED MMD1",
    "OctaMED MMD2",
    "OctaMED MMD3",
    "OctaMED MMDC",
    "Oktalyzer",
    "Paul Robotham",
    "Paul Shields",
    "Paul Summers",
    "Peter Verswyvelen",
    "Pierre Adane Packer",
    "PokeyNoise",
    "Powertracker",
    "Pretracker",
    "Professional Sound Artists",
    "Protracker",
    "Protracker IFF",
    "Pumatracker",
    "Quadra Composer",
    "Quartet PSG", // ST
    "Quartet ST", // ST
    "Richard Joseph",
    "Riff Raff",
    "Rob Hubbard",
    "Rob Hubbard ST", // ST
    "Ron Klaren",
    "SCUMM",
    "Sean Connolly",
    "Sean Conran",
    "SidMon 1",
    "SidMon 2",
    "Silmarils",
    "Sonic Arranger",
    "Sound Images",
    "Sound Master",
    "Sound Master II v1",
    "Sound Master II v3",
    "Sound Programming Language",
    "SoundControl",
    "SoundFX",
    "SoundFactory",
    "SoundPlayer",
    "Soundtracker",
    "Soundtracker 2.6",
    "Soundtracker Pro II",
    "Special FX",
    "Special FX ST", // ST
    "Speedy A1 System",
    "Speedy System",
    "Startrekker AM",
    "Startrekker FLT8",
    "Steve Barrett",
    "Stonetracker",
    "SunTronic",
    "Symphonie",
    "SynTracker",
    "Synth Dream",
    "Synth Pack",
    "Synthesis",
    "TCB Tracker",
    "TFMX",
    "TFMX ST", // ST
    "The Musical Enlightenment",
    "Thomas Hermann",
    "Tomy Tracker",
    "Unique Development",
    "Voodoo Supreme Synthesizer",
    "Wally Beben",
    "YMST", // ST
    "Zoundmonitor",
    NULL
};

#endif /* MODLAND_H_ */

/*
    Copyright (C) 2014  Matti Tiainen <mvtiaine@cc.hut.fi>

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
    char *format, *author, *album;
} modland_data_t;

int modland_init(void);
void modland_cleanup(void);
modland_data_t *modland_lookup(const char *md5);

static const char * const modland_amiga_formats[] = {
    "AHX",
    "AM Composer",
    "AProSys",
    "Actionamics",
    "Activision Pro",
    "Anders Oland",
    "Art And Magic",
    "Art Of Noise",
    "Audio Sculpture",
    "BP SoundMon 2",
    "BP SoundMon 3",
    "Beathoven Synthesizer",
    "Ben Daglish",
    "Ben Daglish SID",
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
    "Hippel-Atari",
    "Hippel-COSO",
    "Hippel-ST",
    "HivelyTracker",
    "Howie Davies",
    "Images Music System",
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
    "Maniacs Of Noise Old",
    "Mark Cooksey",
    "Mark Cooksey Old",
    "Mark II",
    "MaxTrax",
    "Medley",
    "Mike Davies",
    "MultiMedia Sound",
    "Music Assembler",
    "Music Editor",
    "MusicMaker",
    "MusicMaker v8",
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
    "Professional Sound Artists",
    "Protracker",
    "Protracker 3.6",
    "Pumatracker",
    "Quadra Composer",
    "Quartet PSG",
    "Quartet ST",
    "Richard Joseph",
    "Riff Raff",
    "Rob Hubbard",
    "Rob Hubbard 2",
    "Rob Hubbard ST",
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
    "SoundFX 2",
    "SoundFactory",
    "SoundPlayer",
    "SoundTracker 2.6",
    "SoundTracker Pro II",
    "Special FX",
    "Special FX ST",
    "Speedy A1 System",
    "Speedy System",
    "Startrekker AM",
    "Steve Barrett",
    "Stonetracker",
    "SunTronic",
    "Symphonie",
    "SynTracker",
    "Synth Dream",
    "Synth Pack",
    "Synthesis",
    "TFMX",
    "TFMX ST",
    "The Holy Noise",
    "The Musical Enlightenment",
    "Thomas Hermann",
    "Tomy Tracker",
    "Unique Development",
    "Voodoo Supreme Synthesizer",
    "Wally Beben",
    "YMST",
    "Zoundmonitor",
    NULL
};

#endif /* MODLAND_H_ */

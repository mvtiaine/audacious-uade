// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <map>
#include <set>
#include <string>

#include "common.h"
#include "modland.h"

using namespace std;

namespace {

const set<string> modland_amiga_formats ({
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
    "HivelyTracker",
    "Howie Davies",
    "IFF-SMUS",
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
    "Jesper Olsen", // missing data files (WantedTeam.bin)
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
    "Powertracker",
    "Pretracker",
    "Professional Sound Artists",
    "Protracker",
    "Protracker IFF",
    "Pumatracker",
    "Quadra Composer",
    "Richard Joseph",
    "Riff Raff",
    "Rob Hubbard",
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
    "Speedy A1 System",
    "Speedy System",
    "Startrekker AM",
    "Startrekker FLT8",
    "Steve Barrett",
    "Stonetracker",
    "SunTronic",
    "Symphonie",
    "SynTracker",
    "Synth Dream", // sdr.nobuddiesland* broken ?
    "Synth Pack",
    "Synthesis",
    "TFMX",
    "The Musical Enlightenment",
    "Thomas Hermann",
    "Tomy Tracker",
    "Unique Development",
    "Voodoo Supreme Synthesizer",
    "Wally Beben",
    "Zoundmonitor",
    "Ben Daglish SID", // C64
    "Hippel ST", // Atari ST
    "Hippel ST COSO", // Atari ST
    "PokeyNoise", // Atari XL/XE
    "Quartet PSG", // Atari ST
    "Quartet ST", // Atari ST
    "Rob Hubbard ST", // Atari ST
    "Special FX ST", // Atari ST
    "TCB Tracker", // Atari ST
    "TFMX ST", // Atari ST
    "YMST", // Atari ST
});

// only songlengths are used for these
const map<string, bool> modland_incoming ({
    // these dirs from incoming have author info available in "standard" format
    {"vault/Cinemaware", true},
    {"workshop/ABK", true},
    {"workshop/NP2", true},
    {"workshop/P41A", true},
    {"workshop/P4X", true},
    {"workshop/P60", true},
    {"workshop/P6X", true},
    {"workshop/PHA", true},
    {"workshop/PP21", true},
    {"workshop/PP30", true},
    {"workshop/TP3", true},
    {"workshop/channel players", true},
    {"workshop/chiptracker", true},
    {"workshop/prun", true},
    // these are only used for songlengths
    {"vault/Sonix", false},
    {"workshop/NP1", false},
    {"workshop/NP3", false},
    {"workshop/P40A", false},
    {"workshop/P40B", false}, 
    {"workshop/P60A", false},
    {"workshop/P61", false},
    {"workshop/P61A", false},
    {"workshop/PM01", false},
    {"workshop/PP10", false},
    {"workshop/PP20", false},
    {"workshop/PPK", false},
    {"workshop/PRU1", false},
    {"workshop/PRU2", false},
    {"workshop/SKYT", false},
    {"workshop/STPK", false},
    {"workshop/TP1", false},
    {"workshop/TP2", false},
    {"workshop/UNIC", false},
    {"workshop/UNIC2", false},
    {"workshop/XANN", false},
    {"workshop/ZEN", false},
    {"workshop/eureka packer", false},
    {"workshop/fc-m", false},
    {"workshop/fuzzac packer", false},
    {"workshop/heatseeker", false},
    {"workshop/hornet packer", false},
    {"workshop/icetracker", false},
    {"workshop/kefrens sound machine", false},
    {"workshop/kris tracker", false},
    {"workshop/laxity tracker", false},
    {"workshop/promizer", false},
    {"workshop/propacker", false},
    {"workshop/skyt packer", false},
    {"workshop/unic tracker", false},
    {"workshop/xann packer", false},
    {"laboratory/SUNTronicTunes", false},
    {"laboratory/Sierra AGI", false},
    {"laboratory/TCB Tracker", false},
    // misc
    {"laboratory/unsorted gamemusic", false},
    {"laboratory/various_formats_from_amiga_games", false},
    {"warehouse/MOD", false},
    {"delivery bay/Amiga game mods", false},
    {"delivery bay/from_Chip", false},
    {"delivery bay/mbnet_mods", false},
    {"delivery bay/noise", false},
    {"delivery bay/principium" , false},
    {"delivery bay/various", false},
    // no support ?
    {"workshop/TP6", true},
    {"workshop/HCD", false},
    {"workshop/NTP1", false},
    {"workshop/NewtronPacker", false},
    {"workshop/P22A", false},
    {"workshop/P30A", false},
    {"workshop/P60-packedsamples", false},
    {"workshop/PR10", false},
    {"workshop/PR20", false},
    {"workshop/PR40", false},
    {"workshop/moduleprotector", false},
    {"workshop/startrekkerpacker", false},
    {"laboratory/FLAME", false},
    {"laboratory/MP", false},
});

constexpr string_view COOP = "coop-";
constexpr string_view UNKNOWN = "- unknown";
constexpr string_view NOTBY = "not by ";
constexpr string_view UNUSED = "Unused";

} // namespace

bool parse_modland_path(const string &path, ModlandData &item, bool incoming) {
    string format, author, album, filename;
    
    vector<string> tokens = split(path, "/");
    const int count = tokens.size();

    if (count < 3) {
        TRACE("Skipping path: %s\n", path.c_str());
        return false;
    }

    if (incoming) {
        const auto full_path = tokens[0] + tokens[1];
        if (!modland_incoming.count(full_path)) {
            TRACE("Skipping unknown incoming path: %s\n", full_path.c_str());
            return false;
        }
        if (!modland_incoming.at(full_path)) {
            TRACE("Skipping incoming path: %s\n", full_path.c_str());
            return false;
        }
        format = tokens[1];
        tokens.erase(tokens.begin());
    } else {
        if (!modland_amiga_formats.count(format)) {
            TRACE("Skipping non-amiga format %s\n", format.c_str());
            return false;
        }
        format = tokens[0];
    }

    author = tokens[1];

    switch (count) {
        case 3:
            filename = tokens[2];
            break;
        case 4: {
            string token = tokens[2];
            if (token.find(COOP) == 0) {
                author = author + " & " + token.substr(COOP.length());
            } else if (token.find(NOTBY) != 0) {
                album = token;
            }
            filename = tokens[3];
            break;
        }
        case 5: {
            string token = tokens[2];
            if (token.find(COOP) == 0) {
                author = author + " & " + token.substr(COOP.length());
                album = tokens[3];
                filename = tokens[4];
                break;
            } else if (tokens[3] == UNUSED) {
                author = tokens[1];
                album = tokens[2];
                filename = tokens[4];
                break;
            }
            author = tokens[1];
            album = tokens[2] + " (" + tokens[3] + ")";
            filename = tokens[4];
            break;
        }
        case 6:
            if (format == "IFF-SMUS" && author == UNKNOWN) {
                filename = tokens[5];
                break;
            }
        default:
            TRACE("Skipping path %s, token count %d\n", path.c_str(), count);
            break;
    }

    item.format = format;
    if (author == UNKNOWN) {
        item.author = UNKNOWN_AUTHOR;
    } else {
        item.author = author;
    }
    if (album.size()) {
        item.album = album;
    }
    if (filename.size()) {
        item.filename = filename;
    }

    return true;
}

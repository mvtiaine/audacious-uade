// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <libaudcore/audstrings.h>
#include <libaudcore/runtime.h>

#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include "common.h"
#include "modland.h"

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
    "Synth Dream",
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

constexpr string_view COOP = "coop-";
constexpr string_view UNKNOWN = "- unknown";
constexpr string_view NOTBY = "not by ";

constexpr string_view UNKNOWN_AUTHOR = "<Unknown>";

string previous_md5_file = "";

map<string, vector<ModlandData>> ml_map;

bool parse_modland_path(const string &path, ModlandData &item) {
    string format, author, album, filename;
    
    vector<string> tokens = split(path, "/");
    const int count = tokens.size();

    if (count < 3) {
        DEBUG("Unexpected path: %s\n", path.c_str());
        return 1;
    }

    format = tokens[0];

    if (!modland_amiga_formats.count(format)) {
        TRACE("Skipping non-amiga format %s\n", format.c_str());
        return 1;
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
            } else {
                DEBUG("Skipped path: %s\n", path.c_str());
                return 1;
            }
            album = tokens[3];
            filename = tokens[4];
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

void try_init(void) {
    string md5_file = string(aud_get_str (PLUGIN_NAME, MODLAND_ALLMODS_MD5_FILE));

    if (md5_file.empty()) {
        const char *home = getenv ("HOME");
        md5_file = string(home) + "/.uade/allmods_md5_amiga.txt";
    }

    if (previous_md5_file == md5_file) {
        return;
    }
    
    DEBUG("Modland md5 file location changed\n");

    previous_md5_file = md5_file;
    ml_map.clear();
   
    ifstream allmods(md5_file.c_str(), ios::in);
    if (!allmods.is_open()) {
        DEBUG("Could not open modland md5 file %s\n", md5_file.c_str());
        return;
    }

    string line;
    while (getline(allmods, line)) {
        string md5;
        string path;

        // sanity check
        if (line.length() <= 32) {
            ERROR("Too short line %s\n", line.c_str());
            return;
        }

        md5 = line.substr(0, 32);

        path = line.substr(33, line.length());

        ModlandData item {};

        if (parse_modland_path(path, item)) {
            if (ml_map.count(md5)) {
                DEBUG("Duplicate md5: %s\n", line.c_str());
                auto& data = ml_map[md5];
                data.push_back(item);
            } else {
                vector<ModlandData> data;
                data.push_back(item);
                ml_map[md5] = data;
            }
        }
        //TRACE("%s -> format = %s, author = %s, album = %s, filename = %s\n", line.c_str(), item.format, item.author, item.album, item.filename);
    }

    return;
}

} // namespace

optional<ModlandData> modland_lookup(const char *md5, const string &filename) {
    try_init();

    const string key(md5);
    if (ml_map.count(key)) {
        for (const auto& data : ml_map[key]) {
            if (filename == data.filename) {
                return data;
            }
        }
        // return first if no filename match
        return ml_map[key].front();
    }

    return optional<ModlandData>();
}

// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2014-2023 Matti Tiainen <mvtiaine@cc.hut.fi>

#include <fstream>
#include <map>
#include <sstream>
#include <set>
#include <vector>

#include "common.h"
#include "songdb.h"
#include "prefs.h"

namespace {

constexpr auto MODLAND_TSV_FILE = UADEDIR "/modland.tsv";
bool initialized = false;

const set<string> modland_amiga_formats ({
    "Cinemaware", // incoming/vault
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

constexpr string_view COOP = "coop-";
constexpr string_view UNKNOWN = "- unknown";
constexpr string_view NOTBY = "not by ";

constexpr string_view UNKNOWN_AUTHOR = "<Unknown>";

map<pair<string,int>, vector<SongInfo>> db;

bool parse_modland_path(const string &path, ModlandData &item) {
    string format, author, album, filename;
    
    vector<string> tokens = split(path, "/");
    const int count = tokens.size();

    if (count < 3) {
        DEBUG("Unexpected path: %s\n", path.c_str());
        return false;
    }

    format = tokens[0];

    if (!modland_amiga_formats.count(format)) {
        TRACE("Skipping non-amiga format %s\n", format.c_str());
        return false;
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
            } else if (format != "IFF-SMUS") {
                DEBUG("Skipped path: %s\n", path.c_str());
                return false;
            }
            album = tokens[3];
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

} // namespace

void songdb_init(void) {
    if (initialized) {
        return;
    }

    db.clear();
   
    ifstream songdbtsv(MODLAND_TSV_FILE, ios::in);
    if (!songdbtsv.is_open()) {
        ERROR("Could not open modland.tsv file %s\n", MODLAND_TSV_FILE);
        return;
    }

    string line;
    string prevmd5;
    optional<ModlandData> modland_item {};
    while (getline(songdbtsv, line)) {
        const auto cols = split(line, "\t");
        if (cols.size() < 4) {
            ERROR("Invalid line %s\n", line.c_str());
            return;
        }
        string md5 = cols[0];
        int subsong = atoi(cols[1].c_str());
        int length = atoi(cols[2].c_str());
        string reason = cols[3];
        if (prevmd5 != md5 ) {
            if (cols.size() > 4) {
                const auto modland_path = cols[4];
                ModlandData item {};
                if (parse_modland_path(modland_path, item)) {
                    modland_item = item;
                } else {
                    modland_item = {};
                }
            }
        }

        const auto key = pair(md5,subsong);
        const SongInfo info = { md5, subsong, length, reason, modland_item };
        songdb_update(info);

        //TRACE("%s -> format = %s, author = %s, album = %s, filename = %s\n", line.c_str(), item.format, item.author, item.album, item.filename);
    }

    initialized = true;

    return;
}

optional<SongInfo> songdb_lookup(const char *md5, int subsong, const string &filename) {
    const auto key = pair(md5, subsong);
    if (db.count(key)) {
        for (const auto& data : db[key]) {
            if (data.modland_data.has_value() && filename == data.modland_data.value().filename) {
                return data;
            }
        }
        // return first if no filename match
        return db[key].front();
    }

    return {};
}

void songdb_update(const SongInfo &info) {
    if (info.md5.empty() || info.subsong < 0) {
        WARN("Invalid songdb key md5:%s subsong:%d\n", info.md5.c_str(), info.subsong);
        return;
    }
    const auto key = pair(info.md5, info.subsong);
    if (db.count(key)) {
         DEBUG("Duplicate md5 for: %s:%d\n", key.first.c_str(), key.second);
         auto& data = db[key];
         data.push_back(info);
    } else {
         vector<SongInfo> data;
         data.push_back(info);
         db[key] = data;
    }

    return;
}
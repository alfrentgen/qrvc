#ifndef OPTHELP_H
#define OPTHELP_H

#include <string>
#include <vector>
#include "utilities.h"

using namespace std;
static map<string, vector<string>> g_optionsMap {
    {"common" , vector<string>{
    "-i - Input file name. If the option is omitted stdin is used. [stdin]",
    "-o - Output file name. If the option is omitted stdout is used. [stdout]",
    "-f - Frame size. <WidthxHeight> [1280x720]",
    "-c - Ciphering on/off. If a value is given, it is treated as a key file  name. Otherwise key frame is the first one.",
    "-p - Chunks per working thread in a read/write cycle. The maximum is preferrable! <1-99> [8]",
    "-w - Number of working threads. <1-99> [number of CPUs]"
    }},

    //decoder's only
    {"decoder", vector<string> {
    "-m - Decode mode. <quick, mixed, slow> [mixed]",
    "-k - Skip duplicate frames, if occur."
    }},

    //encoder's only
    {"encoder", vector<string> {
    "-t - Number of trailing frames. An addtional number of the last frame repetitions. <0-99> [10]",
    "-r - Number of frame repetitions. <1-9> [1]",
    //'-n': ('Inverse frame colors. (Inversed frames are not supported in decoder.)','none','none'),
    "-a - Size of square alignment block. QR code is centered inside a frame in terms of alignment blocks. <0-64> [8]",
    "-e - QR code error correction capability level. <0-3> [0]",
    "-s - QR code scale factor. <1-99> [4]"
    }}
};

void print_help(string key){
    if(g_optionsMap.find(key) != g_optionsMap.end()){
        for(string s : g_optionsMap[key]){
            LOG("%s\n", s.c_str());
        }
    }
}
#endif

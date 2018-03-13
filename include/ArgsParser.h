#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include "pch.h"

using namespace std;

typedef struct Config {
    bool    m_cypheringOn;
    bool    m_inverseFrame;
    bool    m_skipDupFrames;

    int32_t m_frameWidth;
    int32_t m_frameHeight;
    int32_t m_frameRepeats;
    int32_t m_nTrailingFrames;

    int32_t m_nWorkingThreads;
    int32_t m_framesPerThread;

    int32_t m_qrScale;
    int32_t m_eccLevel;
    int32_t m_qrVersion;
    int32_t m_alignment;

    int32_t m_decMode;

    string  m_ifName;
    string  m_ofName;
    string  m_keyFileName;
} Config;

class ArgsParser
{
    public:
        vector<string> m_options;
        map<string, string> m_parsedOptions;

    public:
        ArgsParser();
        virtual ~ArgsParser();
        virtual int parseOptions(int argc, char **argv);
        virtual Config* GetConfig();
        virtual map<string, string>& getOptions();

    protected:
    private:
        bool IsOptionName(string& val);
};

#endif // ARGSPARSER_H

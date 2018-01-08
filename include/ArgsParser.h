#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include "pch.h"

using namespace std;

typedef struct Config {
    bool    m_counterOn;
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

    int32_t m_decMode;

    string  m_ifName;
    string  m_ofName;
} Config;

class ArgsParserDec
{
    public:
        vector<string> m_options;
        map<string, string> m_parsedOptions;

    public:
        ArgsParserDec();
        virtual ~ArgsParserDec();
        virtual int parseOptions(int argc, char **argv);
        virtual Config* GetConfig();
        virtual map<string, string>& getOptions();

    protected:
    private:
        bool IsOptionName(string& val);
};

#endif // ARGSPARSER_H

#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include "pch.h"
#include <map>

using namespace std;

class ArgsParserEnc
{
    public:
        map<string, string> m_parsed;
        map<string, regex> m_parseThem;

    public:
        ArgsParserEnc();
        //ArgsParserEnc(map<string, string> optionsAndRegex);
        virtual ~ArgsParserEnc();
        virtual int parseOptions(int argc, char **argv);
        virtual map<string, string>* getOptions();

    protected:
    private:
};

#endif // ARGSPARSER_H

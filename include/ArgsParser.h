#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include "pch.h"

using namespace std;

class ArgsParserDec
{
    public:
        vector<string> m_options;
        map<string, string> m_parsedOptions;

    public:
        ArgsParserDec();
        virtual ~ArgsParserDec();
        virtual int parseOptions(int argc, char **argv);
        virtual map<string, string>& getOptions();

    protected:
    private:
        bool IsOptionName(string& val);
};

#endif // ARGSPARSER_H

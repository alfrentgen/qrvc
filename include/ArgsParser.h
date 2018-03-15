#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include "pch.h"
#include "Config.h"

using namespace std;

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

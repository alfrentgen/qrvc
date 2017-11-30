#include "ArgsParser.h"

using namespace std;

ArgsParserDec::ArgsParserDec() :
    m_options({string("-i"), string("-o"), string("-f"), string("-c"), string("-p"), string("-t"), string("-l")})
{
    //ctor
}

ArgsParserDec::~ArgsParserDec()
{
    //dtor.h
}

int ArgsParserDec::parseOptions(int argc, char **argv){
    int n = 0;

    string& inFileOpt = m_options[0];//input file name
    string& outFileOpt = m_options[1];//output file name
    string& sizeOpt = m_options[2];//frame size -s NxN
    string& counterOpt = m_options[3];//counter on/off
    string& chunksPerThOpt = m_options[4];//chunks per thread
    string& threadsNumberOpt = m_options[5];//number of threads
    string& decLibraryOpt = m_options[6];//decode library

    string optionVal;
    string option;
    regex pattern;

    function<int32_t()> CheckOptionVal = [&](){
        if(argc == n+1 || !argv[n+1][0]){
            cerr << "No option value found." << endl;
            return FAIL;
        }else {
            optionVal = string(argv[n+1]);

            if(!ArgsParserDec::IsOptionName(optionVal) && regex_match(optionVal, pattern)){
                m_parsedOptions[option] = optionVal;
                ++n;
            }else{
                return FAIL;
            }
        }
        return OK;
    };

    for(n = 1; n < argc; n++){

        option = string(argv[n]);
        cerr << "On option: " << option << "\n";

        if(option == counterOpt){
            m_parsedOptions[option] = string("1");
            cerr << "Counter is on! OK!\n";

        }else if(option == sizeOpt){
            pattern = "\\d{1,4}x\\d{1,4}";
            if(CheckOptionVal() == OK){
                cerr << "Size pattern matches! Size string: " + optionVal << ". OK!\n";;
            }else{
                cerr << "Bad size format. Terminated.\n";
                return FAIL;
            }
        }else if(option == inFileOpt || option == outFileOpt){
            pattern = ".*";
            if(CheckOptionVal() == OK){
                cerr << "File name: " + optionVal << " - OK!\n";
            }else{
                cerr << "Bad file name. Terminated.\n";
                return FAIL;
            }
        }else if(option == threadsNumberOpt){
            pattern = "\\d{1,2}";
            if(CheckOptionVal() == OK){
                m_parsedOptions[option] = optionVal;
                cerr << "Size pattern matches! Size string: " + optionVal << ". OK!\n";
            }else{
                cerr << "Bad number of threads. Terminate.\n";
                return FAIL;
            }
        }else if(option == chunksPerThOpt){
            pattern = "\\d{1,2}";
            if(CheckOptionVal() == OK){
                cerr << "Size pattern matches! Size string: " + optionVal << ". OK!\n";
            }else{
                cerr << "Bad chunks per thread number. Can be 1-99. Terminate.\n";
                return FAIL;
            }
        }else if(option == decLibraryOpt){
            pattern = "(^quirc$|^zbar$){1}";
            if(CheckOptionVal() == OK){
                cerr << "Library to use: " + optionVal << ". OK!\n";
            }else{
                cerr << "Bad decode library name. Terminate.\n";
                return FAIL;
            }
        }else{
            cerr << "Wrong option. Terminated!\n";
            return FAIL;
        }
    }
    return n;
}

map<string, string>& ArgsParserDec::getOptions(){
    return m_parsedOptions;
}

bool ArgsParserDec::IsOptionName(string& str){
    for (auto & element : m_options) {
        if(element == str){
            cerr << element << ": option value missed\n";
            return true;
        }
    }
    return false;
}

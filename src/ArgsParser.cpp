#include "ArgsParser.h"

using namespace std;

ArgsParserDec::ArgsParserDec() :
    m_options({ string("-i"),
                string("-o"),
                string("-f"),
                string("-c"),
                string("-p"),
                string("-w"),
                string("-m"),
                string("-e"),
                string("-t"),
                string("-s"),
                string("-r")
                })
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
    string& workersNumberOpt = m_options[5];//number of threads
    string& decModeOpt = m_options[6];//decode library
    string& errorLevelOpt = m_options[7];//error correction level
    string& tailOpt = m_options[8];//number of tail frames
    string& qrScaleOpt = m_options[9];//qr scale
    string& repeatOpt = m_options[10];//frame repetition

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
        }else if(option == workersNumberOpt){
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
        }else if(option == decModeOpt){
            pattern = "(^quick$|^normal$){1}";
            if(CheckOptionVal() == OK){
                cerr << "Library to use: " + optionVal << ". OK!\n";
            }else{
                cerr << "Bad decode mode. Terminate.\n";
                return FAIL;
            }
        }else
        if(option == errorLevelOpt){
            pattern = "[0123]";
            if(CheckOptionVal() == OK){
                cerr << "level pattern matches! Level string: " + optionVal << endl;
            }else{
                cerr << "level can be 0-3. Terminated." << endl;
                return FAIL;
            }
            /*if(argc == n+1){
                cerr << "No level string was found" << endl;
                return -1;
            }else{
                string optionVal = string(argv[n+1]);
                if(regex_match(optionVal, bytesPattern)){
                    m_parsed[option] = optionVal;
                    cerr << "level pattern matches! Level string: " + optionVal << endl;
                    ++n;
                }else{
                    cerr << "level can be 0-3. Terminated." << endl;
                    return -1;
                }
            }
            cerr << "Level - OK!\n";*/
        }else
        if(option == tailOpt){
            pattern = "\\d{1,2}";
            if(CheckOptionVal() == OK){
                cerr << "Tail pattern matches! Number of trailing frames: " + optionVal << endl;
            }else{
                cerr << "Number of repeats should be in rage 0-99. Terminated." << endl;
                return FAIL;
            }
            /*regex tailPattern = regex("\\d{1,2}");
            if(argc == n+1){
                cerr << "No tail string found" << endl;
                return -1;
            }else{
                string optionVal = string(argv[n+1]);
                if(regex_match(optionVal, tailPattern)){
                    m_parsed[option] = optionVal;
                    cerr << "Tail pattern matches! Number of trailing frames: " + optionVal << endl;
                    ++n;
                }else{
                    cerr << "Number of repeats should be in rage 0-99. Terminated." << endl;
                    return -1;
                }
            }*/
        } else
        if(option == qrScaleOpt){
            pattern = "[1-3]?\\d{1}";
            if(CheckOptionVal() == OK){
                cerr << "Scale pattern matches! Scale string: " + optionVal << endl;
            }else{
                cerr << "Scale string should be an integer from 1 to 4. Terminated." << endl;
                return FAIL;
            }

            /*regex scalePattern = regex("[1-3]?\\d{1}");
            if(argc == n+1){
                cerr << "No scale string found" << endl;
                return -1;
            }else{
                string optionVal = string(argv[n+1]);
                if(regex_match(optionVal, scalePattern)){
                    m_parsed[option] = optionVal;
                    cerr << "Scale pattern matches! Scale string: " + optionVal << endl;
                    ++n;
                }else{
                    cerr << "Scale string should be an integer from 1 to 4. Terminated." << endl;
                    return -1;
                }
            }
            cerr << "Scale - OK!\n";*/
        }else
        if(option == repeatOpt){
            pattern = "\\d{1,1}";
            if(CheckOptionVal() == OK){
                cerr << "Repeat counter pattern matches! Number of repeats: " + optionVal << endl;
            }else{
                cerr << "Number of repeats should be in rage 0-9. Terminated." << endl;
                return FAIL;
            }
            /*regex repeatPattern = regex("\\d{1,1}");
            if(argc == n+1){
                cerr << "No repeat string found" << endl;
                return -1;
            }else{
                string optionVal = string(argv[n+1]);
                if(regex_match(optionVal, repeatPattern)){
                    m_parsed[option] = optionVal;
                    cerr << "Repeat counter pattern matches! Number of repeats: " + optionVal << endl;
                    ++n;
                }else{
                    cerr << "Number of repeats should be in rage 0-9. Terminated." << endl;
                    return -1;
                }
            }*/
        }
        else{
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

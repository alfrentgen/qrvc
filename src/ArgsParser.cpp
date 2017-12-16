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
        //cerr << "On option: " << option << "\n";

        if(option == counterOpt){
            m_parsedOptions[option] = string("1");
            cerr << "Counter is on!\n";

        }else if(option == sizeOpt){
            pattern = "\\d{1,4}x\\d{1,4}";
            if(CheckOptionVal() == OK){
                cerr << "Frame size: " + optionVal << ".\n";;
            }else{
                cerr << "Bad size format. Terminated.\n";
                return FAIL;
            }
        }else if(option == inFileOpt || option == outFileOpt){
            pattern = ".*";
            if(CheckOptionVal() == OK){
                string direction;
                if(option == inFileOpt){
                    direction = "Input";
                }else{
                    direction = "Output";
                }
                cerr << direction << " file name: " + optionVal << ".\n";
            }else{
                cerr << "Bad file name. Terminated.\n";
                return FAIL;
            }
        }else if(option == workersNumberOpt){
            pattern = "\\d{1,2}";
            if(CheckOptionVal() == OK){
                m_parsedOptions[option] = optionVal;
                cerr << "Number of working threads: " + optionVal << ".\n";
            }else{
                cerr << "Bad number of threads. Terminate.\n";
                return FAIL;
            }
        }else if(option == chunksPerThOpt){
            pattern = "\\d{1,2}";
            if(CheckOptionVal() == OK){
                cerr << "Chunks per thread: " + optionVal << ".\n";
            }else{
                cerr << "Bad chunks per thread number. Can be 1-99. Terminate.\n";
                return FAIL;
            }
        }else if(option == decModeOpt){
            pattern = "(^quick$|^slow$|^mixed$){1}";
            if(CheckOptionVal() == OK){
                cerr << "Decode mode: " + optionVal << ".\n";
            }else{
                cerr << "Bad decode mode. Terminate.\n";
                return FAIL;
            }
        }else
        if(option == errorLevelOpt){
            pattern = "[0123]";
            if(CheckOptionVal() == OK){
                cerr << "ECC level: " + optionVal << ".\n" << endl;
            }else{
                cerr << "ECC level can be 0-3. Terminated." << endl;
                return FAIL;
            }
        }else
        if(option == tailOpt){
            pattern = "\\d{1,2}";
            if(CheckOptionVal() == OK){
                cerr << "Number of trailing frames: " + optionVal << ".\n" << endl;
            }else{
                cerr << "Number of trailing frames should be in rage 0-99. Terminated." << endl;
                return FAIL;
            }
        } else
        if(option == qrScaleOpt){
            pattern = "[1-3]?\\d{1}";
            if(CheckOptionVal() == OK){
                cerr << "QR element scaling factor is: " + optionVal << ".\n" << endl;
            }else{
                cerr << "Scaling factor should be an integer from 1 to 4. Terminated." << endl;
                return FAIL;
            }
        }else
        if(option == repeatOpt){
            pattern = "\\d{1,1}";
            if(CheckOptionVal() == OK){
                cerr << "Number of frame repeats: " + optionVal << ".\n" << endl;
            }else{
                cerr << "Number of frame repeats should be in rage 0-9. Terminated." << endl;
                return FAIL;
            }
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

Config* ArgsParserDec::GetConfig(){
    Config* config = new Config;
    memset(config, 0, sizeof(Config));
    return config;
}

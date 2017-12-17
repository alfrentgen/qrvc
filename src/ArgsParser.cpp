#include "ArgsParser.h"

using namespace std;

#define ECC_LEVEL_L 0
#define ECC_LEVEL_M 1
#define ECC_LEVEL_H 2
#define ECC_LEVEL_Q 3

#define MODE_QUICK 0
#define MODE_MIXED 1
#define MODE_SLOW 2

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
                string("-r"),
                string("-n")
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
    string& inverseOpt = m_options[11];//frame repetition

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
        }else if(option == inverseOpt){
            m_parsedOptions[option] = string("1");
            cerr << "Invertion of frames enabled!\n";
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
    Config& config = *(new Config);
    //memset(config, 0, sizeof(Config));

    config.m_frameWidth = 800;
    config.m_frameHeight = 600;
    config.m_counterOn = true;
    config.m_frameRepeats = 1;
    config.m_framesPerThread = 8;
    config.m_nWorkingThreads = 0;
    config.m_nTrailingFrames = 0;
    config.m_qrScale = 1;
    config.m_eccLevel = ECC_LEVEL_L;
    config.m_ifName.clear();
    config.m_ofName.clear();
    config.m_inverseFrame = true;
    config.m_decMode = MODE_MIXED;

    map<string, string>& optionsMap = getOptions();
    //opening IS and OS
    string key = string("-i");
    map<string, string>::iterator it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Input filename is not specified, reading from stdin.\n";
        config.m_ifName.clear();
    }else{
        config.m_ifName = it->second;
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Output filename is not specified, writing to stdout.\n";
        config.m_ofName.clear();
    }else{
        config.m_ofName = it->second;
    }

    key = string("-c");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Counter is disabled.\n";
        config.m_counterOn = false;
    }else{
        cerr << "Counter is enabled.\n";
        config.m_counterOn = true;
    }

    key = string("-f");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No frame size was specified, using 800x600.\n";
        config.m_frameWidth = 800;
        config.m_frameHeight = 600;
    }else{
        string sizeStr = it->second;
        regex exp = regex("\\d{1,4}");
        smatch result;
        uint32_t* widthHeight[2] = {&config.m_frameWidth, &config.m_frameHeight};

        for(int i = 0; regex_search(sizeStr, result, exp) && i < 2; ++i){
            string found = result[0];
            cerr << found << endl;
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(found);
        }
    }

    key = string("-s");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No qr scale was specified, using 1.\n";
        config.m_qrScale = 1;
    }else{
        config.m_qrScale = stoi(it->second);
    }

    key = string("-e");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No ECC level was specified, using the lowest!\n";
        config.m_eccLevel = 0;
    }else{
        uint32_t l= stoi(it->second);
        switch(l){
        case ECC_LEVEL_L:
            config.m_eccLevel = ECC_LEVEL_L;
            break;
        case ECC_LEVEL_M:
            config.m_eccLevel = ECC_LEVEL_M;
            break;
        case ECC_LEVEL_H:
            config.m_eccLevel = ECC_LEVEL_H;
            break;
        case ECC_LEVEL_Q:
            config.m_eccLevel = ECC_LEVEL_Q;
            break;
        default:
            config.m_eccLevel = ECC_LEVEL_L;
            break;
        }
    }

    key = string("-r");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No number of frame repeats was specified, using 0.\n";
        config.m_frameRepeats = 1;
    }else{
        config.m_frameRepeats = stoi(it->second);
        if(config.m_frameRepeats <= 0){
            config.m_frameRepeats = 1;
        }else if(config.m_frameRepeats > 10){
            config.m_frameRepeats = 10;
        }
        cerr << "Number of frame repeats is " << config.m_frameRepeats << endl;
    }

    key = string("-t");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "No number of trailing frame was specified, using 0.\n";
        config.m_nTrailingFrames = 0;
    }else{
        config.m_nTrailingFrames = stoi(it->second);
        if(config.m_nTrailingFrames < 0){
            config.m_nTrailingFrames = 0;
        }else if(config.m_nTrailingFrames > 99){
            config.m_nTrailingFrames = 99;
        }
        cerr << "Number of trailing frames is " << config.m_nTrailingFrames << endl;
    }

    key = string("-p");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        config.m_framesPerThread = stoi(it->second);
    }else{
        config.m_framesPerThread = 8;
    }

    key = string("-w");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        config.m_nWorkingThreads = stoi(it->second);
    }else{
        config.m_nWorkingThreads = 0;
    }

    config.m_decMode = MODE_MIXED;
    key = string("-m");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        if(it->second == string("quick")){
            config.m_decMode = MODE_QUICK;
        }else if(it->second == string("slow")){
            config.m_decMode = MODE_SLOW;
        }else if(it->second == string("mixed")){
            config.m_decMode = MODE_MIXED;
        }
    }

    key = string("-n");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cerr << "Frame colour inversion is disabled.\n";
        config.m_inverseFrame = false;
    }else{
        cerr << "Frame colour inversion is enabled.\n";
        config.m_inverseFrame = true;
    }

    return &config;
}

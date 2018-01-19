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
                string("-n"),
                string("-k"),
                string("-a")
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
    string& cypherOpt = m_options[3];//cyphering on/off
    string& chunksPerThOpt = m_options[4];//chunks per thread
    string& workersNumberOpt = m_options[5];//number of threads
    string& decModeOpt = m_options[6];//decode library
    string& errorLevelOpt = m_options[7];//error correction level
    string& tailOpt = m_options[8];//number of tail frames
    string& qrScaleOpt = m_options[9];//qr scale
    string& repeatOpt = m_options[10];//frame repetition
    string& inverseOpt = m_options[11];//inverse frame
    string& skipOpt = m_options[12];//skip duplicated frames in decoder
    string& alignOpt = m_options[13];//alignment of left top corner

    string optionVal;
    string option;
    regex pattern;

    function<int32_t()> CheckOptionVal = [&](){
        if(argc == n+1 || !argv[n+1][0]){
            LOG("No option value found.\n");
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

        if(option == cypherOpt){
            m_parsedOptions[option] = string("1");
            LOG("Cyphering is enabled!\n");
        } else
        if(option == sizeOpt){
            pattern = "^\\d{1,4}x\\d{1,4}$";
            if(CheckOptionVal() == OK){
                LOG("Frame size: %s\n", optionVal.c_str());
            }else{
                LOG("Bad size format. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == inFileOpt || option == outFileOpt){
            pattern = "^.*$";
            if(CheckOptionVal() == OK){
                string direction;
                if(option == inFileOpt){
                    direction = "Input";
                }else{
                    direction = "Output";
                }
                LOG("%s file name: %s\n", direction.c_str(), optionVal.c_str());
            }else{
                LOG("Bad file name. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == workersNumberOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() == OK){
                m_parsedOptions[option] = optionVal;
                LOG("Number of working threads: %s\n", optionVal.c_str());
            }else{
                LOG("Bad number of threads. Terminate.\n");
                return FAIL;
            }
        } else
        if(option == chunksPerThOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() == OK){
                LOG("Chunks per thread: %s\n", optionVal.c_str());
            }else{
                LOG("Bad chunks per thread number. Can be 1-99. Terminate.\n");
                return FAIL;
            }
        } else
        if(option == decModeOpt){
            pattern = "(^quick$|^slow$|^mixed$){1}";
            if(CheckOptionVal() == OK){
                LOG("Decode mode: %s\n", optionVal.c_str());
            }else{
                LOG("Bad decode mode. Terminate.\n");
                return FAIL;
            }
        } else
        if(option == errorLevelOpt){
            pattern = "^[0123]$";
            if(CheckOptionVal() == OK){
                LOG("ECC level: %s\n", optionVal.c_str());
            }else{
                LOG("ECC level can be 0-3. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == tailOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() == OK){
                LOG("Number of trailing frames: %s\n", optionVal.c_str());
            }else{
                LOG("Number of trailing frames should be in rage 0-99. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == qrScaleOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() == OK){
                LOG("QR element scaling factor is: %s\n", optionVal.c_str());
            }else{
                LOG("Wrong scaling factor, should be an integer from 1 to 99. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == repeatOpt){
            pattern = "^\\d{1,1}$";
            if(CheckOptionVal() == OK){
                LOG("Number of frame repeats: %s\n", optionVal.c_str());
            }else{
                LOG("Number of frame repeats should be in rage 0-9. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == inverseOpt){
            m_parsedOptions[option] = string("1");
            LOG("Invertion of frames enabled!\n");
        } else
        if(option == skipOpt){
            m_parsedOptions[option] = string("1");
            LOG("Duplicate frames skipping enabled!\n");
        }else
        if(option == alignOpt){
            pattern = "^\\d{1,3}$";
            if(CheckOptionVal() == OK){
                LOG("Number of frame repeats: %s\n", optionVal.c_str());
            }else{
                LOG("Number of frame repeats should be in rage 0-999. Terminated.\n");
                return FAIL;
            }
        }
        else{
            LOG("Wrong option. Terminated!\n");
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
    config.m_cypherOn = false;
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
        LOG("Input filename is not specified, reading from stdin.\n");
        config.m_ifName.clear();
    }else{
        config.m_ifName = it->second;
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("Output filename is not specified, writing to stdout.\n");
        config.m_ofName.clear();
    }else{
        config.m_ofName = it->second;
    }

    key = string("-c");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("Cyphering is disabled.\n");
        config.m_cypherOn = false;
    }else{
        LOG("Cyphering is enabled.\n");
        config.m_cypherOn = true;
    }

    key = string("-f");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("No frame size was specified, using 1280x720.\n");
        config.m_frameWidth = 1280;
        config.m_frameHeight = 720;
    }else{
        string sizeStr = it->second;
        regex exp = regex("\\d{1,4}");
        smatch result;
        int32_t* widthHeight[2] = {&config.m_frameWidth, &config.m_frameHeight};

        for(int i = 0; regex_search(sizeStr, result, exp) && i < 2; ++i){
            string found = result[0];
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(found);
        }
    }

    key = string("-s");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("No qr scale was specified, using 4.\n");
        config.m_qrScale = 4;
    }else{
        config.m_qrScale = stoi(it->second);
    }

    key = string("-e");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("No ECC level was specified, using the lowest!\n");
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
        LOG("No number of frame repeats was specified, using 1.\n");
        config.m_frameRepeats = 1;
    }else{
        config.m_frameRepeats = stoi(it->second);
        if(config.m_frameRepeats <= 0){
            config.m_frameRepeats = 1;
        }else if(config.m_frameRepeats > 99){
            config.m_frameRepeats = 100;
        }
        //LOG("Number of frame repeats is %d\n", config.m_frameRepeats);
    }

    key = string("-t");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("No number of trailing frame was specified, using 0.\n");
        config.m_nTrailingFrames = 0;
    }else{
        config.m_nTrailingFrames = stoi(it->second);
        if(config.m_nTrailingFrames < 0){
            config.m_nTrailingFrames = 0;
        }else if(config.m_nTrailingFrames > 99){
            config.m_nTrailingFrames = 100;
        }
        //LOG("Number of trailing frames is %d\n", config.m_nTrailingFrames);
    }

    key = string("-p");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        config.m_framesPerThread = stoi(it->second);
    }else{
        LOG("No number of frames per thread was specified, using 8.\n");
        config.m_framesPerThread = 8;
    }

    key = string("-w");
    it = optionsMap.find(key);
    if(it != optionsMap.end()){
        config.m_nWorkingThreads = stoi(it->second);
    }else{
        LOG("No number of working threads was specified, trying hardware cores number if available or 2 otherwise.\n");
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
    }else{
        LOG("No mode was specified, using mixed mode.\n");
    }

    key = string("-n");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("Frame colour inversion is disabled.\n");
        config.m_inverseFrame = false;
    }else{
        LOG("Frame colour inversion is enabled.\n");
        config.m_inverseFrame = true;
    }

    key = string("-k");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("Duplicated frames skipping is disabled.\n");
        config.m_skipDupFrames = false;
    }else{
        LOG("Duplicated frames skipping is enabled.\n");
        config.m_skipDupFrames = true;
    }

    key = string("-a");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        LOG("No alignment value was specified, using 0.\n");
        config.m_alignment = 0;
    }else{
        config.m_alignment = stoi(it->second);
        if(config.m_alignment <= 0){
            config.m_alignment = 0;
        }else if(config.m_alignment > 99){
            config.m_alignment = 100;
        }
        //LOG("Number of frame repeats is %d\n", config.m_frameRepeats);
    }

    return &config;
}

#include "ArgsParser.h"

using namespace std;

vector<string> g_options({ string("-i"),
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
            string("-a"),
            string("--stg")
            });

string& inFileOpt = g_options[0];//input file name
string& outFileOpt = g_options[1];//output file name
string& sizeOpt = g_options[2];//frame size -s NxN
string& cypherOpt = g_options[3];//cyphering on/off
string& chunksPerThOpt = g_options[4];//chunks per thread
string& workersNumberOpt = g_options[5];//number of threads
string& decModeOpt = g_options[6];//decode library
string& errorLevelOpt = g_options[7];//error correction level
string& tailOpt = g_options[8];//number of tail frames
string& qrScaleOpt = g_options[9];//qr scale
string& repeatOpt = g_options[10];//frame repetition
string& inverseOpt = g_options[11];//inverse frame
string& skipOpt = g_options[12];//skip duplicated frames in decoder
string& alignOpt = g_options[13];//alignment of left top corner
string& stegOpt = g_options[14];//steganography option

map<const char*, const char*> g_stegOpts({ { "th", "\\d{1,3}$"} , {"kf", ".*$" },  {"up", "[oxj]$"} });
string stgThOpt("th");
string stgKeyFileOpt("kf");
string stgUnitPatOpt("up");

ArgsParser::ArgsParser()
{
    //ctor
}

ArgsParser::~ArgsParser()
{
    //dtor.h
}

int32_t ArgsParser::ParseStegParams(vector<string>& params){
    if(params.size() == 0){
        return;
    }

    for(string param : params){
        bool found = false;
        for (/*map<const char*, const char*>::iterator*/auto it = g_stegOpts.begin(); it != g_stegOpts.end(); ++it){
            string leftPat = string("^(") + string(it->first) + string("=)");
            string rightPat = string("(") + string(it->second) + string(")$");
            string strPat = leftPat + rightPat;
            regex pattern(strPat);
            smatch m;
            //LOG("%s\n", param.c_str());
            //LOG("%s\n", strPat.c_str());
            if(regex_search(param, m, pattern)){
                //LOG("For param \"%s\" parsed: \"%s\"\n", param.c_str(), m[2]);
                m_parsedOptions[string(it->first)] = m[2];
                found = true;
                break;
            }
        }
        if(found == false){
            //LOG("Nothing has been parsed for: \"%s\"\n", param.c_str());
            return FAIL;
        }
    }
    return OK;
}

int ArgsParser::parseOptions(int argc, char **argv){
    int n = 0;

    string optionVal;
    string option;
    regex pattern;

    function<int32_t()> CheckOptionVal = [&](){
        if(argc == n+1 || !argv[n+1][0]){
            LOG("No option value found.\n");
            return FAIL;
        }else {
            optionVal = string(argv[n+1]);

            if(!ArgsParser::IsOptionName(optionVal) && regex_match(optionVal, pattern)){
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

        if(option == cypherOpt){
            pattern = "^.*$";
            if(CheckOptionVal() != OK){
                optionVal.clear();//empty file name means no special file for key frame
                m_parsedOptions[option] = optionVal;
            }
        } else
        if(option == sizeOpt){
            pattern = "^\\d{1,4}x\\d{1,4}$";
            if(CheckOptionVal() != OK){
                LOG("Bad size format. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == inFileOpt || option == outFileOpt){
            pattern = "^.*$";
            if(CheckOptionVal() != OK){
                LOG("Bad file name. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == workersNumberOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() != OK){
                LOG("Bad number of threads. Terminate.\n");
                return FAIL;
            }
        } else
        if(option == chunksPerThOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() != OK){
                LOG("Bad chunks per thread number. Can be 1-99. Terminate.\n");
                return FAIL;
            }
        } else
        if(option == decModeOpt){
            pattern = "(^quick$|^slow$|^mixed$){1}";
            if(CheckOptionVal() != OK){
                LOG("Bad decode mode. Terminate.\n");
                return FAIL;
            }
        } else
        if(option == errorLevelOpt){
            pattern = "^[0123]$";
            if(CheckOptionVal() != OK){
                LOG("ECC level can be 0-3. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == tailOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() != OK){
                LOG("Number of trailing frames should be in rage 0-99. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == qrScaleOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() != OK){
                LOG("Wrong scaling factor, should be an integer from 1 to 99. Terminated.\n");
                return FAIL;
            }
        } else
        if(option == repeatOpt){
            pattern = "^\\d$";
            if(CheckOptionVal() != OK){
                LOG("Number of frame repeats should be in rage 0-9. Terminated.\n");
                return FAIL;
            }
        } else
        /*if(option == inverseOpt){
            m_parsedOptions[option] = string("1");
            //LOG("Invertion of frames enabled!\n");
        } else*/
        if(option == skipOpt){
            m_parsedOptions[option] = string("1");
        }else
        if(option == alignOpt){
            pattern = "^\\d{1,2}$";
            if(CheckOptionVal() != OK){
                LOG("Code alignment should be in 0-99. Terminated.\n");
                return FAIL;
            }
        }else
        if(option == stegOpt){
            m_parsedOptions[option] = string("1");
            ++n;
            vector<string> stegParams(0);
            while(n < argc){
                string param = string(argv[n]);

                if(IsOptionName(param)){
                    n--;
                    break;
                }
                stegParams.push_back(param);
                n++;
            }
            if(ParseStegParams(stegParams) != OK){
                LOG("Wrong steganography parameters given!\n");
                return FAIL;
            }
        }
        else{
            LOG("Wrong option. Terminated!\n");
            LOG("Stopped on option %s\n", option.c_str());
            return FAIL;
        }
    }
    return n;
}

map<string, string>& ArgsParser::getOptions(){
    return m_parsedOptions;
}

bool ArgsParser::IsOptionName(string& str){
    for (auto & element : g_options) {
        if(element == str){
            //cerr << element << ": option value missed\n";
            return true;
        }
    }
    return false;
}

Config* ArgsParser::GetConfig(){
    Config& config = *(new Config);
    config.m_frameWidth = 1280;
    config.m_frameHeight = 720;
    config.m_cipheringOn = false;
    config.m_frameRepeats = 1;
    config.m_framesPerThread = 8;
    config.m_nWorkingThreads = 0;
    config.m_nTrailingFrames = 0;
    config.m_qrVersion = 0;
    config.m_qrScale = 4;
    config.m_eccLevel = ECC_LEVEL_L;
    config.m_ifName.clear();
    config.m_ofName.clear();
    config.m_keyFileName.clear();
    config.m_inverseFrame = false;
    config.m_decMode = MODE_MIXED;
    config.m_skipDupFrames = false;

    map<string, string>& optionsMap = getOptions();
    //opening IS and OS
    map<string, string>::iterator it = optionsMap.find(inFileOpt);
    if(it == optionsMap.end()){
        config.m_ifName.clear();
    }else{
        config.m_ifName = it->second;
    }

    it = optionsMap.find(outFileOpt);
    if(it == optionsMap.end()){
        config.m_ofName.clear();
    }else{
        config.m_ofName = it->second;
    }

    it = optionsMap.find(cypherOpt);
    if(it == optionsMap.end()){
        config.m_cipheringOn = false;
        config.m_keyFileName.clear();
    }else{
        config.m_cipheringOn = true;
        config.m_keyFileName = it->second;
    }

    it = optionsMap.find(sizeOpt);
    if(it == optionsMap.end()){
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

    it = optionsMap.find(qrScaleOpt);
    if(it == optionsMap.end()){
        config.m_qrScale = 4;
    }else{
        config.m_qrScale = stoi(it->second);
    }

    it = optionsMap.find(errorLevelOpt);
    if(it == optionsMap.end()){
        config.m_eccLevel = ECC_LEVEL_L;
    }else{
        uint32_t l = stoi(it->second);
        config.m_eccLevel = l;
        /*switch(l){
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
        }*/
    }

    it = optionsMap.find(repeatOpt);
    if(it == optionsMap.end()){
        config.m_frameRepeats = 1;
    }else{
        config.m_frameRepeats = stoi(it->second);
        if(config.m_frameRepeats <= 0){
            config.m_frameRepeats = 1;
        }else if(config.m_frameRepeats > 99){
            config.m_frameRepeats = 100;
        }
    }

    it = optionsMap.find(tailOpt);
    if(it == optionsMap.end()){
        config.m_nTrailingFrames = 10;
    }else{
        config.m_nTrailingFrames = stoi(it->second);
        if(config.m_nTrailingFrames < 0){
            config.m_nTrailingFrames = 0;
        }else if(config.m_nTrailingFrames > 99){
            config.m_nTrailingFrames = 100;
        }
    }

    it = optionsMap.find(chunksPerThOpt);
    if(it != optionsMap.end()){
        config.m_framesPerThread = stoi(it->second);
    }else{
        config.m_framesPerThread = 8;
    }

    it = optionsMap.find(workersNumberOpt);
    if(it != optionsMap.end()){
        config.m_nWorkingThreads = stoi(it->second);
    }else{
        config.m_nWorkingThreads = 0;
    }
    if(config.m_nWorkingThreads <= 0){
        config.m_nWorkingThreads = std::thread::hardware_concurrency();
        config.m_nWorkingThreads = (config.m_nWorkingThreads == 0) ? 2 : config.m_nWorkingThreads;
    }

    it = optionsMap.find(decModeOpt);
    if(it != optionsMap.end()){
        if(it->second == string("quick")){
            config.m_decMode = MODE_QUICK;
        }else if(it->second == string("slow")){
            config.m_decMode = MODE_SLOW;
        }else if(it->second == string("mixed")){
            config.m_decMode = MODE_MIXED;
        }
    }else{
        config.m_decMode = MODE_MIXED;
    }

    /*option = string("-n");
    it = optionsMap.find(option);
    if(it == optionsMap.end()){
        LOG("Frame colour inversion is disabled.\n");
        config.m_inverseFrame = false;
    }else{
        LOG("Frame colour inversion is enabled.\n");
        config.m_inverseFrame = true;
    }*/

    it = optionsMap.find(skipOpt);
    if(it == optionsMap.end()){
        config.m_skipDupFrames = false;
    }else{
        config.m_skipDupFrames = true;
    }

    it = optionsMap.find(alignOpt);
    if(it == optionsMap.end()){
        config.m_alignment = 8;
    }else{
        config.m_alignment = stoi(it->second);
        if(config.m_alignment <= 0){
            config.m_alignment = 0;
        }else if(config.m_alignment > 99){
            config.m_alignment = 100;
        }
    }

    it = optionsMap.find(stegOpt);
    if(it == optionsMap.end()){
        config.m_stegModeOn = false;
        config.m_stegThreshold = 0;
    }else{
        config.m_stegModeOn = true;
        it = optionsMap.find(stgThOpt);
        if(it == optionsMap.end()){
            config.m_stegThreshold = 8;
        }else{
            config.m_stegThreshold = stoi(it->second);
        }

        it = optionsMap.find(stgKeyFileOpt);
        if(it == optionsMap.end()){
            config.m_keyFileName = string();//string(config.m_ifName) + string(.stg);
        }else{
            config.m_keyFileName = it->second;
        }

        it = optionsMap.find(stgUnitPatOpt);
        if(it == optionsMap.end()){
            config.m_unitPattern = 'o';
        }else{
            config.m_unitPattern = it->second[0];
            //LOG("up=\"%c\"\n", config.m_unitPattern);
        }
    }

    return &config;
}

#include "ArgsParser.h"
//#include "pch.h"

using namespace std;

ArgsParserEnc::ArgsParserEnc()
{}

ArgsParserEnc::~ArgsParserEnc()
{
    //dtor.h
}

/*decoder options to parse:
counter on/off -c
output file name -o
output file name -i
output frame size -f NxN
qr code scale 1,2 ... etc -s
ecc level 1..4 -l
Numberof frame repeats -r
*/

int ArgsParserEnc::parseOptions(int argc, char **argv){
    int n = 0;
    string inFileOpt = string("-i");
    string outFileOpt = string("-o");
    string counterOpt = string("-c");
    string frameSizeOpt = string("-f");
    string qrScaleOpt = string("-s");
    string levelOpt = string("-l");
    string repeatOpt = string("-r");
    string tailOpt = string("-t");

    for(n = 1; n < argc; n++){
        string option = string(argv[n]);
        string optionVal;
        cerr << "Option: " << option << "\n";
        //a cyclic check for every available option
        if(option == counterOpt){
            m_parsed[option] = string("1");
            cerr << "Counter - OK!\n";

        }else
        if(option == frameSizeOpt){
            regex sizePattern = regex("\\d{1,4}x\\d{1,4}");
            if(argc == n+1){
                cerr << "No size string found" << endl;
                return -1;
            }else{
                string optionVal = string(argv[n+1]);
                if(regex_match(optionVal, sizePattern)){
                    m_parsed[option] = optionVal;
                    cerr << "Size pattern matches! Size string: " + optionVal << endl;
                    ++n;
                }else{
                    cerr << "Size string should be in \"WidthxHeight\" format. Terminated." << endl;
                    return -1;
                }
            }
            cerr << "Size - OK!\n";

        }else
        if(option == inFileOpt || option == outFileOpt){
            if(argc == n+1 || !argv[n+1][0]){
                cerr << "No file name was specified. Terminated." << endl;
                return -1;
            }
            optionVal = string(argv[n+1]);
            if(optionVal == inFileOpt || optionVal == outFileOpt
                || optionVal == counterOpt || optionVal == frameSizeOpt
                || optionVal == qrScaleOpt || optionVal == levelOpt){
                cerr << "No file name was specified. Terminated." << endl;
                return -1;
            }
            cerr << "Input file name: " + optionVal << endl;
            m_parsed[option] = optionVal;
            ++n;
            cerr << "OK!\n";

        }else
        if(option == qrScaleOpt){
            regex scalePattern = regex("[1-3]?\\d{1}");
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
            cerr << "Scale - OK!\n";

        }else
        if(option == levelOpt){
            regex bytesPattern = regex("[0123]");
            if(argc == n+1){
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
            cerr << "Level - OK!\n";
        }else
        if(option == repeatOpt){
            regex repeatPattern = regex("\\d{1,1}");
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
            }
        }else
        if(option == tailOpt){
            regex tailPattern = regex("\\d{1,2}");
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
            }
        }
        else{
            cerr << "Wrong option. Terminated!\n";
            exit(0);
        }
    }
    return n;
}

map<string, string>* ArgsParserEnc::getOptions(){
    return &m_parsed;
}

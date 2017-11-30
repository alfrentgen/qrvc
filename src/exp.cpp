#include "pch.h"
#include "ArgsParser.h"

using namespace std;

int _main (int argc, char **argv){
    ArgsParserDec* ap = new ArgsParserDec();

    ap->parseOptions(argc, argv);

    map<string, string>& optionsMap = *ap->getOptions();

    uint32_t frameWidth = 0;
    uint32_t frameHeight = 0;
    bool frameCounter = 0;
    istream* inputStream = &cin;
    ostream* outputStream = &cout;

    string key = string("-i");
    map<string, string>::iterator it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "Input filename is not specified, reading from stdin.\n";
    }else{
        inputStream = new ifstream(it->second, ios_base::in | ios_base::binary);
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "Output filename is not specified, writing to stdout.\n";
    }else{
        outputStream = new ofstream(it->second, ios_base::out | ios_base::binary);
    }

    inputStream->tie(outputStream);

    key = string("-c");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "Counter disabled.\n";
    }else{
        cout << "Counter enabled.\n";
        frameCounter = 1;
    }

    key = string("-s");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "No frame size specified, using 800x600";
        frameWidth = 800;
        frameHeight = 600;
    }else{
        string sizeStr = it->second;
        regex exp = regex("\\d{1,4}");
        smatch result;
        uint32_t* widthHeight[2] = {&frameWidth, &frameHeight};

        for(int i = 0; regex_search(sizeStr, result, exp) && i < 2; ++i){
            cout << result[0] << endl;
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(result[0]);
        }
    }

    vector<uint8_t> rawFrame = vector<uint8_t>(frameWidth * frameHeight);

    delete ap;
    return 0;
}

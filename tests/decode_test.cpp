#include <pch.h>
#include <Decode.h>
#include <ArgsParser.h>
#include <inputQueue.h>
#include <OutputQueue.h>
#include <cstdlib>
#include <chrono>

using namespace std;

int main(int argc, char** argv){

    uint32_t frameWidth = 0;
    uint32_t frameHeight = 0;
    string inputFileName;
    string outputFileName;

    ArgsParserDec ap = ArgsParserDec();
    if(ap.parseOptions(argc, argv) == FAIL)
        return FAIL;

    map<string, string> optionsMap = ap.getOptions();

    string key = string("-f");
    map<string, string>::iterator it = optionsMap.find(key);
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
            string found = result[0];
            cout << found << endl;
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(found);
        }
    }

    key = string("-i");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "Input filename is not specified, reading from \"in.qr\".\n";
        inputFileName = string("in.qr");
    }else{
        inputFileName = it->second;
    }

    key = string("-o");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "Output filename is not specified, writing to \"out.qr\".\n";
        inputFileName = string("out.qr");
    }else{
        outputFileName = it->second;
    }

    system("pwd");
    istream* inputStream = new ifstream(inputFileName, ios_base::in | ios_base::binary);
    ostream* outputStream = new ofstream(outputFileName, ios_base::out | ios_base::binary);

    InputQueue inQ(inputStream, 100, frameWidth * frameHeight);
    OutputQueue outQ(outputStream);

    auto start = chrono::steady_clock::now();
    Decode dec(frameWidth, frameHeight, &inQ, &outQ);
    dec.Do();
    auto end = chrono::steady_clock::now();

    auto delta = chrono::duration_cast<chrono::microseconds>(start - end).count();
    cout << delta << "micro seconds\n";
    return 0;
}

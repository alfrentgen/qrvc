#include <pch.h>
#include <Decode.h>
#include <ArgsParser.h>
#include <inputQueue.h>
#include <OutputQueue.h>
#include <cstdlib>
#include <chrono>
#include "utilities.h"

#define CLASS Decode
#define classname(NAME) #NAME
#define CLASS_NAME(NAME) classname(NAME)

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

    istream* inputStream = new ifstream(inputFileName, ios_base::in | ios_base::binary);
    ostream* outputStream = new ofstream(outputFileName, ios_base::out | ios_base::binary);
    /*char rdBuffer[frameWidth * frameHeight];
    size_t buffSize = frameWidth * frameHeight;
    inputStream->rdbuf()->setbuf(rdBuffer, buffSize);*/

    uint32_t nThreads = std::thread::hardware_concurrency();
    //nThreads = 1;
    cout << "nThreads: " << nThreads << endl;
    cout << "Using: " << CLASS_NAME(CLASS) << endl;

    InputQueue inQ(inputStream, nThreads * 50, frameWidth * frameHeight);
    OutputQueue outQ(outputStream);

    Decode* decs[nThreads];
    thread threads[nThreads];

    for(int i =0; i < nThreads; i++){
        decs[i] = new CLASS(frameWidth, frameHeight, &inQ, &outQ);
    }

    START_TIME_MEASURING;
    try{
        for(int i =0; i < nThreads; i++){
            threads[i] = thread(&Decode::Do, decs[i]);
        }

        for(int i =0; i < nThreads; i++){
            threads[i].join();
        }
    }catch(exception& e){
        cout << e.what() << endl;
    }

    STOP_TIME_MEASURING(milliseconds);

    cout << "nThreads = " << nThreads << endl;
    PRINT_MEASURED_TIME("The whole decoding has taken: ", milliseconds);
    return 0;
}

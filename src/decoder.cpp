#include "pch.h"
#include "ArgsParser.h"
#define STR(s) #s

using namespace std;
using namespace zbar;

int main (int argc, char **argv)
{

    ArgsParserDec ap = ArgsParserDec();
    if(ap.parseOptions(argc, argv) == FAIL)
        return FAIL;

    map<string, string> optionsMap = ap.getOptions();

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

    //inputStream->tie(outputStream);

    key = string("-c");
    it = optionsMap.find(key);
    if(it == optionsMap.end()){
        cout << "Counter disabled.\n";
    }else{
        cout << "Counter enabled.\n";
        frameCounter = 1;
    }

    key = string("-f");
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
            string found = result[0];
            cout << found << endl;
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(found);
            /*cout << result[0] << endl;
            sizeStr = result.suffix().str();
            *(widthHeight[i]) = stoi(result[0]);*/
        }
    }

    vector<uint8_t> rawFrame = vector<uint8_t>(frameWidth * frameHeight);

    // create a reader
    ImageScanner scanner;
    scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    cout << "frameWidth = " << frameWidth << endl;
    cout << "frameHeight = " << frameHeight << endl;
    Image image(frameWidth, frameHeight, string("GREY"), rawFrame.data(), frameWidth * frameHeight);

    // read from binary file:
    int32_t counter = 1;
    int32_t iterCounter = 1;
    while(inputStream->read((char*)rawFrame.data(), rawFrame.size())){
        if(inputStream->gcount() != rawFrame.size()){
            cout << "Incomplete frame has been received!\n";
        }
        //cout << "bytes read: " << inputStream->gcount() << endl;
        /*scanner.recycle_image(image);
        image.set_data(rawFrame.data(), rawFrame.size());
        image.set_size(frameWidth, frameHeight);
        image.set_format(string("GREY"));*/
        scanner.scan(image);
        /*cout << iterCounter << " iteration!\n";
        if(!scanner.scan(image)){
            cout << counter << " can't scan!\n";
            counter++;
        }*/
        //cout << "scanner.scan(image): " << scanner.scan(image) << endl;
        //cout << counter++ << endl;
        for(Image::SymbolIterator symbol = image.symbol_begin();
            symbol != image.symbol_end(); ++symbol) {
            // do something useful with results
            /*string d = symbol->get_data();
            cout << d.size() << endl;*/
            (*outputStream) << symbol->get_data();
            /*cout << "decoded " << symbol->get_type_name()
                 << " symbol \"" << symbol->get_data() << '"' << endl;*/
            outputStream->flush();
        }
        iterCounter++;
    }

    outputStream->flush();
    image.set_data(NULL, 0);
    return(0);
}

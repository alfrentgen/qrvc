#include <pch.h>
#include <Chunk.h>
//#include <cstdlib>
#include <chrono>
#include <random>

using namespace std;

int main(int argc, char** argv){

    std::default_random_engine generator;
    std::uniform_int_distribution<uint8_t> distribution(0, 255);

    Chunk ch = Chunk();
    vector<uint8_t> rnd_vec = vector<uint8_t>(1024, 0);
    for(int i = 0; i< rnd_vec.size(); i++){
        rnd_vec[i] = distribution(generator);
        cout << rnd_vec[i] << endl;
    }

    auto start = chrono::steady_clock::now();
    ch.CalcHashsum(rnd_vec.data(), rnd_vec.size());
    auto end = chrono::steady_clock::now();

    auto delta = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    cout << delta << " nanoseconds\n";
    return 0;
}

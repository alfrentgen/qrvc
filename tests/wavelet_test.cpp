#include "pch.h"

using namespace std;
#include "wavelet.h"
#define DCT_THRESHOLD 2
#define N_REPEATS 1//77 * 177 * 1000

template<typename T, uint32_t dim>
void rand_fill_in(T* arr){
    srand (time(NULL));
    for(int i = 0; i < dim*dim; i++){
        arr[i] = rand() % (256*8);;
    }
}

template<typename T, uint32_t dim>
void print_array(T a[dim][dim]){
    LOG("\n");
    for(int row = 0; row < dim; row++){
        for(int col = 0; col < dim; col++){
            LOG("%d, ", a[row][col]);
        }
        LOG("\n");
    }
}

void print_array_double(double* a, uint32_t dim){
    LOG("\n");
    for(int row = 0; row < dim; row++){
        for(int col = 0; col < dim; col++){
            LOG("%g, ", *(a + row * dim + col));
        }
        LOG("\n");
    }
}

template<typename T, uint32_t dim>
bool compare_2_arr(T a[dim][dim], T b[dim][dim]){
    for(int row = 0; row < dim; row++){
        for(int col = 0; col < dim; col++){
            if(abs(a[row][col] - b[row][col]) > DCT_THRESHOLD){
                LOG("At [%d][%d]: %d != %d\n\n", row, col, a[row][col], b[row][col]);
                return false;
            }
        }
    }
    return true;
}
template<typename T>
T calc_average(vector<T> vec){
    if(vec.size() == 0)
        return 0;
    T res = 0;
    for(T v : vec){
        res += v;
    }
    return res/vec.size();
}


int32_t main(){
    vector<int32_t> image;
    vector<int32_t> buffer;
    int32_t coeffs[4][4] = {0};
    uint8_t out[4][4] = {0};
    for(int i = 0; i < N_REPEATS; i++){
        //LOG("test#%d\n", i);
        image.resize(16, 0);
        buffer.resize(16, 0);
        rand_fill_in<int32_t, 4>(image.data());
        int32_t simple_avg = calc_average(image);
        uint32_t level =0;
        hwt_fwd(image, buffer, 4, 4, level);
        LOG("Image transform average: %d\n", image.back());
        LOG("Image simple average: %d\n", simple_avg);
        /*if(!compare_2_arr<uint8_t, 4>(in, out)){
            LOG("test#%d\n", i);
            LOG("\nInput array:");
            print_array<uint8_t,4>(in);
            LOG("\nOutput array:");
            print_array<uint8_t,4>(out);
            LOG("\nDCT coefficients:");
            print_array<int32_t,4>(coeffs);
            exit(0);
        }*/
    }

    return 0;
}

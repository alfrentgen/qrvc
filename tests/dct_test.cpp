#include "pch.h"
#include "dct.h"
#define DCT_THRESHOLD 1
#define N_REPEATS 177 * 177 * 1000

template<typename T, uint32_t dim>
void rand_fill_in(T arr[dim][dim]){
    srand (time(NULL));
    for(int row = 0; row < dim; row++){
        for(int col = 0; col < dim; col++){
            arr[row][col] = rand() % 256;
        }
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

int32_t main(){
    uint8_t in[4][4] = {0};
    int32_t coeffs[4][4] = {0};
    uint8_t out[4][4] = {0};
    uint64_t nFails(0);
    for(int i = 0; i < N_REPEATS; i++){
        //LOG("test#%d\n", i);
        rand_fill_in(in);
        fwd_4x4_dct<int32_t>(in, coeffs);
        inv_4x4_dct<int32_t>(coeffs, out);

        if(!compare_2_arr<uint8_t, 4>(in, out)){
            LOG("test#%d\n", i);
            LOG("\nInput array:");
            print_array<uint8_t,4>(in);
            LOG("\nOutput array:");
            print_array<uint8_t,4>(out);
            LOG("\nDCT coefficients:");
            print_array<int32_t,4>(coeffs);
            nFails++;
            //exit(0);
        }
    }
    LOG("\nThe test has failed %d out of %d times. \n", nFails, N_REPEATS);
    return 0;
}

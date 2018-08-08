#include "dct.h"
//#include "naive-dct.h"

extern "C"{
    double *NaiveDct_transform(double vector[], size_t len);
    double *NaiveDct_inverseTransform(double vector[], size_t len);
}

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
            if(a[row][col] != b[row][col]){
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
    for(int i = 0; i < 1000; i++){
        LOG("test#%d\n", i);
        rand_fill_in(in);
        fwd_4x4_dct<int32_t>(in, coeffs);
        inv_4x4_dct<int32_t>(coeffs, out);

        if(!compare_2_arr<uint8_t, 4>(in, out)){
            LOG("\nInput array:");
            print_array<uint8_t,4>(in);
            LOG("\nOutput array:");
            print_array<uint8_t,4>(out);
            LOG("\nDCT coefficients:");
            print_array<int32_t,4>(coeffs);
            double d_in[16];
            uint8_t* p_in = (uint8_t*)in;
            for(int i = 0; i < 16; i++){
                d_in[i] = (double)(p_in[i]-128);
            }
            print_array_double(d_in, 4);
            double* d_coeffs;
            d_coeffs = NaiveDct_transform(d_in, 16);
            for(int i = 0; i < 16; i++){
                d_coeffs[i] /= 8.0;
            }
            print_array_double(d_coeffs, 4);
            double* d_out;

            d_out = NaiveDct_inverseTransform((double*)d_coeffs, 16);
            print_array_double(d_out, 4);
            exit(0);
        }
    }
    return 0;
}

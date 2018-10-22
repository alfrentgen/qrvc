#include "pch.h"

using namespace std;
#include "wavelet.h"
#define DCT_THRESHOLD 2
#define N_REPEATS 10//77 * 177 * 1000
#define ACCURACY 8
#define EMBED_POS 2
#define RANGE 256

void print_array_double(double* a, uint32_t dim){
    LOG("\n");
    for(int row = 0; row < dim; row++){
        for(int col = 0; col < dim; col++){
            LOG("%g, ", *(a + row * dim + col));
        }
        LOG("\n");
    }
}

template<typename T>
void print_array(T* a, uint32_t dim){
    LOG("\n");
    for(int row = 0; row < dim; row++){
        for(int col = 0; col < dim; col++){
            LOG("%d, ", a[dim*row + col]);
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
double calc_average(vector<T> vec, size_t n){
    if(vec.size() == 0)
        return 0;
    if(n == 0){
        n = vec.size();
    }
    double res = 0;
    for(int i=0; i < n; i++){
        res += vec[i];
    }
    return res/n;
}

template<typename T>
void rand_fill_in(T* arr, size_t length, uint32_t range){
    for(int i = 0; i < length; i++){
        arr[i] = rand() % RANGE;
    }
}

template<typename T>
void increase_accuracy(T* data, uint32_t size, uint32_t shifts){
    for(int i = 0; i < size; i++){
        data[i] = (data[i] << shifts);
    }
}

template<typename T>
void decrease_accuracy(T* data, uint32_t size, uint32_t shifts){
    for(int i = 0; i < size; i++){
        data[i] = (data[i] >> shifts);
    }
}

template<typename T>
void round_int(T* data, uint32_t size, uint32_t fract_pos){
    T unit = (1<<fract_pos);
    T half_unit = DIV_2(unit);
    if(half_unit == 0){
        return;
    }
    T mantissa;
    T integral;
    T mask = (~T(0))<<fract_pos;
    T value;
    for(int i = 0; i < size; i++){
        integral = data[i] & mask;
        mantissa = abs(data[i] - integral);
        value = data[i];
        if(mantissa > half_unit){
            integral = integral>>(fract_pos);
            data[i] = (integral > 0) ? integral+1 : integral-1;
        }else{
            data[i] = integral>>(fract_pos);
        }
    }
}

template<typename T>
void embed_bits(T* value, T bits){
    *value = *value | bits;
}

int32_t main(){
    vector<int32_t> image;
    vector<int32_t> transform;
    vector<int32_t> restored_image;
    srand (time(NULL));
    for(int i = 0; i < N_REPEATS; i++){
        //LOG("test#%d\n", i);
        image.resize(16, 0);
        rand_fill_in(image.data(), image.size(), RANGE);
        LOG("\nOriginal:");
        print_array(image.data(), 4);
        double orig_avg = calc_average(image, 16);
        LOG("Original avg: %g\n", orig_avg);
        increase_accuracy(image.data(), 16, ACCURACY);
        hwt_4x4_fwd(image);
        transform.assign(image.begin(), image.end());

        int32_t& val = transform.back();
        uint32_t bit = (uint32_t)(0x01<<(EMBED_POS + ACCURACY - 1));
        int32_t mask = 0xffffffff << (EMBED_POS + ACCURACY - 1);
        val ^= bit;//invert bit to be embedded
        val &= mask;//nullify less meaning bits
        hwt_4x4_inv(transform);
        round_int(transform.data(), 16, ACCURACY);
        //decrease_accuracy(transform.data(), 16, ACCURACY);
        LOG("\nRestored:");
        print_array(transform.data(), 4);
        double trn_avg = calc_average(transform, 16);
        LOG("Transform avg: %g\n", trn_avg);
    }

    return 0;
}

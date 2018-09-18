#pragma once

#define MUL_2(X) (X << 1)
#define MUL_4(X) (X << 2)
#define DIV_2(X) (X >> 1)
#define DIV_4(X) (X >> 2)

//#define ENBALE_OPT_DIV
#ifdef ENBALE_OPT_DIV
#define DIV_10(X) ((X * 205)>>11)
#else
#define DIV_10(X) (X / 10)
#endif

template<typename T>
int32_t hwt_fwd(vector<T> image, uint32_t width, uint32_t hight, uint32_t depth){
    /*uint32_t hfolds;//check if the transform of depth given is possible at all
    if(){

    }*/
    for(uint32_t row = 0; row < hight; row++){
        for(int col = 0; col < width; col++){
            ;
        }
    }
    return 0;
}

template<typename T>
int32_t hwt_inv(vector<T> decomposition, uint32_t width, uint32_t hight, uint32_t depth){
    for(uint32_t row = 0; row < hight; row++){
        for(int col = 0; col < width; col++){
            ;
        }
    }
    return 0;
}

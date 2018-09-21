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
/*
uint32_t get_max_level(uint32_t width, uint32_t hight){
    uint32_t smallest_size = (width > hight) ? hight : width;

    uint32_t level = 0;
    while((smallest_size=>>1) != 0){
        depth
    }
}*/

template<typename T>
int32_t hwt_fwd(vector<T>& image, vector<T>& buffer, uint32_t width, uint32_t height, uint32_t level){
    if(height<2 || width<2){
        return 0;
    }

    uint32_t nextWidth = width/2;
    uint32_t nextHeight = height/2;
    buffer.resize(2 * nextWidth * 2 * nextHeight);

    //fold vertically
    uint64_t offset_vert = nextHeight * 2 * nextWidth;
    uint32_t stride = width;
    T* pBuffTop = buffer.data();
    T* pBuffBottom = buffer.data() + offset_vert;
    for(uint32_t col = 0; col < 2*nextWidth; col++){
        for(uint32_t row = 0; row < nextHeight; row++){
            *pBuffTop = (image[stride * (2*row) + col] - image[stride * (2*row+1) + col])/2;//differences in the top half
            *pBuffBottom = (image[stride * (2*row) + col] + image[stride * (2*row+1) + col])/2;//averages in the bottom half
            pBuffTop++;
            pBuffBottom++;
        }
    }

    //append difference to image array
    image.insert(image.end(), buffer.begin(), buffer.begin() + 2 * nextWidth * nextHeight);

    //fold the bottom half horizontally(HL, LL)
    uint64_t offset_horiz = nextHeight * nextWidth;
    T* pBuffLeft = buffer.data();
    T* pBuffRight = buffer.data() + offset_horiz;
    for(uint32_t row = 0; row < nextHeight; row++){
        for(uint32_t col = 0; col < nextWidth; col++){
            *pBuffLeft = (buffer[offset_vert + row * (2 * nextWidth) + col] - buffer[offset_vert + row * (2 * nextWidth) + col + 1])/2;//differences in the left half
            *pBuffRight = (buffer[offset_vert + row * (2 * nextWidth) + col] + buffer[offset_vert + row * (2 * nextWidth) + col + 1])/2;//averages in the right half
            pBuffLeft++;
            pBuffRight++;
        }
    }

    //next level
    return hwt_fwd(image, buffer, width/2, height/2, level++);
}

template<typename T>
int32_t hwt_inv(vector<T>& decompImg, vector<T>& buffer, uint32_t width, uint32_t hight, uint32_t depth){
    for(uint32_t row = 0; row < hight; row++){
        for(int col = 0; col < width; col++){
            ;
        }
    }
    return 0;
}

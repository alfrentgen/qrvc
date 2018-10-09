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
int32_t hwt_fwd(vector<T>& image, vector<T>& buffer, uint32_t width, uint32_t height, uint32_t& level){
    if(height<2 || width<2){
        return 0;
    }

    level++;
    uint32_t nextWidth = width/2;
    uint32_t nextHeight = height/2;
    T val1 = 0;
    T val2 = 0;
    buffer.resize(2 * nextWidth * 2 * nextHeight, 0);
    image.reserve(image.size() + buffer.size());//reserve additional room for results

    T* pData = image.data() + image.size() - width*height;

    uint64_t offset_vert = nextHeight * 2 * nextWidth;
    uint32_t stride = width;
    T* pDifferences = buffer.data();
    T* pAverages = buffer.data() + offset_vert;
    //filtrate vertically
    for(uint32_t row = 0; row < nextHeight; row++){
        for(uint32_t col = 0; col < 2*nextWidth; col++){
            val1 = pData[2 * row * stride + col];
            val2 = pData[(2 * row + 1) * stride + col];
            *pDifferences = (val1 - val2)/2;
            *pAverages = (val1 + val2)/2;
            pDifferences++;
            pAverages++;
        }
    }

    //append vertical difference(H coefficients) to image array.
    //Leaving it unsplitted, as we won't use high freqs for hiding bits.
    image.insert(image.end(), buffer.begin(), buffer.begin() + 2 * nextWidth * nextHeight);

    //fold the bottom half horizontally and put results into the top half L->HL, LL
    uint64_t offset_horiz = nextHeight * nextWidth;
    stride = 2 * nextWidth;
    pDifferences = buffer.data();
    pAverages = buffer.data() + offset_horiz;
    pData = buffer.data() + offset_vert;
    for(uint32_t row = 0; row < nextHeight; row++){
        for(uint32_t col = 0; col < nextWidth; col++){
            val1 = pData[row * stride + 2*col];
            val2 = pData[row * stride + 2*col + 1];
            *pDifferences = (val1 - val2)/2;//differences in the left half
            *pAverages = (val1 + val2)/2;//averages in the right half
            pDifferences++;
            pAverages++;
        }
    }

    //append HL, LL to the image
    image.insert(image.end(), buffer.begin(), buffer.begin() + 2 * nextWidth * nextHeight);
    //Having H,HL,LL coefficients appended to image, proceed to next level recursively
    return hwt_fwd(image, buffer, nextWidth, nextHeight, level);
}

template<typename T>
int32_t hwt_inv(vector<T>& transImage, uint32_t originalWidth, uint32_t originalHeight, uint32_t level){

    uint32_t curWidth = originalWidth >> level;
    uint32_t curHeight = originalHeight >> level;
    if(curHeight <= 0 || curWidth <= 0){
        return -1;
    }
    uint32_t imageSize = originalWidth*originalHeight;

    uint32_t currentLevelSize = curWidth * curHeight;//LL plane size
    T* pSrc = transImage.data() + transImage.size() - (currentLevelSize << 2);
    T* pDst = nullptr;

    for(int l = level; l >= 1; l--){
        uint32_t nextLevelSize = (l == 1) ? imageSize : currentLevelSize << 2;
        pDst;

        for(uint32_t row = 0; row < originalHeight; row++){
            for(int col = 0; col < originalWidth; col++){
                ;
            }
        }

        if(l == 1){
            break;
        }

        curWidth <<= 1;
        curHeight <<= 1;
    }
    return 0;
}

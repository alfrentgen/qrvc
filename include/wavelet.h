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
inline void recreateLevel(T* pLevel, T* pTransform, uint32_t levelHeight, uint32_t levelWidth){
    uint32_t trnHeight = levelHeight >> 1;
    uint32_t trnWidth = levelWidth >> 1;
    T* pTransformH = pTransform;
    T* pTransformHL = pTransform + trnHeight * 2 * trnWidth;
    T* pTransformLL = pTransformHL + trnHeight*trnWidth;
    for(uint32_t row = 0; row < trnHeight; row++){
        for(uint32_t col = 0; col < trnWidth; col++){
            T H1 = pTransformH[row*(trnWidth<<1) + (col<<1)];
            T H2 = pTransformH[row*(trnWidth<<1) + (col<<1)+1];
            T HL = pTransformHL[row*trnWidth + col];
            T LL = pTransformLL[row*trnWidth + col];
            T L1 = LL + HL;
            T L2 = LL - HL;

            pLevel[(row<<1)*levelWidth + (col<<1)] = L1 + H1;
            pLevel[(row<<1)*levelWidth + (col<<1) + 1] = L2 + H2;
            pLevel[(row<<1 + 1)*levelWidth + (col<<1)] = L1 - H1;
            pLevel[(row<<1 + 1)*levelWidth + (col<<1) + 1] = L2 - H2;
        }
    }
}

template<typename T>
int32_t hwt_inv(vector<T>& transform, uint32_t origWidth, uint32_t origHeight, uint32_t level){//level which the transform was applied down to

    uint32_t curWidth = origWidth >> level;
    uint32_t curHeight = origHeight >> level;
    if((curHeight|curWidth)==0){
        return -1;
    }

    uint32_t origSize = origWidth * origHeight;
    T* pImage = transform.data();
    T* pTransform = transform.data();

    for(uint32_t curLevel = level-1; curLevel >=0; curLevel--){//curLevel is a level which we restore LL plane for
        uint32_t size = origSize;
        uint32_t imgOffset = 0;
        uint32_t trnOffset = size;
        for(uint32_t l = 1; l <= curLevel; l++){
            imgOffset += size;
            size >>= 2;
            trnOffset += size * 4;
            imgOffset += size * 3;
        }
        pTransform += trnOffset;
        pImage += imgOffset;
        uint32_t levelHeight = origHeight >> curLevel;
        uint32_t levelWidth = origWidth >> curLevel;
        recreateLevel(pImage, pTransform, levelHeight, levelWidth);
    }
    return 0;
}

template<typename T>
void hwt_2x2_fwd(T x0, T x1, T x2, T x3, T* ll, T* lh, T* hl, T* hh){
    *ll = ((x0 + x1) + (x2 + x3)) >> 2;
    *lh = ((x0 + x1) - (x2 + x3)) >> 2;
    *hl = ((x0 - x1) + (x2 - x3)) >> 2;
    *hh = ((x0 - x1) - (x2 - x3)) >> 2;
}

template<typename T>
void hwt_2x2_inv(T ll, T lh, T hl, T hh, T* x0, T* x1, T* x2, T* x3){
    *x0 = ll + hl + lh + hh;
    *x1 = ll + lh - (hl + hh);
    *x2 = ll + hl - (lh + hh);
    *x3 = ll + hh - (hl + lh);
}

template<typename T>
hwt_4x4_fwd(vector<T>& image){
    image.resize(16 + 16 + 4, 0);
    T* pImg = image.data();
    T* phh = pImg + 16;
    T* phl = phh + 4;
    T* plh = phl + 4;
    T* pll = plh + 4;
    T x0, x1, x2, x3;
    //level 1
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            x0 = pImg[i<<3 + j<<1];
            x1 = pImg[i<<3 + j<<1 + 1];
            x2 = pImg[i<<3 + j<<1 + 4];
            x3 = pImg[i<<3 + j<<1 + 4 + 1];
            hwt_2x2_fwd(x0, x1, x2, x3, pll++, plh++, phl++, phh++);
        }
    }
    //level 2
    pImg = pll - 4;
    phh = pll;
    phl = phh + 1;
    plh = phl + 1;
    pll = plh + 1;
    x0 = pImg[0]; x1 = pImg[1]; x2 = pImg[2]; x3 = pImg[3];
    hwt_2x2_fwd(x0, x1, x2, x3, pll, plh, phl, phh);
}

template<typename T>
hwt_4x4_inv(vector<T>& transform){
    T* pTrn = transform.data() + 16 + 16;
    T* pImg = pTrn - 4;
    //level 1 reconstruction
    hwt_2x2_inv(pTrn[3], pTrn[2], pTrn[1], pTrn[0], pImg, pImg+1, pImg+2, pImg+3);

    //level 0 reconstruction
    pTrn -= 16;
    pImg = pTrn - 16;
    T* pBlock;
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 2; j++){
            pBlock = pImg + i<<3 + j<<1;
            hwt_2x2_inv(pTrn[12], pTrn[8], pTrn[4], pTrn[0], pBlock, pBlock+1, pBlock+8, pBlock+8+1);
            pTrn++;
        }
    }
}

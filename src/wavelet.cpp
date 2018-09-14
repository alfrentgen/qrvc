#include "pch.h"

//forward transform
/*Y=(Cf*X*Cft).*Ef
const float a = 1.0/2.0;
const float b = sqrt(2.0/5.0);
float table_c_f[4][4]={
    {1, 1, 1, 1},
    {2, 1, -1, -2},
    {1, -1, -1, 1},
    {1, -2, 2, -1},
};
float table_c_f_transp[4][4]={
    {1, 2, 1, 1},
    {1, 1, -1, -2},
    {1, -1, -1, 2},
    {1, -2, 1, 1},
};
float table_e_f[4][4]={
    {a*a, a*b/2, a*a, a*b/2},
    {a*b/2, b*b/4, a*b/2, b*b/4},
    {a*a, a*b/2, a*a, a*b/2},
    {a*b/2, b*b/4, a*b/2, b*b/4},
};*/
void mul_Cf_M(int16_t in[4][4], int16_t out[4][4]);
void mul_M_Cft(int16_t in[4][4], int16_t out[4][4]);
void dot_M_Ef(int16_t in[4][4], int16_t out[4][4]);
void fwd_4x4_dct(uint8_t in[4][4], uint16_t out[4][4]);

#define MUL_2(X) (X << 1)
template<typename T>
void mul_Cf_M(T in[4][4], T out[4][4]){
    for(int col = 0; col < 4; col++){
        out[0][col] = in[0][col] + in[1][col] + in[2][col] + in[3][col];
        out[1][col] = MUL_2(in[0][col]) + in[1][col] - in[2][col] - MUL_2(in[3][col]);
        out[2][col] = in[0][col] - in[1][col] - in[2][col] + in[3][col];
        out[3][col] = in[0][col] - MUL_2(in[1][col]) + MUL_2(in[2][col]) - in[3][col];
    }
}
template<typename T>
void mul_M_Cft(T in[4][4], T out[4][4]){
    for(int row = 0; row < 4; row++){
        out[row][0] = in[row][0] + in[row][1] + in[row][2] + in[row][3];
        out[row][1] = MUL_2(in[row][0]) + in[row][1] - in[row][2] - MUL_2(in[row][3]);
        out[row][2] = in[row][0] - in[row][1] - in[row][2] + in[row][3];
        out[row][3] = in[row][0] - MUL_2(in[row][1]) + MUL_2(in[row][2]) + in[row][3];
    };
}
#define DIV_2(X) (X >> 1)
#define DIV_4(X) (X >> 2)
template<typename T>
void dot_M_Ef(T in[4][4], T out[4][4]){
    const float b = sqrt(2.0/5.0);
    out[0][0] = DIV_4(in[0][0]);        out[0][1] = DIV_4(in[0][1]) * b;    out[0][2] = DIV_4(in[0][2]);        out[0][3] = DIV_4(in[0][3]) * b;
    out[1][0] = DIV_4(in[1][0]) * b;    out[1][1] = in[1][1] / 10;         out[1][2] = DIV_4(in[1][2]) * b;    out[1][3] = in[1][3] / 10;
    out[2][0] = DIV_4(in[2][0]);        out[2][1] = DIV_4(in[2][1]) * b;    out[2][2] = DIV_4(in[2][2]);        out[2][3] = DIV_4(in[2][3]) * b;
    out[3][0] = DIV_4(in[3][0]) * b;    out[3][1] = in[3][1] / 10;         out[3][2] = DIV_4(in[3][2]) * b;    out[3][3] = in[3][3] / 10;
}

//inverse transform
/*X = Cit*(Y.*Ei)*Ci
*/
float table_c_i[4][4]={
    {1,     1,      1,      1},
    {1,     1/2.0,  -1/2.0, -1},
    {1,     -1,     -1,     1},
    {1/2.0, -1,     1,      -1/2.0}
};

float table_c_i_transp[4][4] = {
    {1,     1,      1,      1/2.0},
    {1,     1/2.0,  -1,     -1},
    {1,     -1/2.0, -1,     1},
    {1,     -1,     1,      -1/2.0},
};

float table_e_i[4][4]={
    {a*a, a*b, a*a, a*b},
    {a*b, b*b, a*b, b*b},
    {a*a, a*b, a*a, a*b},
    {a*b, b*b, a*b, b*b},
};

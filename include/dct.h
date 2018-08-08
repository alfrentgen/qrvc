#include "pch.h"

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

template<int32_t s, typename T1, typename T2, typename Tres>
void matrix_product(T1 m1[s][s], T2 m2[s][s], Tres m_res[s][s]){
    for(int row = 0; row < s; row++){
        for(int col = 0; col < s; col++){
            m_res[row][col] = 0;
            for(int i = 0; i < s; i++){
                m_res[row][col] += m1[row][i]*m2[i][row];
            }
        }
    }
}

template<int32_t s, typename T1, typename T2, typename Tres>
void matrix_dot_product(T1 m1[s][s], T2 m2[s][s], Tres m_res[s][s]){
    for(int row = 0; row < s; row++){
        for(int col = 0; col < s; col++){
            m_res[row][col] = m1[row][col] * m2[row][col];
        }
    }
}

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
template<typename T>
void mul_Cf_M(T in[4][4], T out[4][4]){
    for(int col = 0; col < 4; col++){
        out[0][col] = in[0][col]        + in[1][col]        + in[2][col]        + in[3][col];
        out[1][col] = MUL_2(in[0][col]) + in[1][col]        - in[2][col]        - MUL_2(in[3][col]);
        out[2][col] = in[0][col]        - in[1][col]        - in[2][col]        + in[3][col];
        out[3][col] = in[0][col]        - MUL_2(in[1][col]) + MUL_2(in[2][col]) - in[3][col];
    }
}
template<typename T>
void mul_M_Cft(T in[4][4], T out[4][4]){
    for(int row = 0; row < 4; row++){
        out[row][0] = in[row][0]        + in[row][1]        + in[row][2]        + in[row][3];
        out[row][1] = MUL_2(in[row][0]) + in[row][1]        - in[row][2]        - MUL_2(in[row][3]);
        out[row][2] = in[row][0]        - in[row][1]        - in[row][2]        + in[row][3];
        out[row][3] = in[row][0]        - MUL_2(in[row][1]) + MUL_2(in[row][2]) + in[row][3];
    };
}

template<typename T>
void dot_M_Ef(T in[4][4], T out[4][4]){
    const float b = sqrt(2.0/5.0);
    out[0][0] = DIV_4(in[0][0]);        out[0][1] = DIV_4((T)(in[0][1]*b));    out[0][2] = DIV_4(in[0][2]);        out[0][3] = DIV_4((T)(in[0][3]*b));
    out[1][0] = DIV_4((T)(in[1][0]*b));    out[1][1] = DIV_10(in[1][1]);         out[1][2] = DIV_4((T)(in[1][2]*b));    out[1][3] = DIV_10(in[1][3]);
    out[2][0] = DIV_4(in[2][0]);        out[2][1] = DIV_4((T)(in[2][1]*b));    out[2][2] = DIV_4(in[2][2]);        out[2][3] = DIV_4((T)(in[2][3]*b));
    out[3][0] = DIV_4((T)(in[3][0]*b));    out[3][1] = DIV_10(in[3][1]);         out[3][2] = DIV_4((T)(in[3][2]*b));    out[3][3] = DIV_10(in[3][3]);
}

template<typename T>
void fwd_4x4_dct(uint8_t in[4][4], T out[4][4]){
    T in_conv[4][4] = {0};
    T tmp1[4][4] = {0};
    T tmp2[4][4] = {0};

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            in_conv[i][j] = (T)(in[i][j] - 128);
        }
    }
    mul_Cf_M(in_conv, tmp1);
    mul_M_Cft(tmp1, tmp2);
    dot_M_Ef(tmp2, out);
}

//inverse transform
/*X = Cit*(Y.*Ei)*Ci
const float a = 1.0/2.0;
const float b = sqrt(2.0/5.0);
float table_e_i[4][4]={
    {a*a, a*b, a*a, a*b},
    {a*b, b*b, a*b, b*b},
    {a*a, a*b, a*a, a*b},
    {a*b, b*b, a*b, b*b},
};
float table_c_i_transp[4][4] = {
    {1,     1,      1,      1/2.0},
    {1,     1/2.0,  -1,     -1},
    {1,     -1/2.0, -1,     1},
    {1,     -1,     1,      -1/2.0},
};
float table_c_i[4][4]={
    {1,     1,      1,      1},
    {1,     1/2.0,  -1/2.0, -1},
    {1,     -1,     -1,     1},
    {1/2.0, -1,     1,      -1/2.0}
};
*/
template<typename T>
void dot_M_Ei(T in[4][4], T out[4][4]){
    const float b = sqrt(2.0/5.0);
    out[0][0] = DIV_4(in[0][0]);        out[0][1] = DIV_2((T)(in[0][1]*b));    out[0][2] = DIV_4(in[0][2]);        out[0][3] = DIV_2((T)(in[0][3]*b));
    out[1][0] = DIV_2((T)(in[1][0]*b));    out[1][1] = DIV_10(MUL_4(in[1][1]));out[1][2] = DIV_2((T)(in[1][2]*b));    out[1][3] = DIV_10(MUL_4(in[1][3]));
    out[2][0] = DIV_4(in[2][0]);        out[2][1] = DIV_2((T)(in[2][1]*b));    out[2][2] = DIV_4(in[2][2]);        out[2][3] = DIV_2((T)(in[2][3]*b));
    out[3][0] = DIV_2((T)(in[3][0]*b));    out[3][1] = DIV_10(MUL_4(in[3][1]));out[3][2] = DIV_2((T)(in[3][2]*b));    out[3][3] = DIV_10(MUL_4(in[3][3]));
}
template<typename T>
void mul_Cit_M(T in[4][4], T out[4][4]){
    for(int col = 0; col < 4; col++){
        out[0][col] = in[0][col] + in[1][col]           + in[2][col] + DIV_2(in[3][col]);
        out[1][col] = in[0][col] + DIV_2(in[1][col])    - in[2][col] - in[3][col];
        out[2][col] = in[0][col] - DIV_2(in[1][col])    - in[2][col] + in[3][col];
        out[3][col] = in[0][col] - in[1][col]           + in[2][col] - DIV_2(in[3][col]);
    }
}
template<typename T>
void mul_M_Ci(T in[4][4], T out[4][4]){
    for(int row = 0; row < 4; row++){
        out[row][0] = in[row][0] + in[row][1]        + in[row][2] + DIV_2(in[row][3]);
        out[row][1] = in[row][0] + DIV_2(in[row][1]) - in[row][2] - in[row][3];
        out[row][2] = in[row][0] - DIV_2(in[row][1]) - in[row][2] + in[row][3];
        out[row][3] = in[row][0] - in[row][1]        + in[row][2] - DIV_2(in[row][3]);
    };
}

template<typename T>
void inv_4x4_dct(T in[4][4], uint8_t out[4][4]){
    T tmp1[4][4] = {0};
    T tmp2[4][4] = {0};

    dot_M_Ei(in, tmp1);
    mul_Cit_M(tmp1, tmp2);
    mul_M_Ci(tmp2, tmp1);

    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            out[i][j] = (uint8_t)(tmp1[i][j] + 128);
        }
    }
}

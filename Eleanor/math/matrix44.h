//
//  matrix44.h
//  Eleanor
//
//  Created by cliff on 08/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef matrix44_h
#define matrix44_h

struct matrix44 {
    float m[4][4];
    
    matrix44();
    matrix44(const matrix44 &mm);
    
    static matrix44 identity();
    
    void dump();
    
    float &operator ()(const int x, const int y);
    vector4 operator *(const vector4 &vv);
    matrix44 operator *(matrix44 &mm);
    
    void transpose();
    void inverse();
};

inline matrix44::matrix44() {
    memset(m, 0, sizeof(m));
}

inline matrix44::matrix44(const matrix44 &mm) {
    for (int i=0; i<4; i++)
        for (int j=0; j<4; j++)
            m[i][j] = mm.m[i][j];
}

inline float &matrix44::operator ()(const int x, const int y) {
    return m[x][y];
}

inline vector4 matrix44::operator *(const vector4 &vv) {
    vector4 v;
    v.x = m[0][0]*vv.x + m[0][1]*vv.y + m[0][2]*vv.z + m[0][3]*vv.w;
    v.y = m[1][0]*vv.x + m[1][1]*vv.y + m[1][2]*vv.z + m[1][3]*vv.w;
    v.z = m[2][0]*vv.x + m[2][1]*vv.y + m[2][2]*vv.z + m[2][3]*vv.w;
    v.w = m[3][0]*vv.x + m[3][1]*vv.y + m[3][2]*vv.z + m[3][3]*vv.w;
    return v;
}

inline matrix44 matrix44::operator *(matrix44 &mm) {
    matrix44 r;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            float v = 0.0f;
            for (int k = 0; k < 4; k++) {
                v+= m[i][k] * mm(k,j);
            }
            r(i, j) = v;
        }
    }
    return r;
}

inline matrix44 matrix44::identity() {
    matrix44 m;
    m(0,0) = 1;
    m(1,1) = 1;
    m(2,2) = 1;
    m(3,3) = 1;
    
    return m;
}

inline void matrix44::transpose() {
    for (int i = 0; i < 4; i++)
        for (int j = i+1; j < 4; j++) {
            float t = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = t;
        }
}

inline void matrix44::inverse() {
    float t[3][6];
    int i, j, k;
    float f;
    
    for(i = 0; i < 3; i++)
        for(j = 0; j < 6; j++) {
            if(j < 3)
                t[i][j] = m[i][j];
            else if(j == i + 3)
                t[i][j] = 1;
            else
                t[i][j] = 0;
        }
    
    for(i = 0; i < 3; i++) {
        f = t[i][i];
        for(j = 0; j < 6; j++)
            t[i][j] /= f;
        for(j = 0; j < 3; j++) {
            if(j != i) {
                f = t[j][i];
                for(k = 0; k < 6; k++)
                    t[j][k] = t[j][k] - t[i][k] * f;
            }
        }
    }
    
    for(i = 0; i < 3; i++)
        for(j = 3; j < 6; j++)
            m[i][j-3] = t[i][j];
    
    m[3][0] = -m[3][0];
    m[3][1] = -m[3][1];
    m[3][2] = -m[3][2];
}

void matrix44::dump() {
    printf("%.2f  %.2f  %.2f  %.2f\n", m[0][0], m[0][1], m[0][2], m[0][3]);
    printf("%.2f  %.2f  %.2f  %.2f\n", m[1][0], m[1][1], m[1][2], m[1][3]);
    printf("%.2f  %.2f  %.2f  %.2f\n", m[2][0], m[2][1], m[2][2], m[2][3]);
    printf("%.2f  %.2f  %.2f  %.2f\n\n", m[3][0], m[3][1], m[3][2], m[3][3]);
}

#endif /* matrix44_h */

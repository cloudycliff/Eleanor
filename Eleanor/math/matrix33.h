//
//  matrix33.h
//  Eleanor
//
//  Created by cliff on 22/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef matrix33_h
#define matrix33_h

#include "matrix44.h"

struct matrix33 {
    float m[3][3];
    
    matrix33();
    matrix33(const matrix44 &mm);
    matrix33(const vector3 &v1, const vector3 &v2, const vector3 &v3);
    
    static matrix33 identity();
    
    void dump();
    
    float &operator ()(const int x, const int y);
    vector3 operator *(const vector3 &vv);
    matrix33 operator *(matrix33 &mm);
    
    void transpose();
    void inverse();
};

inline matrix33::matrix33() {
    memset(m, 0, sizeof(m));
}

inline matrix33::matrix33(const matrix44 &mm) {
    for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            m[i][j] = mm.m[i][j];
        }
    }
}

inline matrix33::matrix33(const vector3 &v1, const vector3 &v2, const vector3 &v3) {
    for (int i=0; i<3; i++) {
        m[i][0] = v1[i];
        m[i][1] = v2[i];
        m[i][2] = v3[i];
    }
}

inline float &matrix33::operator ()(const int x, const int y) {
    return m[x][y];
}

inline vector3 matrix33::operator *(const vector3 &vv) {
    vector3 v;
    v.x = m[0][0]*vv.x + m[0][1]*vv.y + m[0][2]*vv.z;
    v.y = m[1][0]*vv.x + m[1][1]*vv.y + m[1][2]*vv.z;
    v.z = m[2][0]*vv.x + m[2][1]*vv.y + m[2][2]*vv.z;
    return v;
}

inline matrix33 matrix33::operator *(matrix33 &mm) {
    matrix33 r;
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            float v = 0.0f;
            for (int k = 0; k < 3; k++) {
                v+= m[i][k] * mm(k,j);
            }
            r(i, j) = v;
        }
    }
    return r;
}

inline matrix33 matrix33::identity() {
    matrix33 m;
    m(0,0) = 1;
    m(1,1) = 1;
    m(2,2) = 1;
    
    return m;
}

inline void matrix33::transpose() {
    for (int i = 0; i < 3; i++)
        for (int j = i+1; j < 3; j++) {
            float t = m[i][j];
            m[i][j] = m[j][i];
            m[j][i] = t;
        }
}

inline void matrix33::inverse() {
    int mat[3][3];
    for (int i=0; i<3; i++)
        for (int j=0; j<3; j++)
            mat[i][j] = m[i][j];
    
    float determinant = 0;
    
    //finding determinant
    for(int i = 0; i < 3; i++)
        determinant = determinant + (mat[0][i] * (mat[1][(i+1)%3] * mat[2][(i+2)%3] - mat[1][(i+2)%3] * mat[2][(i+1)%3]));
    
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++)
            m[i][j] = ((mat[(j+1)%3][(i+1)%3] * mat[(j+2)%3][(i+2)%3]) - (mat[(j+1)%3][(i+2)%3] * mat[(j+2)%3][(i+1)%3]))/ determinant;
        
    }
}

void matrix33::dump() {
    printf("%.2f  %.2f  %.2f\n", m[0][0], m[0][1], m[0][2]);
    printf("%.2f  %.2f  %.2f\n", m[1][0], m[1][1], m[1][2]);
    printf("%.2f  %.2f  %.2f\n", m[2][0], m[2][1], m[2][2]);
}

#endif /* matrix33_h */

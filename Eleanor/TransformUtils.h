//
//  TransformUtils.h
//  Eleanor
//
//  Created by cliff on 15/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef TransformUtils_h
#define TransformUtils_h

#include "math/math.h"

matrix44 viewport(int x, int y, int w, int h) {
    matrix44 m = matrix44::identity();
    
    m(0, 3) = x + w/2.0f;
    m(1, 3) = y + h/2.0f;
    m(2, 3) = 0.5f;
    
    m(0, 0) = w/2.0f;
    m(1, 1) = h/2.0f;
    m(2, 2) = 0.5f;
    
    return m;
}

matrix44 lookat(vector3 eye, vector3 center, vector3 up) {
    vector3 z = (eye - center).normalize();
    vector3 x;
    vector3Cross(x, up, z);
    x.normalize();
    vector3 y;
    vector3Cross(y, z, x);
    y.normalize();
    
    matrix44 res = matrix44::identity();
    for (int i = 0; i < 3; i++) {
        res(0, i) = x[i];
        res(1, i) = y[i];
        res(2, i) = z[i];
        res(i, 3) = -center[i];
    }
    
    return res;
}

matrix44 projection(float coeff) {
    matrix44 projection = matrix44::identity();
    projection(3, 2) = coeff;
    return projection;
}

matrix44 projectionFOV(float fovy, float aspect, float near, float far) {
    matrix44 m;
    float d2r = PI/180.0f;
    float nmf = near - far;
    float tanHalfFovy = (float)tan(d2r * fovy);
    m.m[0][0] = 1.0f / (aspect * tanHalfFovy);
    m.m[1][1] = 1.0f / tanHalfFovy;
    m.m[2][2] = (near+far)/nmf;
    m.m[2][3] = -1.0f;
    m.m[3][2] = 2*far*near/nmf;
    m.m[3][3] = 0.0f;

    return m;
}

matrix44 orthogonal(float l, float r, float b, float t, float zn, float zf) {
    matrix44 m;
    m.m[0][0] = 2.0f / (r - l);
    m.m[1][1] = 2.0f / (t - b);
    m.m[2][2] = 1.0f / (zf - zn);
    m.m[3][0] = (l+r)/(l-r);
    m.m[3][1] = (t+b)/(b-t);
    m.m[3][2] = zn / (zn-zf);
    m.m[3][3] = 1.0f;
    m.m[0][1] = m.m[0][2] = m.m[0][3] = m.m[1][0] = m.m[1][2] = m.m[1][3] = 0.0f;
    m.m[2][0] = m.m[2][1] = m.m[2][3] = 0.0f;
    return m;
}

matrix44 scaleMatrix(float x, float y, float z) {
    matrix44 m;
    m(0, 0) = x;
    m(1, 1) = y;
    m(2, 2) = z;
    m(3, 3) = 1;
    return m;
}

matrix44 translateMatrix(vector3 v) {
    matrix44 m = matrix44::identity();
    m(0, 3) = v.x;
    m(1, 3) = v.y;
    m(2, 3) = v.z;
    return m;
}

matrix44 translateMatrix(float x, float y, float z) {
    matrix44 m = matrix44::identity();
    m(0, 3) = x;
    m(1, 3) = y;
    m(2, 3) = z;
    return m;
}

matrix44 rotateMatrix(float x, float y, float z, float theta) {
    if (x == 0 && y == 0 && z ==0) return matrix44::identity();
    
    float qsin = sin(theta * 0.5f);
    float qcos = cos(theta * 0.5f);
    
    vector3 v(x, y, z);
    v.normalize();
    
    float w = qcos;
    x = v.x * qsin;
    y = v.y * qsin;
    z = v.z * qsin;
    
    matrix44 m;
    
    m(0, 0) = 1 - 2 * y * y - 2 * z * z;
    m(0, 1) = 2 * x * y - 2 * w * z;
    m(0, 2) = 2 * x * z + 2 * w * y;
    m(1, 0) = 2 * x * y + 2 * w * z;
    m(1, 1) = 1 - 2 * x * x - 2 * z * z;
    m(1, 2) = 2 * y * z - 2 * w * x;
    m(2, 0) = 2 * x * z - 2 * w * y;
    m(2, 1) = 2 * y * z + 2 * w * x;
    m(2, 2) = 1 - 2 * x * x - 2 * y * y;
    m(3, 3) = 1.0f;

    return m;
}

#endif /* TransformUtils_h */

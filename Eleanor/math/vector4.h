//
//  vector4.h
//  Eleanor
//
//  Created by cliff on 09/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef vector4_h
#define vector4_h

struct vector4 {
    union {
        struct {float x,y,z,w;};
        struct {float r,g,b,a;};
    };
    
    vector4();
    vector4(const vector4 &vv);
    vector4(const float xx, const float yy, const float zz, const float ww);
    vector4(const vector3 &vv, const float ww);
    
    float length() const;
    vector4 &normalize();
    
    float &operator [](const int index);
    const vector4 &operator =(const vector4 &vv);
    vector4 &operator /(const float d) {
        x /= d;
        y /= d;
        z /= d;
        w /= d;
        return *this;
    }
    
    void dump() {
        printf("%f %f %f %f\n", x, y, z, w);
    }
};

inline vector4::vector4() {}
inline vector4::vector4(const vector4 &vv) : x(vv.x), y(vv.y), z(vv.z), w(vv.w) {}
inline vector4::vector4(const float xx, const float yy, const float zz, const float ww) : x(xx), y(yy), z(zz), w(ww) {}
inline vector4::vector4(const vector3 &vv, const float ww) : x(vv.x), y(vv.y), z(vv.z), w(ww) {}

inline float vector4::length() const {
    return sqrt(x*x+y*y+z*z+w*w);
}

inline vector4 &vector4::normalize() {
    const float len = length();
    const float invLen = 1.0f/len;
    x *= invLen;
    y *= invLen;
    z *= invLen;
    w *= invLen;
    return *this;
}

inline float &vector4::operator [](const int index) {
    if (index == 0) return x;
    else if (index == 1) return y;
    else if (index == 2) return z;
    else return w;
}

inline const vector4 &vector4::operator =(const vector4 &vv) {
    x = vv.x;
    y = vv.y;
    z = vv.z;
    w = vv.w;
    return *this;
}

#endif /* vector4_h */

//
//  vector3.h
//  Eleanor
//
//  Created by cliff on 08/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef vector3_h
#define vector3_h

struct vector3 {
    union {
        struct {float x, y, z;};
        struct {float r, g, b;};
    };
    
    vector3();
    vector3(const vector3 &vv);
    vector3(const float xx, const float yy, const float zz);
    
    float length() const;
    vector3 &normalize();
    
    vector3 operator +(const vector3 &vv) const;
    vector3 operator -(const vector3 &vv) const;
    float &operator [](const int index);
    const float &operator [](const int index) const;
    const vector3 &operator =(const vector3 &vv);
    float operator *(const vector3 vv);
    
    vector3 &operator *(const float d) {
        x *= d;
        y *= d;
        z *= d;
        return *this;
    }
    
    vector3 &operator /(const float d) {
        x /= d;
        y /= d;
        z /= d;
        return *this;
    }
    
    vector3 operator-(void) const {
        return vector3(-x, -y, -z);
    }
    
    void dump() {
        printf("%f %f %f\n", x, y, z);
    }
};

inline vector3::vector3() {}
inline vector3::vector3(const vector3 &vv) : x(vv.x), y(vv.y), z(vv.z) {}
inline vector3::vector3(const float xx, const float yy, const float zz) : x(xx), y(yy), z(zz) {}

inline float vector3::length() const {
    return sqrt(x*x+y*y+z*z);
}

inline vector3 &vector3::normalize() {
    const float len = length();
    const float invLen = 1.0f/len;
    x *= invLen;
    y *= invLen;
    z *= invLen;
    return *this;
}

inline vector3 vector3::operator +(const vector3 &vv) const {
    return vector3(x + vv.x, y + vv.y, z + vv.z);
}

inline vector3 vector3::operator -(const vector3 &vv) const {
    return vector3(x - vv.x, y - vv.y, z - vv.z);
}

inline float &vector3::operator [](const int index) {
    if (index == 0) return x;
    else if (index == 1) return y;
    else return z;
}

inline const float &vector3::operator [](const int index) const {
    if (index == 0) return x;
    else if (index == 1) return y;
    else return z;
}

inline const vector3 &vector3::operator=(const vector3 &vv) {
    x = vv.x;
    y = vv.y;
    z = vv.z;
    return *this;
}

inline float vector3::operator *(const vector3 vv) {
    return x * vv.x + y * vv.y + z * vv.z;
}

inline void vector3Cross(vector3 &out, const vector3 &v1, const vector3 &v2) {
    out.x = v1.y * v2.z - v1.z * v2.y;
    out.y = v1.z * v2.x - v1.x * v2.z;
    out.z = v1.x * v2.y - v1.y * v2.x;
}

inline float vector3Dot(const vector3 &v1, const vector3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

vector3 reflect(const vector3 &in, vector3 &normal) {
    return in - normal * (vector3Dot(in, normal) * 2);
}

#endif /* vector3_h */

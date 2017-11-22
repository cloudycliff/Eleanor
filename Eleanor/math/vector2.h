//
//  vector2.h
//  Eleanor
//
//  Created by cliff on 08/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef vector2_h
#define vector2_h

struct vector2 {
    float x, y;
    
    vector2();
    vector2(const vector2 &vv);
    vector2(const float xx, const float yy);
    
    vector2 operator -(const vector2 &vv) const;
    float &operator [](const int index);
};

inline vector2::vector2() {}
inline vector2::vector2(const vector2 &vv) : x(vv.x), y(vv.y) {}
inline vector2::vector2(const float xx, const float yy) : x(xx), y(yy) {}

inline vector2 vector2::operator -(const vector2 &vv) const {
    return vector2(x - vv.x, y - vv.y);
}

float &vector2::operator [](const int index) {
    if (index == 0) return x;
    else return y;
}

#endif /* vector2_h */

//
//  camera.h
//  Eleanor
//
//  Created by cliff on 24/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include "math/math.h"
#include "TransformUtils.h"

struct Camera {
    vector3 center;
    vector3 up;
    vector3 pos;
    
    Camera() {
        center = vector3(0,0,0);
        up = vector3(0,1,0);
        pos = vector3(1,1,1);
    }
    
    matrix44 lookAt() {
        return lookat(pos, center, up);
    }
    
    void updatePos(float x, float y, float z) {
        pos.x = x;
        pos.y = y;
        pos.z = z;
    }
};

#endif /* camera_h */

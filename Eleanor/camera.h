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
    
    float fovy;
    float aspect;
    float near;
    float far;
    
    Camera(float posX, float posY, float posZ) {
        center = vector3(0,0,0);
        up = vector3(0,1,0);
        pos = vector3(posX,posY,posZ);
    }
    
    void init(float fovy, float aspect, float near, float far) {
        this->fovy = fovy;
        this->aspect = aspect;
        this->near = near;
        this->far = far;
    }
    
    matrix44 lookAt() {
        return lookat(pos, center, up);
    }
    
    matrix44 projection() {
        return projectionFOV(fovy, aspect, near, far);
    }
    
    void updatePos(float dx, float dy, float dz) {
        pos.x += dx;
        pos.y += dy;
        pos.z += dz;
    }
};

#endif /* camera_h */

//
//  scene.h
//  Eleanor
//
//  Created by cliff on 27/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef scene_h
#define scene_h

#include "ModelLoader.h"
#include "camera.h"
#include "math/math.h"

struct ModelNode {
    Model *model;
    vector3 position;
    vector3 rotate;
    float angle;
    
    void updateRotate(float a) {
        angle = a;
    }
};

struct Scene {
    ModelNode *modelNode;
    Camera *camera;
    vector3 *light;
    
    Scene() {}
};

#endif /* scene_h */

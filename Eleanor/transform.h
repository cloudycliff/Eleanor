//
//  transform.h
//  Eleanor
//
//  Created by cliff on 27/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef transform_h
#define transform_h

#include "math/math.h"

struct Transforms {
    matrix44 MVP;
    matrix44 MVP_IT;
    
    matrix44 viewport;
    matrix44 view;
    matrix44 projection;
    matrix44 model;
    
    void update() {
        MVP = projection * view * model;
        MVP_IT = matrix44(MVP);
        MVP_IT.inverse();
        MVP_IT.transpose();
    }
};


#endif /* transform_h */

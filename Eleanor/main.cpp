//
//  main.cpp
//  Eleanor
//
//  Created by cliff on 07/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "softrenderer.h"
#include "shaders.h"
#include "viewer.h"
#include "scene.h"
#include "camera.h"
#include "ModelLoader.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

//std::string inputfile = "obj/floor.obj";
std::string inputfile = "obj/african_head/african_head.obj";
//std::string inputfile = "obj/brickwall.obj";


int main(int argc, const char * argv[]) {
    
    Viewer viewer(SCREEN_WIDTH, SCREEN_HEIGHT);
    viewer.init();
    
    Camera camera(vector3(2,2,10), vector3(0,1,0), -100, 0);
    
    Scene scene;
    scene.camera = &camera;
    
    Model modelObj(inputfile);
    ModelNode modelNode;
    modelNode.model = &modelObj;
    modelNode.angle = 0.0f;
    modelNode.position = vector3(0, 1, 0);
    
    scene.modelNode = &modelNode;
    
    vector3 light = vector3(1,1,1);
    light.normalize();
    scene.light = &light;
    
    viewer.setScene(&scene);
    
    
    TestShader shader;
    viewer.setShader(&shader, 0);
    
    PhongShader shader1;
    viewer.setShader(&shader1, 1);
    
    TangentNormalShader shader2;
    viewer.setShader(&shader2, 2);
    
    
    viewer.start();
    
    return 0;
}

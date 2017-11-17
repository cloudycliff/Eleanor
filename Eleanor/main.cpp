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

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

#include "softrenderer.h"
#include "FPSDisplay.h"
#include "ModelLoader.h"
#include "math/math.h"
#include "TransformUtils.h"
#include "TGAImage.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

bool init_sdl(int width, int height, const char *title);
void close_sdl();
void handleKeyEvent(int k);
void handleEvent();

SDL_Window *sdlWindow = NULL;
SDL_Renderer *sdlRenderer = NULL;
bool quit = false;
bool enableZ = true;

Color red(255, 0, 0);
Color green(0, 255, 0);
Color blue(0, 0, 255);


vector3 light_dir(3, 2, 1);

float cameraX = 0;
float cameraY = 0;
float cameraZ = 10;
float speed = 1;
float rotateAngle = 0.0f;
vector3 center(0,0,0);
vector3 up(0,1,0);

#define LOOP

//std::string inputfile = "obj/floor.obj";
std::string inputfile = "obj/african_head/african_head.obj";

Model modelObj(inputfile);

matrix44 mMVP;
matrix44 mMVP_IT;
matrix44 mViewport;
matrix44 mView;
matrix44 mProjection;
matrix44 mModel;

struct TestShader : public IShader {
    
    vector2 uvs[3];
    vector3 normals[3];
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj.getIndex(nface, nthvert);
        
        vector4 pos = modelObj.getVertex(idx.vertex_index);
        vector4 gl_Position = mViewport * mMVP * pos;
        
        vector3 normal = modelObj.getNormal(idx.normal_index);

        vector4 n = vector4(normal, 0.0f);
        vector4 nn = mMVP_IT * n;
        normals[nthvert] = vector3(nn.x, nn.y, nn.z);
        
        uvs[nthvert] = modelObj.getUV(idx.texcoord_index);

        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, Color &c) {
        vector3 n;
        n.x = normals[0].x*bc.x + normals[1].x*bc.y + normals[2].x*bc.z;
        n.y = normals[0].y*bc.x + normals[1].y*bc.y + normals[2].y*bc.z;
        n.z = normals[0].z*bc.x + normals[1].z*bc.y + normals[2].z*bc.z;
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        float diff = std::max(0.0f, n*light_dir);
        TGAColor cc = modelObj.getDiffuse(uv.x, uv.y);
        c.r = cc.bgra[2]*diff;
        c.g = cc.bgra[1]*diff;
        c.b = cc.bgra[0]*diff;
    }
};

int main(int argc, const char * argv[]) {
    
    if (!init_sdl(SCREEN_WIDTH, SCREEN_HEIGHT, "Eleanor")) {
        close_sdl();
        return -1;
    }
    
    SoftRenderer renderer(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    //vector2 pts[3] = {vector2(10,10), vector2(100,30), vector2(190,160)};
    
    FPSDisplay fpsDisplay;
    fpsDisplay.init(sdlRenderer);
    
    mViewport = viewport(SCREEN_WIDTH/8, SCREEN_HEIGHT/8, SCREEN_WIDTH*3/4, SCREEN_HEIGHT*3/4);
    light_dir.normalize();
    
    TestShader shader;
    
#ifdef LOOP
    while (!quit) {
#endif
        handleEvent();
        
        renderer.enableZTest(enableZ);
        
        vector3 camera(cameraX,cameraY,cameraZ);
        
        matrix44 rotate = rotateMatrix(0.0f, 1.0f, 0.0f, rotateAngle);
        matrix44 translate = translateMatrix(0.0f, 0.0f, 0.0f);
        
        mView = lookat(camera, center, up);
        mProjection = projection(-1.0f/(camera-center).length());
        
        mModel = translate * rotate;
        
        mMVP = mProjection * mView * mModel;
        
        mMVP_IT = mProjection * mView * mModel;
        mMVP_IT.inverse();
        mMVP_IT.transpose();
        
        SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(sdlRenderer);
        
        renderer.clear();
        
        //wireframe(attrib, shapes, renderer);
        //triangle(pts, frameBuffer, red);
        //model(attrib, shapes, renderer, m);
        renderer.model(modelObj, shader);
        
        renderer.draw(sdlRenderer);
        
        fpsDisplay.update(sdlRenderer);
        
        SDL_RenderPresent(sdlRenderer);
        
#ifdef LOOP
    }
#endif
    
    fpsDisplay.release();
    close_sdl();
    return 0;
}


bool init_sdl(int width, int height, const char *title) {
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
        std::cout << "SDL hint render scale quality error " << std::endl;
    }
    
    sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (sdlWindow == NULL) {
        std::cout << "SDL create window failed: " << SDL_GetError() << std::endl;
        return false;
    }
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer == NULL) {
        std::cout << "SDL create renderer failed: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    
    return true;
}

void close_sdl() {
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    sdlRenderer = NULL;
    sdlWindow = NULL;
    SDL_Quit();
}

void handleKeyEvent(int k) {
    if (k == SDL_SCANCODE_ESCAPE) quit = true;
    else if (k == SDL_SCANCODE_W) cameraY += speed;
    else if (k == SDL_SCANCODE_S) cameraY -= speed;
    else if (k == SDL_SCANCODE_A) cameraX += speed;
    else if (k == SDL_SCANCODE_D) cameraX -= speed;
    else if (k == SDL_SCANCODE_Q) rotateAngle += 0.1;
    else if (k == SDL_SCANCODE_E) rotateAngle -= 0.1;
    else if (k == SDL_SCANCODE_Z) enableZ = !enableZ;

}

void handleEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) quit = true;
        else if (e.type == SDL_KEYDOWN) handleKeyEvent(e.key.keysym.scancode);
    }
}

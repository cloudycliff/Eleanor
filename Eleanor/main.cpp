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


vector3 light_dir(1, 1, 1);

float cameraX = 0;
float cameraY = -1;
float cameraZ = 3;
float speed = 1;
float rotateAngle = 10.0f;
vector3 center(0,0,0);
vector3 up(0,1,0);
vector3 camera(cameraX,cameraY,cameraZ);

#define LOOP

//std::string inputfile = "obj/floor.obj";
std::string inputfile = "obj/african_head/african_head.obj";
//std::string inputfile = "obj/brickwall.obj";

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
        
        vector4 pos = vector4(modelObj.getVertex(idx.vertex_index), 1.0f);
        vector4 gl_Position = mViewport * mMVP * pos;
        
        vector3 normal = modelObj.getNormal(idx.normal_index);

        vector4 n = vector4(normal, 0.0f);
        vector4 nn = mMVP_IT * n;
        normals[nthvert] = vector3(nn.x, nn.y, nn.z);
        
        uvs[nthvert] = modelObj.getUV(idx.texcoord_index);
        
        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        vector3 n;
        n.x = normals[0].x*bc.x + normals[1].x*bc.y + normals[2].x*bc.z;
        n.y = normals[0].y*bc.x + normals[1].y*bc.y + normals[2].y*bc.z;
        n.z = normals[0].z*bc.x + normals[1].z*bc.y + normals[2].z*bc.z;
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        float diff = std::max(0.0f, n*light_dir);
        c = modelObj.getDiffuse(uv.x, uv.y)*diff;
    }
};

struct TangentShader : public IShader {
    
    vector2 uvs[3];
    vector3 normals[3];
    
    vector3 tangentLightPoss[3];
    vector3 tangentViewPoss[3];
    vector3 tangentFragPoss[3];
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj.getIndex(nface, nthvert);
        
        vector3 pos = modelObj.getVertex(idx.vertex_index);
        vector3 fragPos = matrix33(mModel) * pos;
        
        vector3 normal = modelObj.getNormal(idx.normal_index);
        
        uvs[nthvert] = modelObj.getUV(idx.texcoord_index);
        
        matrix33 normalMatrix = matrix33(mModel);
        normalMatrix.inverse();
        normalMatrix.transpose();
        
        vector3 tangent = modelObj.getTangent(nface);
        vector3 T = normalMatrix * tangent;
        T.normalize();
        vector3 N = normalMatrix * normal;
        N.normalize();
        
        T = T - N * vector3Dot(T, N);
        
        T.normalize();
        vector3 B;
        vector3Cross(B, N, T);
        
        matrix33 TBN = matrix33(T, B, N);
        TBN.transpose();
        
        tangentLightPoss[nthvert] = TBN * light_dir;
        tangentViewPoss[nthvert] = TBN * camera;
        tangentFragPoss[nthvert] = TBN * fragPos;
        
        vector4 gl_Position = mViewport * mMVP * vector4(pos, 1.0f);
        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        vector3 n;
        n.x = normals[0].x*bc.x + normals[1].x*bc.y + normals[2].x*bc.z;
        n.y = normals[0].y*bc.x + normals[1].y*bc.y + normals[2].y*bc.z;
        n.z = normals[0].z*bc.x + normals[1].z*bc.y + normals[2].z*bc.z;
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        vector3 normal = modelObj.getNormal(uv.x, uv.y);
        normal.normalize();

        TGAColor color = modelObj.getDiffuse(uv.x, uv.y);
        TGAColor ambient = color * 0.2;
        
        vector3 tangentLightPos;
        tangentLightPos.x = tangentLightPoss[0].x*bc.x + tangentLightPoss[1].x*bc.y + tangentLightPoss[2].x*bc.z;
        tangentLightPos.y = tangentLightPoss[0].y*bc.x + tangentLightPoss[1].y*bc.y + tangentLightPoss[2].y*bc.z;
        tangentLightPos.z = tangentLightPoss[0].z*bc.x + tangentLightPoss[1].z*bc.y + tangentLightPoss[2].z*bc.z;
        vector3 tangentViewPos;
        tangentViewPos.x = tangentViewPoss[0].x*bc.x + tangentViewPoss[1].x*bc.y + tangentViewPoss[2].x*bc.z;
        tangentViewPos.y = tangentViewPoss[0].y*bc.x + tangentViewPoss[1].y*bc.y + tangentViewPoss[2].y*bc.z;
        tangentViewPos.z = tangentViewPoss[0].z*bc.x + tangentViewPoss[1].z*bc.y + tangentViewPoss[2].z*bc.z;
        vector3 tangentFragPos;
        tangentFragPos.x = tangentFragPoss[0].x*bc.x + tangentFragPoss[1].x*bc.y + tangentFragPoss[2].x*bc.z;
        tangentFragPos.y = tangentFragPoss[0].y*bc.x + tangentFragPoss[1].y*bc.y + tangentFragPoss[2].y*bc.z;
        tangentFragPos.z = tangentFragPoss[0].z*bc.x + tangentFragPoss[1].z*bc.y + tangentFragPoss[2].z*bc.z;

        vector3 lightDir = tangentLightPos - tangentFragPos;
        lightDir.normalize();

        float diff = std::max(lightDir*normal, 0.0f);

        TGAColor diffuse = color * diff;
        
        vector3 viewDir = tangentViewPos - tangentFragPos;
        //vector3 reflectDir = reflect(-lightDir, normal);
        vector3 halfwayDir = lightDir + viewDir;
        halfwayDir.normalize();
        float spec = std::pow(std::max(normal*halfwayDir, 0.0f), 32.0f);
        
        TGAColor specular = TGAColor(32,32,32) * spec;

        c = ambient + diffuse + specular;
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
    
    mViewport = viewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    light_dir.normalize();
    
    TangentShader shader;
    
#ifdef LOOP
    while (!quit) {
#endif
        handleEvent();
        
        renderer.enableZTest(enableZ);
        
        matrix44 rotate = rotateMatrix(0.0f, 1.0f, 0.0f, rotateAngle);
        matrix44 translate = translateMatrix(0.0f, 0.0f, 0.0f);
        
        mView = lookat(camera, center, up);
        
        mProjection = projectionFOV(90.0f, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 500.0f);

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

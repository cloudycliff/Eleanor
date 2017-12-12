//
//  viewer.h
//  Eleanor
//
//  Created by cliff on 27/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef viewer_h
#define viewer_h

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

#include "FPSDisplay.h"
#include "softrenderer.h"
#include "TransformUtils.h"
#include "scene.h"


class Viewer {
public:
    Viewer(int w, int h);
    void init();
    void start();
    
    void setScene(Scene *s);
    void setShader(IShader *s, int sid);
    
private:
    int width, height;
    bool shouldQuit = false;
    bool enableZ = true;
    
    SDL_Window *sdlWindow = NULL;
    SDL_Renderer *sdlRenderer = NULL;
    FPSDisplay fpsDisplay;
    SoftRenderer *renderer;
    Scene *scene;
    Transforms transforms;
    
    IShader *shader[3];
    int shaderId = 0;
    
    
    float rotateAngle = 0.0f;
    
    
    bool initSDL(const char *title);
    void closeSDL();
    
    void update();
    
    void handleEvent();

};

Viewer::Viewer(int w, int h) {
    width = w;
    height = h;
}

void Viewer::init() {
    if (!initSDL("Eleanor")) {
        closeSDL();
        exit(-1);
    }
    
    fpsDisplay.init(sdlRenderer);
    
    renderer = new SoftRenderer(width, height);
}

void Viewer::start() {
    
    while (!shouldQuit)
        update();
    
    fpsDisplay.release();
    closeSDL();
}

void Viewer::update() {
    
    handleEvent();
    
    renderer->enableZTest(enableZ);
    
    SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(sdlRenderer);
    
    renderer->clear();
    
    scene->modelNode->updateRotate(rotateAngle);
    
    matrix44 rotate = rotateMatrix(0.0f, 1.0f, 0.0f, scene->modelNode->angle);
    matrix44 translate = translateMatrix(scene->modelNode->position);
    transforms.model = translate * rotate;
    
    transforms.view = scene->camera->GetViewMatrix();
    transforms.projection = projectionFOV(scene->camera->Zoom, (float)width/(float)height, 0.1f, 100.f);
    
    transforms.update();
    
    renderer->setTransforms(&transforms);
    
    shader[shaderId]->modelObj = scene->modelNode->model;
    shader[shaderId]->transforms = &transforms;
    shader[shaderId]->light = scene->light;
    shader[shaderId]->camera = scene->camera;
    
    //renderer->line(100, 100, 500, 600, TGAColor(255,0,0));
    
    //vector3 pts[3] = {vector3(10,10,0), vector3(100,320,0), vector3(490,460,0)};
    //renderer->triangle(pts, TGAColor(0,255,0));
    
    //renderer->wireframe(*scene->modelNode->model, TGAColor(0,0,255));
    
    renderer->drawAxes();
    
    renderer->model(*scene->modelNode->model, *shader[shaderId]);
    
    renderer->draw(sdlRenderer);
    
    fpsDisplay.update(sdlRenderer);
    
    SDL_RenderPresent(sdlRenderer);
}

void Viewer::handleEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) shouldQuit = true;
        else if (e.type == SDL_KEYDOWN) {
            int k = e.key.keysym.scancode;
            
            if (k == SDL_SCANCODE_ESCAPE) shouldQuit = true;
            
            else if (k == SDL_SCANCODE_W) scene->camera->ProcessKeyboard(FORWARD, fpsDisplay.getDeltaTime()/10000.0);
            else if (k == SDL_SCANCODE_S) scene->camera->ProcessKeyboard(BACKWARD, fpsDisplay.getDeltaTime()/10000.0);
            else if (k == SDL_SCANCODE_A) scene->camera->ProcessKeyboard(LEFT, fpsDisplay.getDeltaTime()/10000.0);
            else if (k == SDL_SCANCODE_D) scene->camera->ProcessKeyboard(RIGHT, fpsDisplay.getDeltaTime()/10000.0);
            
            else if (k == SDL_SCANCODE_Q) rotateAngle += 0.1;
            else if (k == SDL_SCANCODE_E) rotateAngle -= 0.1;
            
            else if (k == SDL_SCANCODE_Z) enableZ = !enableZ;
            
            else if (k == SDL_SCANCODE_1) shaderId = 0;
            else if (k == SDL_SCANCODE_2) shaderId = 1;
            else if (k == SDL_SCANCODE_3) shaderId = 2;
            
        } else if (e.type == SDL_MOUSEMOTION) {
            scene->camera->ProcessMouseMovement(e.motion.xrel, e.motion.yrel);
        } else if (e.type == SDL_MOUSEWHEEL) {
            scene->camera->ProcessMouseScroll(e.wheel.y);
        }
    }
}

void Viewer::setScene(Scene *s) {
    this->scene = s;
}

void Viewer::setShader(IShader *s, int sid) {
    this->shader[sid] = s;
}

bool Viewer::initSDL(const char *title) {
    
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

void Viewer::closeSDL() {
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    sdlRenderer = NULL;
    sdlWindow = NULL;
    SDL_Quit();
}

#endif /* viewer_h */

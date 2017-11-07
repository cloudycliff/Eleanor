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

#include <SDL2/SDL.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "framebuffer.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

bool init_sdl(int width, int height, const char *title);
void close_sdl();
void handleKeyEvent(int k);
void handleEvent();

SDL_Window *sdlWindow = NULL;
SDL_Renderer *sdlRenderer = NULL;
bool quit = false;

Color red(255, 0, 0);
Color green(0, 255, 0);
Color blue(0, 0, 255);

void line(int x0, int y0, int x1, int y1, FrameBuffer &buffer, Color color) {
    
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    int derror2 = std::abs(dy)*2;
    int error2 = 0;
    int y = y0;
    for (int x=x0; x<=x1; x++) {
        if (steep) {
            buffer.set(y, x, color);
        } else {
            buffer.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1>y0?1:-1);
            error2 -= dx*2;
        }
    }
}

void wireframe(std::string inputfile, FrameBuffer &buffer) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());
    if (!err.empty()) {
        std::cerr << err << std::endl;
    }
    if (!ret) {
        close_sdl();
        return;
    }
    
    for (size_t s = 0; s < shapes.size(); s++) {
        for (size_t f = 0; f < shapes[s].mesh.indices.size()/3; f++) {
            tinyobj::index_t idx0 = shapes[s].mesh.indices[3*f + 0];
            tinyobj::index_t idx1 = shapes[s].mesh.indices[3*f + 1];
            tinyobj::index_t idx2 = shapes[s].mesh.indices[3*f + 2];
            
            float v[3][3];
            int f0 = idx0.vertex_index;
            int f1 = idx1.vertex_index;
            int f2 = idx2.vertex_index;
            for (int k = 0; k < 3; k++) {
                v[0][k] = attrib.vertices[3 * f0 + k];
                v[1][k] = attrib.vertices[3 * f1 + k];
                v[2][k] = attrib.vertices[3 * f2 + k];
            }
            
            for (int k = 0; k < 3; k++) {
                float *v0 = v[k];
                float *v1 = v[(k+1)%3];
                
                int x0 = (v0[0]+1.0)*SCREEN_WIDTH/2.0;
                int y0 = (-v0[1]+1.0)*SCREEN_HEIGHT/2.0;
                int x1 = (v1[0]+1.0)*SCREEN_WIDTH/2.0;
                int y1 = (-v1[1]+1.0)*SCREEN_HEIGHT/2.0;
                
                line(x0, y0, x1, y1, buffer, red);
            }
        }
    }
}

int main(int argc, const char * argv[]) {
    
    if (!init_sdl(SCREEN_WIDTH, SCREEN_HEIGHT, "Eleanor")) {
        close_sdl();
        return -1;
    }
    
    FrameBuffer frameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    std::string inputfile = "obj/african_head/african_head.obj";
    
    while (!quit) {
        
        handleEvent();
        
        SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(sdlRenderer);
        
        wireframe(inputfile, frameBuffer);
        
        frameBuffer.draw(sdlRenderer);

    }
    
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
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
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
}

void handleEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) quit = true;
        else if (e.type == SDL_KEYDOWN) handleKeyEvent(e.key.keysym.scancode);
    }
}

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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "framebuffer.h"

#include "math/math.h"

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

void line(int x0, int y0, int x1, int y1, FrameBuffer &buffer, Color &color) {
    
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
                
                if (x0 > 400)
                    line(x0, y0, x1, y1, buffer, red);
                else
                    line(x0, y0, x1, y1, buffer, blue);
            }
        }
    }
}

vector3 barycentric(vector3 *pts, vector2 p) {
    vector3 u;
    vector3Cross(u, vector3(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-p.x), vector3(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-p.y));
    if (std::abs(u.z) < 1) return vector3(-1, 1, 1);
    return vector3(1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void triangle(vector3 *pts, FrameBuffer &buffer, float *zbuffer, Color &color) {
    vector2 bboxmin(buffer.getWidth()-1, buffer.getHeight()-1);
    vector2 bboxmax(0, 0);
    vector2 clamp(buffer.getWidth()-1, buffer.getHeight()-1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    vector2 p;
    for (p.x=bboxmin.x; p.x <= bboxmax.x; p.x++) {
        for (p.y=bboxmin.y; p.y <= bboxmax.y; p.y++) {
            vector3 bc = barycentric(pts, p);
            if (bc.x<0 || bc.y<0 || bc.z<0) continue;
            
            float z = 0;
            for (int i=0; i<3; i++) z += pts[i][2]*bc[i];
            if (zbuffer[int(p.x+p.y*buffer.getWidth())] < z) {
                zbuffer[int(p.x+p.y*buffer.getWidth())] = z;
                buffer.set(p.x, p.y, color);
            }
        }
    }
}

void model(std::string inputfile, FrameBuffer &buffer, float *zbuffer, matrix44 &m) {
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
    vector3 light(0, 0, -1);
    
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
            
            vector3 pts[3];
            vector3 worldCoords[3];
            for (int k = 0; k < 3; k++) {
                float *v0 = v[k];
                
                vector3 vv = vector3(v0[0], v0[1], v0[2]);
                vector4 vvv = m * vector4(vv, 1.0f);
                
                pts[k] = vector3(vvv.x/vvv.w, vvv.y/vvv.w, vvv.z/vvv.w);
                
                worldCoords[k] = vv;
            }
            
            vector3 n;
            vector3Cross(n, worldCoords[2]-worldCoords[0], worldCoords[1] - worldCoords[0]);
            n.normalize();
            
            float intensity = vector3Dot(n, light);
            
            if (intensity > 0) {
                Color c(intensity*255, intensity*255, intensity*255);
                triangle(pts, buffer, zbuffer, c);
            }
        }
    }
}
int depth = 255;
matrix44 viewport(int x, int y, int w, int h) {
    matrix44 m = matrix44::identity();
    m(0, 3) = x + w/2.0f;
    m(1, 3) = y + h/2.0f;
    m(2, 3) = depth/2.0f;
    
    m(0, 0) = w/2.0f;
    m(1, 1) = h/2.0f;
    m(2, 2) = depth/2.0f;
    
    return m;
}

int main(int argc, const char * argv[]) {
    
    if (!init_sdl(SCREEN_WIDTH, SCREEN_HEIGHT, "Eleanor")) {
        close_sdl();
        return -1;
    }
    
    FrameBuffer frameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    std::string inputfile = "obj/african_head/african_head.obj";
    
    //vector2 pts[3] = {vector2(10,10), vector2(100,30), vector2(190,160)};
    
    float fps = 0.0f;
    int frame = 0;
    Uint32 start = SDL_GetTicks();
    
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("CONSOLA.TTF", 18);
    SDL_Color color = {255, 0, 0};
    SDL_Surface *surface = TTF_RenderText_Solid(font, "FPS: 0.00", color);
    SDL_Rect rect = {0,0,100,40};
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    
    float *zbuffer = new float[SCREEN_WIDTH*SCREEN_HEIGHT];
    for (int i = 0; i < SCREEN_HEIGHT*SCREEN_WIDTH; i++) {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    
    vector3 camera(0,0,3);
    matrix44 projection = matrix44::identity();
    projection(3, 2) = -1.0f/camera.z;
    matrix44 mviewport = viewport(SCREEN_WIDTH/8, SCREEN_HEIGHT/8, SCREEN_WIDTH*3/4, SCREEN_HEIGHT*3/4);
    
    matrix44 m = mviewport * projection;
    
    while (!quit) {
        
        handleEvent();
        
        SDL_SetRenderDrawColor(sdlRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(sdlRenderer);
        
        //wireframe(inputfile, frameBuffer);
        //triangle(pts, frameBuffer, red);
        model(inputfile, frameBuffer, zbuffer, m);
        
        frameBuffer.draw(sdlRenderer);
        
        char *f = new char[10];
        sprintf(f, "FPS: %.3f", fps);
        surface = TTF_RenderText_Solid(font, f, color);
        texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
        
        SDL_RenderCopy(sdlRenderer, texture, NULL, &rect);
        SDL_RenderPresent(sdlRenderer);
        
        frame++;
        fps = frame / (float)(SDL_GetTicks() - start);
        
    }
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
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

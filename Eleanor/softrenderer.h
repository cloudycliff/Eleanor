//
//  framebuffer.h
//  Eleanor
//
//  Created by cliff on 07/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef framebuffer_h
#define framebuffer_h

#include <SDL2/SDL.h>
#include "math/math.h"

struct Color {
    Uint8 r,g,b,a;
    
    Color(Uint8 rr, Uint8 gg, Uint8 bb, Uint8 aa=255) {
        r = rr;
        g = gg;
        b = bb;
        a = aa;
    }
};

class SoftRenderer {
private:
    Color *buffer;
    float *zbuffer;
    int width;
    int height;
    
    vector3 barycentric(vector3 *pts, vector2 p);
    
public:
    SoftRenderer(int w, int h) {
        width = w;
        height = h;
        buffer = (Color*)malloc(width*height*sizeof(Color));
        memset(buffer, 0, width*height*sizeof(Uint32));
        
        zbuffer = new float[width*height];
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = -std::numeric_limits<float>::max();
        }
    }
    
    bool set(int x, int y, Color &c) {
        if (x<0 || x>=width || y<0 || y>=height) return false;
        buffer[y * width + x] = c;
        return true;
    }
    
    void draw(SDL_Renderer *sdlRenderer) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Color color = buffer[y * width + x];
                SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(sdlRenderer, x, y);
            }
        }
    }
    
    int getWidth() {return width;}
    int getHeight() {return height;}
    
    void clear() {
        memset(buffer, 0, width*height*sizeof(Uint32));
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = -std::numeric_limits<float>::max();
        }
    }
    
    void triangle(vector3 *pts, Color &color);
    void line(int x0, int y0, int x1, int y1, Color &color);
};

void SoftRenderer::line(int x0, int y0, int x1, int y1, Color &color) {
    
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
            set(y, x, color);
        } else {
            set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1>y0?1:-1);
            error2 -= dx*2;
        }
    }
}

vector3 SoftRenderer::barycentric(vector3 *pts, vector2 p) {
    vector3 u;
    vector3Cross(u, vector3(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-p.x), vector3(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-p.y));
    if (std::abs(u.z) < 1) return vector3(-1, 1, 1);
    return vector3(1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void SoftRenderer::triangle(vector3 *pts, Color &color) {
    vector2 bboxmin(width-1, height-1);
    vector2 bboxmax(0, 0);
    vector2 clamp(width-1, height-1);
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
            if (zbuffer[int(p.x+p.y*width)] < z) {
                zbuffer[int(p.x+p.y*width)] = z;
                set(p.x, p.y, color);
            }
        }
    }
}

#endif /* framebuffer_h */

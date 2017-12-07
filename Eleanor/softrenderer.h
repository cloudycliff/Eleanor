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
#include "ModelLoader.h"
#include "TGAImage.h"
#include "shaders.h"
#include "TransformUtils.h"

class SoftRenderer {
private:
    unsigned char *buffer = NULL;
    int bytespp = 4;
    float *zbuffer;
    int width;
    int height;
    bool _enableZTest = true;
    float zDefault = -5000.0f;
    matrix44 mViewport;
    
    vector3 barycentric(vector3 *pts, vector2 p);
    
public:
    SoftRenderer(int w, int h) {
        width = w;
        height = h;
        
        mViewport = viewport(0, 0, width, height);
        
        int nbytes = width*height*bytespp*sizeof(unsigned char);
        buffer = new unsigned char[nbytes];
        memset(buffer, 0, nbytes);
        
        zbuffer = new float[width*height];
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = zDefault;
        }
    }
    
    void enableZTest(bool z) {
        _enableZTest = z;
    }
    
    bool set(int x, int y, const TGAColor &c) {
        if (x<0 || x>=width || y<0 || y>=height) return false;
        
        memcpy(buffer+(x+y*width)*bytespp, c.bgra, bytespp);
        return true;
    }
    
    void draw(SDL_Renderer *sdlRenderer) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                TGAColor color(buffer+(x+y*width)*bytespp, bytespp);
                SDL_SetRenderDrawColor(sdlRenderer, color.bgra[2], color.bgra[1], color.bgra[0], color.bgra[3]);
                //flip y axis
                SDL_RenderDrawPoint(sdlRenderer, x, height - y);
            }
        }
    }
    
    int getWidth() {return width;}
    int getHeight() {return height;}
    
    void clear() {
        memset(buffer, 0, width*height*sizeof(Uint32));
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = zDefault;
        }
    }
    
    void triangle(vector3 *pts, const TGAColor &color);
    void triangle(vector4 *pts, IShader &shader);
    void line(int x0, int y0, int x1, int y1, const TGAColor &color);
    
    void model(Model &modelObj, IShader &shader);
    
    void wireframe(Model &modelObj, const TGAColor &color);
};

void SoftRenderer::line(int x0, int y0, int x1, int y1, const TGAColor &color) {
    
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

void SoftRenderer::triangle(vector3 *pts, const TGAColor &color) {
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
            if (zbuffer[int(p.x+p.y*width)] >= z) {
                zbuffer[int(p.x+p.y*width)] = z;
                set(p.x, p.y, color);
            }
        }
    }
}

void SoftRenderer::triangle(vector4 *in_pts, IShader &shader) {
    vector3 pts[3];
    for (int i = 0; i < 3; i++) {
        vector4 v = mViewport * in_pts[i];
        pts[i] = vector3(v.x, v.y, v.z);
    }
    
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
    int x, y;
    for (x = bboxmin.x; x <= bboxmax.x; x++) {
        for (y = bboxmin.y; y <= bboxmax.y; y++) {
            p.x = x;
            p.y = y;
            vector3 bc = barycentric(pts, p);

            if (bc.x<0 || bc.y<0 || bc.z<0) continue;
            
            float z = 0;
            for (int i=0; i<3; i++) {
                z += pts[i].z*bc[i];
            }
            
            bool retain = false;
            if (!_enableZTest || (_enableZTest && zbuffer[int(p.x+p.y*width)] <= z)) retain = true;

            if (retain) {
                zbuffer[int(p.x+p.y*width)] = z;
                TGAColor color;
                bool keep = shader.fragment(bc, color);
                if (keep) set(p.x, p.y, color);
            }
        }
    }
}

void SoftRenderer::model(Model &modelObj, IShader &shader) {
    
    shader.init();
    
    for (int f = 0; f < modelObj.getIndexSize()/3; f++) {
        
        vector4 pts[3];
        for (int k = 0; k < 3; k++) {
            vector4 v = shader.vertex(f, k);
            pts[k] = vector4(v.x/v.w, v.y/v.w, v.z/v.w, 1.0f);
        }
        
        triangle(pts, shader);
    }
}

void SoftRenderer::wireframe(Model &modelObj, const TGAColor &color) {
    for (int f = 0; f < modelObj.getIndexSize()/3; f++) {
        
        vector3 v[3];
        for (int k = 0; k < 3; k++) {
            tinyobj::index_t idx = modelObj.getIndex(f, k);
            v[k] = modelObj.getVertex(idx.vertex_index);
        }
        
        for (int k = 0; k < 3; k++) {
            vector3 v0 = v[k];
            vector3 v1 = v[(k+1)%3];
            
            int x0 = (v0[0]+1.0)*width/2.0;
            int y0 = (v0[1]+1.0)*height/2.0;
            int x1 = (v1[0]+1.0)*width/2.0;
            int y1 = (v1[1]+1.0)*height/2.0;
            
            line(x0, y0, x1, y1, color);
        }
    }
}

#endif /* framebuffer_h */

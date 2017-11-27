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


class SoftRenderer {
private:
    unsigned char *buffer = NULL;
    int bytespp = 4;
    float *zbuffer;
    int width;
    int height;
    bool _enableZTest = true;
    float zDefault = 255.0f;
    
    vector3 barycentric(vector3 *pts, vector2 p);
    vector3 barycentric(vector4 *pts, vector2 p);
    
public:
    SoftRenderer(int w, int h) {
        width = w;
        height = h;
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
    
    bool set(int x, int y, TGAColor &c) {
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
    
    void triangle(vector3 *pts, TGAColor &color);
    void triangle(vector4 *pts, IShader &shader);
    void line(int x0, int y0, int x1, int y1, TGAColor &color);
    
    void model(Model &modelObj, IShader &shader);
};

void SoftRenderer::line(int x0, int y0, int x1, int y1, TGAColor &color) {
    
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

void SoftRenderer::triangle(vector3 *pts, TGAColor &color) {
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

vector3 SoftRenderer::barycentric(vector4 *pts, vector2 p) {

    vector3 u;
    vector3 s1 = vector3(pts[2].x/pts[2].w-pts[0].x/pts[0].w, pts[1].x/pts[1].w-pts[0].x/pts[0].w, pts[0].x/pts[0].w-p.x);
    vector3 s2 = vector3(pts[2].y/pts[2].w-pts[0].y/pts[0].w, pts[1].y/pts[1].w-pts[0].y/pts[0].w, pts[0].y/pts[0].w-p.y);
    
    vector3Cross(u, s1, s2);
    if (std::abs(u.z) > 1e-2) return vector3(1.0f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return vector3(-1, 1, 1);
}

void SoftRenderer::triangle(vector4 *pts, IShader &shader) {
    vector2 bboxmin(width-1, height-1);
    vector2 bboxmax(0, 0);
    vector2 clamp(width-1, height-1);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts[i][j]/pts[i].w));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]/pts[i].w));
        }
    }

    vector2 p;
    int x, y;
    for (x=bboxmin.x; x <= bboxmax.x; x++) {
        for (y=bboxmin.y; y <= bboxmax.y; y++) {
            p.x = x;
            p.y = y;
            vector3 bc = barycentric(pts, p);

            if (bc.x<0 || bc.y<0 || bc.z<0) continue;
            
            vector3 bc_clip = vector3(bc.x/pts[0].w, bc.y/pts[1].w, bc.z/pts[2].w);
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);
            
            float z = 0, w = 0;
            for (int i=0; i<3; i++) {
                z += pts[i].z*bc_clip[i];
                w += pts[i].w*bc_clip[i];
            }
            z = std::max(0, std::min(255, int(z/w+0.5)));
            
            bool retain = false;
            if (!_enableZTest || (_enableZTest && zbuffer[int(p.x+p.y*width)] >= z)) retain = true;

            if (retain) {
                zbuffer[int(p.x+p.y*width)] = z;
                TGAColor color;
                shader.fragment(bc_clip, color);
                set(p.x, p.y, color);
            }
        }
    }
}

void SoftRenderer::model(Model &modelObj, IShader &shader) {
    
    for (int f = 0; f < modelObj.getIndexSize()/3; f++) {
        
        vector4 pts[3];
        for (int k = 0; k < 3; k++) {
            pts[k] = shader.vertex(f, k);
        }
        
        triangle(pts, shader);
    }

}

/*
void wireframe(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes, SoftRenderer &buffer) {
    
    int w = 800;
    int h = 600;
    Color blue(0,0,255);
    
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
                
                int x0 = (v0[0]+1.0)*w/2.0;
                int y0 = (-v0[1]+1.0)*h/2.0;
                int x1 = (v1[0]+1.0)*w/2.0;
                int y1 = (-v1[1]+1.0)*h/2.0;
                
                buffer.line(x0, y0, x1, y1, blue);
            }
        }
    }
}

void model(tinyobj::attrib_t attrib, std::vector<tinyobj::shape_t> shapes, SoftRenderer &renderer, matrix44 &m) {
    
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
                
                pts[k] = vector3(int(vvv.x/vvv.w), int(vvv.y/vvv.w), int(vvv.z/vvv.w));
                
                worldCoords[k] = vv;
            }
            
            vector3 n;
            vector3Cross(n, worldCoords[2]-worldCoords[0], worldCoords[1] - worldCoords[0]);
            n.normalize();
            
            float intensity = vector3Dot(n, light);
            
            if (intensity > 0) {
                Color c(intensity*255, intensity*255, intensity*255);
                renderer.triangle(pts, c);
            }
        }
    }
}

 */

#endif /* framebuffer_h */

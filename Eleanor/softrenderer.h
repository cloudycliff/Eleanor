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

struct Color {
    Uint8 r,g,b,a;
    
    Color() {}
    
    Color(Uint8 rr, Uint8 gg, Uint8 bb, Uint8 aa=255) {
        r = rr;
        g = gg;
        b = bb;
        a = aa;
    }
    
    Color operator *(float f) {
        Color ret;
        ret.r = r * f;
        ret.g = g * f;
        ret.b = b * f;
        ret.a = a * f;
        
        return ret;
    }
};

struct IShader {
    virtual vector4 vertex(int nface, int nthvert) = 0;
    virtual void fragment(vector3 bc, Color &c) = 0;
};

class SoftRenderer {
private:
    Color *buffer;
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
        buffer = (Color*)malloc(width*height*sizeof(Color));
        memset(buffer, 0, width*height*sizeof(Uint32));
        
        zbuffer = new float[width*height];
        for (int i = 0; i < width*height; i++) {
            zbuffer[i] = zDefault;
        }
    }
    
    void enableZTest(bool z) {
        _enableZTest = z;
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
            zbuffer[i] = zDefault;
        }
    }
    
    void triangle(vector3 *pts, Color &color);
    void triangle(vector4 *pts, IShader &shader);
    void line(int x0, int y0, int x1, int y1, Color &color);
    
    void model(Model &modelObj, IShader &shader);
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

vector3 SoftRenderer::barycentric(vector4 *pts, vector2 p) {

    vector3 u;
//    vector3 s1 = vector3(pts[2].x/pts[2].w-pts[0].x/pts[0].w, pts[1].x/pts[1].w-pts[0].x/pts[0].w, pts[0].x/pts[0].w-p.x);
//    vector3 s2 = vector3(pts[2].y/pts[2].w-pts[0].y/pts[0].w, pts[1].y/pts[1].w-pts[0].y/pts[0].w, pts[0].y/pts[0].w-p.y);
    vector3 s1 = vector3(pts[2].x-pts[0].x, pts[1].x-pts[0].x, pts[0].x-p.x);
    vector3 s2 = vector3(pts[2].y-pts[0].y, pts[1].y-pts[0].y, pts[0].y-p.y);
    
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
            bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
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
            
            float z = 0, w = 0;
            for (int i=0; i<3; i++) {
                z += pts[i].z*bc[i];
                w += pts[i].w*bc[i];
            }
            z = std::max(0, std::min(255, int(z/w+0.5)));
            
            bool retain = false;
            if (!_enableZTest || (_enableZTest && zbuffer[int(p.x+p.y*width)] >= z)) retain = true;

            if (retain) {
                zbuffer[int(p.x+p.y*width)] = z;
                Color color;
                shader.fragment(bc, color);
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

matrix44 lookat(vector3 eye, vector3 center, vector3 up) {
    vector3 z = (eye - center).normalize();
    vector3 x;
    vector3Cross(x, up, z);
    x.normalize();
    vector3 y;
    vector3Cross(y, z, x);
    y.normalize();
    
    matrix44 res = matrix44::identity();
    for (int i = 0; i < 3; i++) {
        res(0, i) = x[i];
        res(1, i) = y[i];
        res(2, i) = z[i];
        res(i, 3) = -center[i];
    }
    return res;
}

matrix44 projection(float coeff) {
    matrix44 projection = matrix44::identity();
    projection(3, 2) = coeff;
    return projection;
}

#endif /* framebuffer_h */

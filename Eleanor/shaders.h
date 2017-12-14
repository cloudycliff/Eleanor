//
//  shaders.h
//  Eleanor
//
//  Created by cliff on 27/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef shaders_h
#define shaders_h

#include <cmath>

#include "math/math.h"
#include "ModelLoader.h"
#include "transform.h"
#include "camera.h"

struct IShader {
    
    Model *modelObj;
    Transforms *transforms;
    vector3 *light;
    Camera *camera;
    
    virtual void init() {};
    virtual vector4 vertex(int nface, int nthvert) = 0;
    virtual void fragment(vector3 bc, TGAColor &c) = 0;
};

struct TestShader : public IShader {
    
    vector2 uvs[3];
    vector3 normals[3];
    
    vector3 l;
    
    virtual void init() {
        l = matrix33(transforms->MVP) * (*light);
    }
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj->getIndex(nface, nthvert);
        
        vector4 pos = vector4(modelObj->getVertex(idx.vertex_index), 1.0f);
        vector4 gl_Position = transforms->MVP * pos;
        
        vector3 normal = modelObj->getNormal(idx.normal_index);
        
        vector4 n = vector4(normal, 0.0f);
        vector4 nn = transforms->MVP_IT * n;
        normals[nthvert] = vector3(nn.x, nn.y, nn.z);
        
        uvs[nthvert] = modelObj->getUV(idx.texcoord_index);

        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        vector3 n;
        n.x = normals[0].x*bc.x + normals[1].x*bc.y + normals[2].x*bc.z;
        n.y = normals[0].y*bc.x + normals[1].y*bc.y + normals[2].y*bc.z;
        n.z = normals[0].z*bc.x + normals[1].z*bc.y + normals[2].z*bc.z;
        n.normalize();
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        float diff = std::max(0.0f, n * l);
        
        c = modelObj->getDiffuse(uv.x, uv.y)*diff;
    }
};

struct PhongShader : public IShader {
    
    vector2 uvs[3];
    vector3 normals[3];
    
    vector3 l;
    
    virtual void init() {
        l = matrix33(transforms->MVP) * (*light);
    }
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj->getIndex(nface, nthvert);
        
        vector4 pos = vector4(modelObj->getVertex(idx.vertex_index), 1.0f);
        vector4 gl_Position = transforms->MVP * pos;
        
        vector3 normal = modelObj->getNormal(idx.normal_index);
        
        vector4 n = vector4(normal, 0.0f);
        vector4 nn = transforms->MVP_IT * n;
        normals[nthvert] = vector3(nn.x, nn.y, nn.z);
        
        uvs[nthvert] = modelObj->getUV(idx.texcoord_index);
        
        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        vector3 n;
        n.x = normals[0].x*bc.x + normals[1].x*bc.y + normals[2].x*bc.z;
        n.y = normals[0].y*bc.x + normals[1].y*bc.y + normals[2].y*bc.z;
        n.z = normals[0].z*bc.x + normals[1].z*bc.y + normals[2].z*bc.z;
        n.normalize();
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        c = modelObj->getDiffuse(uv.x, uv.y);
        
        float diff = std::max(0.f, n*l);
        
        vector3 r = (n*(n*l*2.f) - l).normalize();
        float spec = std::pow(std::max(r.z, 0.0f), modelObj->getSpecular(uv.x, uv.y));
        
        for (int i=0; i<3; i++) c.bgra[i] = std::min<float>(5 + c.bgra[i]*(diff + .6*spec), 255);
    }
};

struct TangentShader : public IShader {
    
    vector2 uvs[3];
    
    vector3 tangentLightPoss[3];
    vector3 tangentViewPoss[3];
    vector3 tangentFragPoss[3];
    
    vector3 l;
    
    virtual void init() {
        l = vector3(10,10,10);
    }
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj->getIndex(nface, nthvert);
        
        vector3 pos = modelObj->getVertex(idx.vertex_index);
        vector4 fragPos = transforms->model * vector4(pos, 1.0f);
        
        vector3 normal = modelObj->getNormal(idx.normal_index);
        
        uvs[nthvert] = modelObj->getUV(idx.texcoord_index);
        
        matrix33 normalMatrix = matrix33(transforms->model);
        normalMatrix.inverse();
        normalMatrix.transpose();
        
        vector3 tangent = modelObj->getTangent(nface);
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
        
        tangentLightPoss[nthvert] = TBN * l;//(*light);
        tangentViewPoss[nthvert] = TBN * camera->Position;
        tangentFragPoss[nthvert] = TBN * vector3(fragPos.x,fragPos.y,fragPos.z);
        
        vector4 gl_Position = transforms->MVP * vector4(pos, 1.0f);
        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        vector3 normal = modelObj->getNormal(uv.x, uv.y);
        normal.normalize();
        
        TGAColor color = modelObj->getDiffuse(uv.x, uv.y);
        TGAColor ambient = color * 0.1;
        
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
        
        
        vector3 lightDir = tangentLightPos-tangentFragPos;

        float diff = std::max(0.0f, lightDir*normal);
        
        TGAColor diffuse = color * diff;
        
        vector3 viewDir = tangentViewPos-tangentFragPos;

        //vector3 reflectDir = reflect(-lightDir, normal);
        vector3 halfwayDir = lightDir + viewDir;
        halfwayDir.normalize();
        float spec = std::pow(std::max(normal*halfwayDir, 0.0f), 32.0f);
        
        TGAColor specular = TGAColor(32,32,32) * spec;
        
        c = ambient + diffuse + specular;
    }
};

struct TangentNormalShader : IShader {
    vector2 uvs[3];
    vector3 normals[3];
    
    vector3 ndc_tri[3];
    
    vector3 l;
    
    virtual void init() {
        l = matrix33(transforms->MVP) * (*light);
    }
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj->getIndex(nface, nthvert);
        
        vector4 pos = vector4(modelObj->getVertex(idx.vertex_index), 1.0f);
        vector4 gl_Position = transforms->MVP * pos;
        
        vector3 normal = modelObj->getNormal(idx.normal_index);
        
        vector4 n = vector4(normal, 0.0f);
        vector4 nn = transforms->MVP_IT * n;
        normals[nthvert] = vector3(nn.x, nn.y, nn.z);
        
        uvs[nthvert] = modelObj->getUV(idx.texcoord_index);
        
        ndc_tri[nthvert] = vector3(gl_Position.x/gl_Position.w, gl_Position.y/gl_Position.w, gl_Position.z/gl_Position.w);
        
        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        vector3 n;
        n.x = normals[0].x*bc.x + normals[1].x*bc.y + normals[2].x*bc.z;
        n.y = normals[0].y*bc.x + normals[1].y*bc.y + normals[2].y*bc.z;
        n.z = normals[0].z*bc.x + normals[1].z*bc.y + normals[2].z*bc.z;
        n.normalize();
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        vector3 a = ndc_tri[1] - ndc_tri[0];
        vector3 b = ndc_tri[2] - ndc_tri[0];
        
        matrix33 A = matrix33(a, b, n);
        A.transpose();
        A.inverse();
        
        vector3 i = A * vector3(uvs[1].x-uvs[0].x, uvs[2].x-uvs[0].x, 0);
        vector3 j = A * vector3(uvs[1].y-uvs[0].y, uvs[2].y-uvs[0].y, 0);
        matrix33 B = matrix33(i, j, n);
        
        vector3 normal = modelObj->getNormal(uv.x, uv.y);
        normal.normalize();
        
        vector3 N = B * normal;
        N.normalize();
        
        float diff = std::max(0.0f, N * l);
        
        c = modelObj->getDiffuse(uv.x, uv.y)*diff;
    }
};

struct TangentAShader : public IShader {
    
    vector2 uvs[3];
    
    matrix33 TBN;
    
    virtual vector4 vertex(int nface, int nthvert) {
        tinyobj::index_t idx = modelObj->getIndex(nface, nthvert);
        
        vector3 pos = modelObj->getVertex(idx.vertex_index);
        
        vector3 normal = modelObj->getNormal(idx.normal_index);
        
        uvs[nthvert] = modelObj->getUV(idx.texcoord_index);
        
        matrix33 normalMatrix = matrix33(transforms->model);
        normalMatrix.inverse();
        normalMatrix.transpose();
        
        vector3 tangent = modelObj->getTangent(nface);
        vector3 T = normalMatrix * tangent;
        T.normalize();
        vector3 N = normalMatrix * normal;
        N.normalize();
        
        T = T - N * vector3Dot(T, N);
        T.normalize();
        
        vector3 B;
        vector3Cross(B, N, T);
        
        TBN = matrix33(T, B, N);
        
        vector4 gl_Position = transforms->MVP * vector4(pos, 1.0f);
        return gl_Position;
    }
    
    virtual void fragment(vector3 bc, TGAColor &c) {
        
        vector2 uv;
        uv.x = uvs[0].x*bc.x + uvs[1].x*bc.y + uvs[2].x*bc.z;
        uv.y = uvs[0].y*bc.x + uvs[1].y*bc.y + uvs[2].y*bc.z;
        
        vector3 normal = modelObj->getNormal(uv.x, uv.y);
        normal.normalize();
        
        TGAColor color = modelObj->getDiffuse(uv.x, uv.y);
        TGAColor ambient = color * 0.2;
        
        normal = TBN * normal;
        normal.normalize();
        
        float diff = std::max(0.0f, (*light)*normal);
        
        TGAColor diffuse = color * diff;
        
        c = ambient + diffuse;
    }
};

#endif /* shaders_h */

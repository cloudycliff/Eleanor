//
//  ModelLoader.h
//  Eleanor
//
//  Created by cliff on 14/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef ModelLoader_h
#define ModelLoader_h

#include <vector>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "TGAImage.h"

class Model {
private:
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    vector3 *tangents;
    
    TGAImage diffuseMap;
    TGAImage normalMap;
    
    void loadTexture(std::string filename, const char *suffix, TGAImage &img);
    void calcTangents();
public:
    Model(std::string inputfile) {
        std::string err;
        tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());
        if (!err.empty()) {
            std::cerr << err << std::endl;
        }
        
        loadTexture(inputfile, "_diffuse.tga", diffuseMap);
        loadTexture(inputfile, "_nm_tangent.tga", normalMap);
        
        tangents = new vector3[getIndexSize()/3];
        calcTangents();
    }
    
    int getIndexSize() {
        return (int) shapes[0].mesh.indices.size();
    }
    
    tinyobj::index_t getIndex(int nface, int nthvert) {
        return shapes[0].mesh.indices[3*nface + nthvert];
    }
    
    vector3 getVertex(int vid) {
        vector3 pos;
        pos.x = attrib.vertices[3 * vid];
        pos.y = attrib.vertices[3 * vid + 1];
        pos.z = attrib.vertices[3 * vid + 2];
        return pos;
    }
    
    vector3 getNormal(int vid) {
        vector3 normal;
        normal.x = attrib.normals[3 * vid];
        normal.y = attrib.normals[3 * vid + 1];
        normal.z = attrib.normals[3 * vid + 2];
        return normal;
    }
    
    vector2 getUV(int vid) {
        vector2 uv;
        uv.x = attrib.texcoords[2 * vid];
        uv.y = attrib.texcoords[2 * vid + 1];
        return uv;
    }
    
    TGAColor getDiffuse(float u, float v) {
        return diffuseMap.get(u * diffuseMap.width, v * diffuseMap.height);
    }
    
    vector3 getNormal(float u, float v) {
        TGAColor c = normalMap.get(u * normalMap.width, v * normalMap.height);
        vector3 res;
        res.x = (float)c.bgra[2]/255.0f*2.0f-1.0f;
        res.y = (float)c.bgra[1]/255.0f*2.0f-1.0f;
        res.z = (float)c.bgra[0]/255.0f*2.0f-1.0f;
        return res;
    }
    
    vector3 getTangent(int f) {
        return tangents[f];
    }
};

void Model::loadTexture(std::string filename, const char *suffix, TGAImage &img) {
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        bool ret = img.read_tga_file(texfile.c_str());
        std::cout << "load texture file " << texfile << " " << ret << std::endl;
        img.flip_vertically();
    }
}

void Model::calcTangents() {
    for (int f = 0; f < getIndexSize()/3; f++) {
        
        tinyobj::index_t idx1 = getIndex(f, 0);
        vector3 pos1 = getVertex(idx1.vertex_index);
        vector2 uv1 = getUV(idx1.texcoord_index);
        
        tinyobj::index_t idx2 = getIndex(f, 1);
        vector3 pos2 = getVertex(idx2.vertex_index);
        vector2 uv2 = getUV(idx2.texcoord_index);
        
        tinyobj::index_t idx3 = getIndex(f, 2);
        vector3 pos3 = getVertex(idx3.vertex_index);
        vector2 uv3 = getUV(idx3.texcoord_index);
        
        vector3 edge1 = pos2-pos1;
        vector3 edge2 = pos3-pos1;
        vector2 deltaUV1 = uv2-uv1;
        vector2 deltaUV2 = uv3-uv1;
        
        float ff = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        
        vector3 tangent, bitangent;
        tangent.x = ff * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = ff * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = ff * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent.normalize();
        
        bitangent.x = ff * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = ff * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = ff * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent.normalize();
        
        tangents[f] = tangent;
    }
}


#endif /* ModelLoader_h */

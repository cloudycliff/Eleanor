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
    
    TGAImage diffusemap;
    
    void loadTexture(std::string filename, const char *suffix, TGAImage &img);
public:
    Model(std::string inputfile) {
        std::string err;
        tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());
        if (!err.empty()) {
            std::cerr << err << std::endl;
        }
        
        loadTexture(inputfile, "_diffuse.tga", diffusemap);
    }
    
    int getIndexSize() {
        return (int) shapes[0].mesh.indices.size();
    }
    
    tinyobj::index_t getIndex(int nface, int nthvert) {
        return shapes[0].mesh.indices[3*nface + nthvert];
    }
    
    vector4 getVertex(int vid) {
        vector4 pos;
        pos.x = attrib.vertices[3 * vid];
        pos.y = attrib.vertices[3 * vid + 1];
        pos.z = attrib.vertices[3 * vid + 2];
        pos.w = 1.0f;
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
        return diffusemap.get(u * diffusemap.width, v * diffusemap.height);
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

#endif /* ModelLoader_h */

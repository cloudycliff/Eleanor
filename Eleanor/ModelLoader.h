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

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


class Model {
private:
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    
public:
    Model(std::string inputfile) {
        std::string err;
        tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());
        if (!err.empty()) {
            std::cerr << err << std::endl;
        }
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
};

#endif /* ModelLoader_h */

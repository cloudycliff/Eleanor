//
//  TGAImage.h
//  Eleanor
//
//  Created by cliff on 16/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef TGAImage_h
#define TGAImage_h

#include <fstream>

#pragma pack(push,1)
struct TGA_Header {
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
    unsigned char bgra[4];
    unsigned char bytespp;
    
    TGAColor() : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
    }
    TGAColor(const unsigned char *p, unsigned char bpp) : bgra(), bytespp(bpp) {
        for (int i=0; i<(int)bpp; i++) {
            bgra[i] = p[i];
        }
        for (int i=bpp; i<4; i++) {
            bgra[i] = 0;
        }
    }
};

struct TGAImage {
    unsigned char *data = NULL;
    int width;
    int height;
    int bytespp;
    
    bool read_tga_file(const char *filename);
    bool load_rle_data(std::ifstream &in);
    bool flip_horizontally();
    bool flip_vertically();
    
    TGAColor get(int x, int y);
    bool set(int x, int y, TGAColor &c);
};

bool TGAImage::read_tga_file(const char *filename) {
    std::ifstream in;
    in.open(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "cannot open file " << filename << std::endl;
        in.close();
        return false;
    }
    
    TGA_Header header;
    in.read((char *)&header, sizeof(header));
    if (!in.good()) {
        std::cerr << "error reading file header " << filename << std::endl;
        in.close();
        return false;
    }
    
    width = header.width;
    height = header.height;
    bytespp = header.bitsperpixel>>3;
    unsigned long nbytes = width * height * bytespp;
    data = new unsigned char[nbytes];
    if (3==header.datatypecode || 2==header.datatypecode) {
        in.read((char *)data, nbytes);
        if (!in.good()) {
            std::cerr << "error reading file data " << filename << std::endl;
            in.close();
            return false;
        }
    } else if (10==header.datatypecode || 11==header.datatypecode) {
        if (!load_rle_data(in)) {
            std::cerr << "error reading file data " << filename << std::endl;
            in.close();
            return false;
        }
    } else {
        std::cerr << "unknown file format " << filename << std::endl;
        in.close();
        return false;
    }
    
    if (!(header.imagedescriptor & 0x20)) flip_vertically();
    if (header.imagedescriptor & 0x10) flip_horizontally();
    
    in.close();
    return true;
}

bool TGAImage::load_rle_data(std::ifstream &in) {
    unsigned long pixelcount = width*height;
    unsigned long currentpixel = 0;
    unsigned long currentbyte  = 0;
    TGAColor colorbuffer;
    do {
        unsigned char chunkheader = 0;
        chunkheader = in.get();
        if (!in.good()) {
            std::cerr << "an error occured while reading the data\n";
            return false;
        }
        if (chunkheader<128) {
            chunkheader++;
            for (int i=0; i<chunkheader; i++) {
                in.read((char *)colorbuffer.bgra, bytespp);
                if (!in.good()) {
                    std::cerr << "an error occured while reading the header\n";
                    return false;
                }
                for (int t=0; t<bytespp; t++)
                    data[currentbyte++] = colorbuffer.bgra[t];
                currentpixel++;
                if (currentpixel>pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        } else {
            chunkheader -= 127;
            in.read((char *)colorbuffer.bgra, bytespp);
            if (!in.good()) {
                std::cerr << "an error occured while reading the header\n";
                return false;
            }
            for (int i=0; i<chunkheader; i++) {
                for (int t=0; t<bytespp; t++)
                    data[currentbyte++] = colorbuffer.bgra[t];
                currentpixel++;
                if (currentpixel>pixelcount) {
                    std::cerr << "Too many pixels read\n";
                    return false;
                }
            }
        }
    } while (currentpixel < pixelcount);
    return true;
}

bool TGAImage::flip_horizontally() {
    if (!data) return false;
    int half = width>>1;
    for (int i=0; i<half; i++) {
        for (int j=0; j<height; j++) {
            TGAColor c1 = get(i, j);
            TGAColor c2 = get(width-1-i, j);
            set(i, j, c2);
            set(width-1-i, j, c1);
        }
    }
    return true;
}

TGAColor TGAImage::get(int x, int y) {
    if (!data || x<0 || y<0 || x>=width || y>=height) {
        return TGAColor();
    }
    return TGAColor(data+(x+y*width)*bytespp, bytespp);
}

bool TGAImage::set(int x, int y, TGAColor &c) {
    if (!data || x<0 || y<0 || x>=width || y>=height) {
        return false;
    }
    memcpy(data+(x+y*width)*bytespp, c.bgra, bytespp);
    return true;
}

bool TGAImage::flip_vertically() {
    if (!data) return false;
    unsigned long bytes_per_line = width*bytespp;
    unsigned char *line = new unsigned char[bytes_per_line];
    int half = height>>1;
    for (int j=0; j<half; j++) {
        unsigned long l1 = j*bytes_per_line;
        unsigned long l2 = (height-1-j)*bytes_per_line;
        memmove((void *)line,      (void *)(data+l1), bytes_per_line);
        memmove((void *)(data+l1), (void *)(data+l2), bytes_per_line);
        memmove((void *)(data+l2), (void *)line,      bytes_per_line);
    }
    delete [] line;
    return true;
}

#endif /* TGAImage_h */

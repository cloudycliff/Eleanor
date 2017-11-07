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

struct Color {
    Uint8 r,g,b,a;
    
    Color(Uint8 rr, Uint8 gg, Uint8 bb, Uint8 aa=255) {
        r = rr;
        g = gg;
        b = bb;
        a = aa;
    }
};

class FrameBuffer {
private:
    Color *buffer;
    int width;
    int height;
    
public:
    FrameBuffer(int w, int h) {
        width = w;
        height = h;
        buffer = (Color*)malloc(width*height*sizeof(Color));
        memset(buffer, 0, width*height*sizeof(Uint32));
    }
    
    bool set(int x, int y, Color c) {
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
        SDL_RenderPresent(sdlRenderer);
    }
};

#endif /* framebuffer_h */

//
//  FPSDisplay.h
//  Eleanor
//
//  Created by cliff on 14/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef FPSDisplay_h
#define FPSDisplay_h

#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

class FPSDisplay {
private:
    TTF_Font *font;
    SDL_Surface *surface;
    SDL_Texture *texture;
    
    SDL_Rect rect;
    SDL_Color color;
    
    float fps = 0.0f;
    int frame = 0;
    Uint32 start;
    
public:
    
    FPSDisplay() {
        color = {255, 0, 0};
        rect = {0,0,100,40};
    }
    
    void init(SDL_Renderer *sdlRenderer) {
        TTF_Init();
        font = TTF_OpenFont("CONSOLA.TTF", 18);
        surface = TTF_RenderText_Solid(font, "FPS: 0.00", color);
        texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
        
        start = SDL_GetTicks();
    }
    
    void update(SDL_Renderer *sdlRenderer) {
        frame++;
        fps = frame * 1000 / (float)(SDL_GetTicks() - start);
        
        char *f = new char[10];
        sprintf(f, "FPS: %.3f", fps);
        surface = TTF_RenderText_Solid(font, f, color);
        texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
        
        SDL_RenderCopy(sdlRenderer, texture, NULL, &rect);
    }
    
    void release() {
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        SDL_DestroyTexture(texture);
    }
};

#endif /* FPSDisplay_h */

// This file contains most of the SDL functions
//  required to display the User Interface (UI)
//  including graphics, buttons, menus, etc.

#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif

// NOTE: Some of the constants occur at the
//  end of this file
static const char* WINDOW_TITLE = "Evolution Simulator";
static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;


SDL_Texture* load_texture(SDL_Renderer* pRenderer,
        const char* filePath){
    SDL_Texture* ans = IMG_LoadTexture(pRenderer, filePath);
    assert(ans != NULL);
    return ans;
}


// TODO: Eventually, I will want a draw_frame function
//  which includes this function for every object, except
//  the SDL_RenderPresent portion is ran only once
void draw_texture(SDL_Renderer* pRenderer, SDL_Texture* pTexture,
        int xPos, int yPos, int height, int width){
    // xPos = 0, yPos = 0 refers to the top left corner of the window
    // Not sure what pSrc refers to.
    //  pSrc may represent the original object, but I'm not sure
    //  If pSrc == NULL, then a new texture is rendered
    //  Otherwise, the texture may be replaced (???)
    // pDst represents the new object
    //  If dst == NULL, then the texture fills the entire window
    SDL_Rect* pDst = new SDL_Rect;
    pDst->x = xPos; pDst->y = yPos;
    pDst->h = height; pDst->w = width;
    //SDL_RenderCopy(pRenderer, pTexture, pSrc, pDst);
    SDL_RenderCopy(pRenderer, pTexture, NULL, pDst);
    //  This renders the opject according to pDst
    //  I could replace NULL with pSrc, but I don't know
    //      what pSrc refers to
}



SDL_Window* init_SDL_window(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* pWindow = SDL_CreateWindow(
        WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
        WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI
    );
    assert(pWindow != NULL);
    return pWindow;
}

SDL_Renderer* init_SDL_renderer(
        SDL_Window* pWindow){
    SDL_Renderer* pRenderer = SDL_CreateRenderer(
        pWindow, -1, SDL_RENDERER_ACCELERATED
    );
    assert(pRenderer != NULL);
    return pRenderer;
}

void wait_for_user_to_exit_SDL(){
    SDL_Event windowEvent;
    while(true){
        if(SDL_PollEvent(&windowEvent)){
            if(windowEvent.type == SDL_QUIT){
                return;
            }
        }
    }
}

void exit_SDL(SDL_Window* window){
    SDL_DestroyWindow(window);
    SDL_Quit();
}


SDL_Window* pWindow = init_SDL_window();
SDL_Renderer* pRenderer = init_SDL_renderer(pWindow);

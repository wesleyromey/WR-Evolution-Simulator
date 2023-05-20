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
static const int WINDOW_WIDTH  = 10*UB_X < 1000 ? 10*UB_X : 1000;
static const int WINDOW_HEIGHT = 10*UB_Y <  1000 ? 10*UB_Y : 1000;

bool mouseButtonDownPrevFrame = false;
bool simIsRunning = true; // If false, exit the program



SDL_Window* init_SDL_window();
SDL_Window* P_WINDOW = init_SDL_window();
SDL_Renderer* init_SDL_renderer();
SDL_Renderer* P_RENDERER = init_SDL_renderer();
SDL_Texture* load_texture(const char* filePath);
SDL_Texture* P_CELL_TEX = load_texture("res/cell.png");
SDL_Texture* P_DEAD_CELL_TEX = load_texture("res/deadCell.png");
SDL_Texture* P_BKGND_TEX = load_texture("res/bkgnd.png");



SDL_Texture* load_texture(const char* filePath){
    SDL_Texture* ans = IMG_LoadTexture(P_RENDERER, filePath);
    assert(ans != NULL);
    return ans;
}

// TODO: Eventually, I will want a draw_frame function
//  which includes this function for every object, except
//  the SDL_RenderPresent portion is ran only once
void draw_texture(SDL_Texture* pTexture, int xPos, int yPos, int height, int width){
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
    SDL_RenderCopy(P_RENDERER, pTexture, NULL, pDst);
    //  This renders the opject according to pDst
    //  I could replace NULL with pSrc, but I don't know what pSrc refers to
}

SDL_Window* init_SDL_window(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* pWindow = SDL_CreateWindow(
        WINDOW_TITLE, 0, 20, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI
    );
    assert(pWindow != NULL);
    return pWindow;
}

SDL_Renderer* init_SDL_renderer(){
    SDL_Renderer* pRenderer = SDL_CreateRenderer(
        P_WINDOW, -1, SDL_RENDERER_ACCELERATED
    );
    assert(pRenderer != NULL);
    return pRenderer;
}

void wait_for_user_to_exit_SDL(){
    SDL_Event windowEvent;
    if(!simIsRunning) return;
    while(true){
        if(SDL_PollEvent(&windowEvent)){
            if(windowEvent.type == SDL_QUIT){
                simIsRunning = false;
                return;
            }
        }
    }
}

void exit_SDL(){
    SDL_DestroyWindow(P_WINDOW);
    SDL_Quit();
    std::cout << "SDL is quit!\n";
}


// Handle events between frames using SDL
//  e.g. Allow the user to decide when to advance to the next frame
//bool KEYS[322] = {0}; // 322 is the number of SDLK_DOWN events
//SDL_EnableKeyRepeat(0,0); // ???
void SDL_event_handler(){
    bool pauseSim = true;
    SDL_Event windowEvent;
    while(pauseSim){
        while(SDL_WaitEvent(&windowEvent)){
            switch(windowEvent.type){
                case SDL_KEYDOWN:
                switch(windowEvent.key.keysym.sym){
                    case SDLK_ESCAPE:
                    simIsRunning = false;
                    case SDLK_n:
                    case SDLK_SPACE:
                    pauseSim = false;
                    break;
                }
                break;
                case SDL_QUIT:
                simIsRunning = false;
                pauseSim = false;
                break;
            }
            if(!pauseSim) break;
        }
    }
    if(!simIsRunning) std::cout << "Simulation will exit..." << std::endl;
}



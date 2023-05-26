// Other #includes (if they exist)
//  I MUST check the license of each

// Microsoft C++ Standard Library
//  See https://learn.microsoft.com/en-us/cpp/standard-library/cpp-standard-library-header-files?view=msvc-170
//  for a list of these files
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <tuple>
#include <vector>


// Needed for SDL2 and SDL2_image to work
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// #includes that I made
#include "globalVals.h"
#include "debug.h"


// Initialize SDL Frame Rendering, Textures, etc.
SDL_Window* init_SDL_window(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* pWindow = SDL_CreateWindow(
        WINDOW_TITLE, 0, 20, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI
    );
    assert(pWindow != NULL);
    return pWindow;
}
SDL_Window* P_WINDOW = init_SDL_window();
SDL_Renderer* init_SDL_renderer(){
    SDL_Renderer* pRenderer = SDL_CreateRenderer(
        P_WINDOW, -1, SDL_RENDERER_ACCELERATED
    );
    assert(pRenderer != NULL);
    return pRenderer;
}
SDL_Renderer* P_RENDERER = init_SDL_renderer();

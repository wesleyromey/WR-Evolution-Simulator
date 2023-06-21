// The includes in this file only depend on mainIncludes.h
#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif

#include "custom.h"
#include "eventHandling.h"
#include "images.h"


void exit_SDL(){
    // Technically, I'm supposed to also call SDL_DestoryTexture(pTexture), but
    //  there are many textures to destroy
    //  TODO: create a list of pointers to SDL Textures then call
    //  SDL_DestroyTexture(pTexture) for all of these textures
    for(auto pTex : SDLTextureList) SDL_DestroyTexture(pTex);
    SDL_DestroyRenderer(P_RENDERER);
    SDL_DestroyWindow(P_WINDOW);
    SDL_Quit();
    std::cout << "SDL is quitting!\n";
}

void enforce_frame_rate(Uint32 frameStart, Uint32 frameDelay){
    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if(frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
}
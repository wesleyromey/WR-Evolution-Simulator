// Here, I will make a bunch of frames which can be used for a video

#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

#ifndef SIM_H
#include "sim.h"
#define SIM_H
#endif




// NOTE: This function may stop working properly I update my simulator and
//  (possibly) create new videos
void do_video1(){
    deallocate_all_cells();
    SDL_RenderClear(P_RENDERER);

    // Create the background using custom RGB values
    int red = 50, grn = 50, blue = 50;
    SDL_SetRenderDrawColor(P_RENDERER, red, grn, blue, SDL_ALPHA_OPAQUE);
    SDL_Rect bkgnd = {0, 0, UB_X_PX, UB_Y_PX};
    SDL_RenderDrawRect(P_RENDERER, &bkgnd);
    SDL_RenderFillRect(P_RENDERER, &bkgnd);

    // Start by generating 4 cells (plant, worm, predator, and [balanced] mutant)
    // This is mostly for drawing text
    draw_text(20,20,60,60,0,0,"testing text");

    //SDL_Delay(10000);

    //simState = SIM_STATE_QUIT;
    draw_user_interface(pAlives.size());
    //SDL_Delay(5000);
    SDL_RenderPresent(P_RENDERER);
}
// Here, I will make a bunch of frames which can be used for a video

#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

#ifndef SIM_H
#include "sim.h"
#define SIM_H
#endif




void gen_demo_cells_video1(int frameNum, int xPos, int yPos){
    std::map<std::string, int> varVals;
    varVals["age"] = 0;
    varVals["attack"] = 0;
    varVals["attackCooldown"] = 10;
    varVals["cloningDirection"] = 0;
    varVals["dia"] = 2;
    varVals["doAttack"] = false;
    varVals["doSelfDestruct"] = false;
    varVals["EAM[EAM_SUN]"] = 100;
    varVals["EAM[EAM_GND]"] = 0;
    varVals["EAM[EAM_CELLS]"] = 0;
    varVals["energy"] = 1000;
    varVals["maxEnergy"] = 10000;
    varVals["health"] = 1;
    varVals["maxHealth"] = 1;
    varVals["mutationRate"] = 0;
    varVals["posX"] = xPos;
    varVals["posY"] = yPos;
    varVals["speedDir"] = 0;
    varVals["speedMode"] = IDLE_MODE;
    varVals["speedRun"] = 0;
    varVals["speedWalk"] = 0;
    varVals["visionDist"] = 0;

    gen_cell();
    switch(frameNum){
        case 2:
        varVals["dia"] = 5;
        varVals["maxEnergy"] = 1000;
        varVals["energy"] = 500;
        pAlives[0]->set_int_stats(varVals);
        cout << pAlives[0]->energy << ", " << pAlives[0]->maxEnergy << endl;
        break;
    }
}

void clear_frame(){
    SDL_RenderClear(P_RENDERER);
    // Create the background using custom RGB values
    int red = 50, grn = 50, blue = 50;
    SDL_SetRenderDrawColor(P_RENDERER, red, grn, blue, SDL_ALPHA_OPAQUE);
    SDL_Rect bkgnd = {0, 0, UB_X_PX, UB_Y_PX};
    SDL_RenderDrawRect(P_RENDERER, &bkgnd);
    SDL_RenderFillRect(P_RENDERER, &bkgnd);
}

// NOTE: This function may stop working properly I update my simulator and
//  (possibly) create new videos
void do_video1(){
    assert(UB_X <= 30 && UB_Y <= 20);
    if(frameNum == 0){
        deallocate_all_cells();
        doCellAi = false;
        automateEnergy = false;
    }
    //SDL_RenderClear(P_RENDERER);
    clear_frame();

    // Start by generating 4 cells (plant, worm, predator, and [balanced] mutant)
    // This is mostly for drawing text
    int x0 = 20, y0 = 20, textWidth = 60, textHeight = 60;
    int numFrames = 10;
    string text = "";
    frameNum %= numFrames;
    switch(frameNum){
        case 0:
        draw_text(x0,y0,textWidth,textHeight,0,0,"testing text");
        break;
        case 1:
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"I will talk\nabout  them\n   later");
        break;
        case 2:
        // Plant cell gets sunlight rapidly
        gen_demo_cells_video1(frameNum, 10, 10);
        init_sim_gnd_energy(0);
        gndEnergyPerIncrease.set_val(0);
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        energyFromSunPerSec = 50;
        for(auto pAlive : pAlives) pAlive->set_ai_outputs(0, 0, IDLE_MODE, false, false, false);
        for(auto pAlive : pAlives) pAlive->energy += energyFromSunPerSec;
        SDL_draw_frame();
        break;
    }
    if(frameNum > 7){
        text = "end of text lol\nFrame " + conv_int_to_str(frameNum);
        text += " of " + conv_int_to_str(numFrames);
        draw_text(x0,y0,textWidth,textHeight,0,0,text);
    }
    
    

    //SDL_Delay(10000);
    //simState = SIM_STATE_QUIT;
    //draw_user_interface(pAlives.size());
    //SDL_Delay(5000);
    SDL_RenderPresent(P_RENDERER);
}
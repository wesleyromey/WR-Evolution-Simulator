// Here, I will make a bunch of frames which can be used for a video

#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

#ifndef SIM_H
#include "sim.h"
#define SIM_H
#endif




void gen_demo_cells_video1(int scenarioNum, int xPos, int yPos){
    std::map<std::string, int> varVals;
    varVals["age"] = 0;
    varVals["attack"] = 0;
    varVals["attackCooldown"] = 10;
    varVals["cloningDirection"] = 0;
    varVals["dia"] = 2;
    varVals["doAttack"] = false;
    varVals["doSelfDestruct"] = false;
    varVals["EAM[EAM_SUN]"] = 0;
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
    varVals["speedIdle"] = 0;
    varVals["visionDist"] = 0;


    switch(scenarioNum){
        case 1:
        automateEnergy = false;
        gen_cell();
        varVals["dia"] = 5;
        varVals["EAM[EAM_SUN]"] = 100;
        //varVals["maxEnergy"] = 10000;
        //varVals["energy"] = 1000;
        pAlives[0]->set_int_stats(varVals);
        //cout << pAlives[0]->energy << ", " << pAlives[0]->maxEnergy << endl;
        automateEnergy = true;
        break;
        case 2:
        automateEnergy = false;
        gen_cell();
        varVals["dia"] = 5;
        varVals["EAM[EAM_GND]"] = 100;
        varVals["speedWalk"] = 1; varVals["speedRun"] = 1;
        varVals["maxEnergy"] = 10000;
        pAlives[0]->set_int_stats(varVals);
        automateEnergy = true;
        break;
        case 3:
        deallocate_all_cells();

        // Create a plant, worm, and predator
        automateEnergy = false;
        dayNightMode.set_val(DAY_NIGHT_ALWAYS_DAY_MODE);
        maxSunEnergyPerSec.set_val(50);
        varVals["dia"] = 5; varVals["attack"] = 0;
        varVals["energy"] = 15000; varVals["maxEnergy"] = 25000;
        enableAutomaticAttack = true;
        maxGndEnergy.set_val(100);
        init_sim_gnd_energy(100);
        gndEnergyPerIncrease.set_val(10);

        gen_cell();
        varVals["EAM[EAM_SUN]"] = 100; varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_CELLS]"] = 0;
        varVals["speedWalk"] = 0; varVals["speedRun"] = 0;
        varVals["posX"] = 2; varVals["posY"] = 6;
        pAlives[0]->set_int_stats(varVals);
        gen_cell();
        varVals["EAM[EAM_SUN]"] = 0; varVals["EAM[EAM_GND]"] = 100; varVals["EAM[EAM_CELLS]"] = 0;
        varVals["speedIdle"] = 1; varVals["speedWalk"] = 1; varVals["speedRun"] = 1;
        varVals["posX"] = 2; varVals["posY"] = 12;
        pAlives[1]->set_int_stats(varVals);
        gen_cell();
        varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_CELLS]"] = 100;
        varVals["speedWalk"] = 3; varVals["speedRun"] = 3;
        varVals["posX"] = xPos + 10; varVals["posY"] = 14;
        varVals["energy"] = 5000; varVals["maxEnergy"] = 10000;
        varVals["attack"] = 1;
        pAlives[2]->set_int_stats(varVals);

        automateEnergy = true;
        break;

        case 4:
        // Show a worm losing energy faster and faster despite getting identical sunlight
        // We may need a smaller window size for the remaining simulations
        gen_cell();
        dayNightMode.set_val(DAY_NIGHT_ALWAYS_DAY_MODE);
        maxSunEnergyPerSec.set_val(0);
        init_sim_gnd_energy(200);
        gndEnergyPerIncrease.set_val(20);
        varVals["EAM[EAM_SUN]"] = 0; varVals["EAM[EAM_GND]"] = 100; varVals["EAM[EAM_CELLS]"] = 0;
        varVals["speedIdle"] = 1; varVals["speedWalk"] = 1; varVals["speedRun"] = 1;
        varVals["posX"] = 1; varVals["posY"] = 1;
        pAlives[0]->set_int_stats(varVals);
        break;
        

        //default:
        // Use Paint to illustrate most of the 2b portions (health, size)
        //  Simulate 2b (energy) and 2b (age)
        //  2c-2e was likely already captured, so I should probably show them
        //      beside the relevant stats when describing them
        // Need a new scenario to simulate 3d, 3f, 3g, 3i, 3j, 3k, 3l
        // Need new scenarios to simulate 4a (speed and direction, self-destruct)
        // Need new scenarios to simulate 4d, 4e, 4i, 6b, 6c, 6d, 6e, 6h, 7e, 7jk
        // Need text for 4a, 5a, and section 7.
        // Can most likely use simulations or windows I already have
        //  for the remaining animations and text.
    }
}

void clear_frame(){
    SDL_RenderClear(P_RENDERER);
    // Create the background using custom RGB values
    int red = 50, grn = 50, blue = 50;
    SDL_SetRenderDrawColor(P_RENDERER, red, grn, blue, SDL_ALPHA_OPAQUE);
    SDL_Rect bkgnd = {0, 0, ubX_px, ubY_px};
    SDL_RenderDrawRect(P_RENDERER, &bkgnd);
    SDL_RenderFillRect(P_RENDERER, &bkgnd);
}

/*
void simulate_confused_movement(Cell* pCell, float chanceToChangeSpeed = 0.01, float chanceToChangeDir = 0.01){
    // Based on random rng
    if(std_uniform_dist(rng) < chanceToChangeSpeed * 1.5){
        pCell->speedMode = gen_uniform_int_dist(rng, IDLE_MODE, RUN_MODE);
    }
    if(std_uniform_dist(rng) < chanceToChangeDir){
        pCell->speedDir = gen_uniform_int_dist(rng, 0, 359);
    }
}
*/

// NOTE: This function may stop working properly I update my simulator and
//  (possibly) create new videos
void do_video1(){
    assert(UB_X <= 30 && UB_Y <= 20);
    if(frameNum == 0){
        deallocate_all_cells();
        doCellAi = true;
        automateEnergy = true;
        bool enableAutomaticAttack = false;
        bool enableAutomaticSelfDestruct = false;
        bool enableAutomaticCloning = false;
    }
    //SDL_RenderClear(P_RENDERER);
    clear_frame();

    // Start by generating 4 cells (plant, worm, predator, and [balanced] mutant)
    // This is mostly for drawing text
    int x0 = 20, y0 = 20, textWidth = 60, textHeight = 60;
    int numFrames = 10000;
    string text = "";
    frameNum %= numFrames;
    if(frameNum == 0) draw_text(x0,y0,textWidth,textHeight,0,0,"testing text");
    else if(frameNum == 1){
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"I will talk\nabout  them\n   later");
        frameNum = 2000;
    }
    else if(frameNum == 2){
        // Plant cell gets sunlight rapidly, then dies at the end of the long night
        gen_demo_cells_video1(1, 10, 10);
        init_sim_gnd_energy(0);
        gndEnergyPerIncrease.set_val(0);
        
        dayNightMode.set_val(DAY_NIGHT_DEFAULT_MODE);
        maxSunEnergyPerSec.set_val(200);
        dayLenSec.set_val(1000);
        dayNightCycleTime = 0;
        for(auto pAlive : pAlives) pAlive->set_ai_outputs(0, 0, IDLE_MODE, false, false, false);
        //cout << dayNightCycleTime << endl;
    } else if(frameNum <= 1200){
        //cout << dayNightCycleTime << endl;
        for(auto pAlive : pAlives) pAlive->set_ai_outputs(0, 0, IDLE_MODE, false, false, false);
        //for(auto pAlive : pAlives) pAlive->energy += energyFromSunPerSec;
        SDL_draw_frame();
    } else if(frameNum == 1201){
        gen_demo_cells_video1(2, 2, 2);
        maxGndEnergy.set_val(100);
        init_sim_gnd_energy(50);
        gndEnergyPerIncrease.set_val(10);
        dayNightMode.set_val(DAY_NIGHT_ALWAYS_DAY_MODE);
        maxSunEnergyPerSec.set_val(0);
    } else if(frameNum <= 2000){
        SDL_draw_frame();
    } else if(frameNum == 2001){
        gen_demo_cells_video1(3,3,4);
    } else if(frameNum <= 3000){
        // pAlives = {plant, worm, predator}
        //pAlives[1]->posX = frameNum % UB_X; pAlives[1]->posY = 10;
        SDL_draw_frame();
    } else if(frameNum == 3001){
        // This is the last frame for the introduction
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"(Pretend this is true)");
    } else if(frameNum < 3020){
        // Show a predator killing a worm, teleporting to its corpse, then staying there and gaining energy rapidly
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"TODO");
    }
    
    if (frameNum > 5000) {
        text = "end of animations lol\nFrame " + conv_int_to_str(frameNum);
        text += " of " + conv_int_to_str(numFrames);
        draw_text(x0,y0,textWidth,textHeight,0,0,text);
    }
    
    

    //SDL_Delay(10000);
    //simState = SIM_STATE_QUIT;
    //draw_user_interface(pAlives.size());
    //SDL_Delay(5000);
    SDL_RenderPresent(P_RENDERER);
}
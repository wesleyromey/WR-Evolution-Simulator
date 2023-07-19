// Here, I will make a bunch of frames which can be used for a video

#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

#ifndef SIM_H
#include "sim.h"
#define SIM_H
#endif


std::map<std::string, int> gen_std_plant_stats(int posX, int posY, int dia, int initEnergy, int maxEnergy){
    std::map<std::string, int> varVals;
    varVals["age"] = 0; varVals["attack"] = 0; varVals["attackCooldown"] = 10;
    varVals["dia"] = dia;
    varVals["EAM[EAM_SUN]"] = 100; varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_CELLS]"] = 0;
    varVals["energy"] = initEnergy; varVals["maxEnergy"] = maxEnergy;
    varVals["maxHealth"] = 1; varVals["health"] = varVals["maxHealth"];
    varVals["mutationRate"] = 0;
    varVals["posX"] = posX; varVals["posY"] = posY;
    varVals["speedIdle"] = 0; varVals["speedWalk"] = 0; varVals["speedRun"] = 0;
    varVals["visionDist"] = 0;
    return varVals;
}
std::map<std::string, int> gen_std_worm_stats(int posX, int posY, int dia, int initEnergy, int maxEnergy){
    std::map<std::string, int> varVals;
    varVals["age"] = 0; varVals["attack"] = 0; varVals["attackCooldown"] = 10;
    varVals["dia"] = dia;
    varVals["EAM[EAM_SUN]"] = 0; varVals["EAM[EAM_GND]"] = 100; varVals["EAM[EAM_CELLS]"] = 0;
    varVals["energy"] = initEnergy; varVals["maxEnergy"] = maxEnergy;
    varVals["health"] = 1; varVals["maxHealth"] = 1;
    varVals["mutationRate"] = 0;
    varVals["posX"] = posX; varVals["posY"] = posY;
    varVals["speedIdle"] = 0; varVals["speedWalk"] = 1; varVals["speedRun"] = 2;
    varVals["visionDist"] = 0;
    return varVals;
}
std::map<std::string, int> gen_std_predator_stats(int posX, int posY, int dia, int initEnergy, int maxEnergy){
    std::map<std::string, int> varVals;
    varVals["age"] = 0; varVals["attack"] = 1; varVals["attackCooldown"] = 10;
    varVals["dia"] = dia;
    varVals["EAM[EAM_SUN]"] = 0; varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_CELLS]"] = 100;
    varVals["energy"] = initEnergy; varVals["maxEnergy"] = maxEnergy;
    varVals["health"] = 1; varVals["maxHealth"] = 1;
    varVals["mutationRate"] = 0;
    varVals["posX"] = posX; varVals["posY"] = posY;
    varVals["speedIdle"] = 0; varVals["speedWalk"] = 1; varVals["speedRun"] = 2;
    varVals["visionDist"] = 0;
    return varVals;
}

void gen_demo_cells_video1(int scenarioNum){
    dayNightCycleTime = 0;
    
    //int testVar1 = 102, testVar2 = 14;
    //set_vals(&testVar1, 30, &testVar2, 50);
    //print_scalar_vals("testVar1", testVar1, "testVar2", testVar2);
    std::map<std::string, int> varVals;
    switch(scenarioNum){
        case 1:
        // Plant cell gets sunlight rapidly, then dies at the end of the long night
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy, &dayLenSec},
            {6, 4, DAY_NIGHT_DEFAULT_MODE, 200, 0, 100, 200});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(1, 1, 2, 100, 600);
        varVals["maxHealth"] = 60; varVals["health"] = varVals["maxHealth"];
        pAlives[0]->set_int_stats(varVals);
        //print_scalar_vals("EAM[EAM_SUN]", varVals["EAM[EAM_SUN]"], "EAM[EAM_SUN]", pAlives[0]->EAM[EAM_SUN]);
        automateEnergy = true;
        break;

        case 2:
        // A lone worm gains energy from the ground
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {6, 4, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 10, 100});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(1, 1, 2, 1000, 10000);
        varVals["speedIdle"] = 1; varVals["speedWalk"] = 1; varVals["speedRun"] = 1;
        pAlives[0]->set_int_stats(varVals);
        automateEnergy = true;
        break;

        case 3:
        // A plant and worm exist specifically to be eaten by a lone predator
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {9, 6, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 10, 100});
        set_vals(&automateEnergy, false, &enableAutomaticAttack, true);
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(2, 6, 2, 15000, 25000);
        pAlives[0]->set_int_stats(varVals);
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(2, 12, 2, 15000, 25000);
        varVals["speedIdle"] = 1; varVals["speedWalk"] = 1; varVals["speedRun"] = 1;
        pAlives[1]->set_int_stats(varVals);
        gen_cell();
        varVals.clear(); varVals = gen_std_predator_stats(10, 14, 2, 5000, 10000);
        varVals["speedWalk"] = 3; varVals["speedRun"] = 3;
        pAlives[2]->set_int_stats(varVals);

        automateEnergy = true;
        break;

        case 4:
        // Show a worm losing energy faster and faster despite getting identical sunlight
        // We may need a smaller window size for the remaining simulations
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {6, 4, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 20, 200});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(1, 1, 2, 5000, 10000);
        varVals["speedIdle"] = 1; varVals["speedWalk"] = 1; varVals["speedRun"] = 1;
        pAlives[0]->set_int_stats(varVals);
        automateEnergy = true;
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

// NOTE: This function may stop working properly I update my simulator and
//  (possibly) create new videos
void do_video1(){
    clear_frame();

    int x0 = 20, y0 = 20, textWidth = 60, textHeight = 60;
    int numFrames = 10000;
    string text = "";
    static const int kF0 = 0, kF1 = 250, kF2 = 1000, kF3 = 2000, kF4 = 3000; // key frames
    frameNum %= numFrames;
    switch(frameNum){
        case kF0:
        deallocate_all_cells();
        set_vals(&doCellAi, true, &automateEnergy, true, &enableAutomaticAttack, false,
            &enableAutomaticCloning, false, &enableAutomaticSelfDestruct, false);
        draw_text(x0,y0,textWidth,textHeight,0,0,"testing text");
        break;

        case kF0+1:
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"I will talk\nabout  them\n   later");
        break;

        case kF0+2:
        gen_demo_cells_video1(1);
        break;

        case kF1:
        gen_demo_cells_video1(2);
        break;

        case kF2:
        gen_demo_cells_video1(3);
        break;

        case kF3:
        // Show a predator killing a worm, teleporting to its corpse, then staying there and gaining energy rapidly
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"(Pretend this is true)");
        break;

        case kF3+1:
        gen_demo_cells_video1(4);
        break;

        default:
        if(frameNum < kF1){
            SDL_draw_frame();
        } else if(frameNum < kF2){
            pAlives[0]->set_ai_outputs(0, 0, WALK_MODE, false, false, false);
            SDL_draw_frame();
        } else if(frameNum < kF3){
            pAlives[1]->set_ai_outputs(0, 0, WALK_MODE, false, false, false);
            pAlives[2]->set_ai_outputs(0, 0, RUN_MODE, false, false, false);
            SDL_draw_frame();
        } else if(frameNum < kF4){
            pAlives[0]->age += 9;
            SDL_draw_frame();
        } else {
            text = "end of animations lol\nFrame " + conv_int_to_str(frameNum);
            text += " of " + conv_int_to_str(numFrames);
            draw_text(x0,y0,textWidth,textHeight,0,0,text);
        }
    }
    SDL_RenderPresent(P_RENDERER);
}
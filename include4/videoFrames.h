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
        pCellsHist[0]->set_int_stats(varVals, 0);
        //print_scalar_vals("EAM[EAM_SUN]", varVals["EAM[EAM_SUN]"], "EAM[EAM_SUN]", pCellsHist[0]->EAM[EAM_SUN]);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        automateEnergy = true;
        break;

        case 2:
        // A lone worm gains energy from the ground
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {15, 10, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 20, 200});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(1, 2, 5, 200, 2000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);
        automateEnergy = true;
        break;

        case 3:
        // A plant and worm exist specifically to be eaten by a lone predator
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {15, 10, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 10, 100});
        set_vals(&automateEnergy, false, &enableAutomaticAttack, true);
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(2, 1, 3, 1000, 15000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(6, 5, 2, 1000, 10000);
        pCellsHist[1]->set_int_stats(varVals, 0);
        pCellsHist[1]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);
        gen_cell();
        varVals.clear(); varVals = gen_std_predator_stats(2, 7, 2, 1000, 10000);
        pCellsHist[2]->set_int_stats(varVals);
        pCellsHist[2]->force_decision(10, 0, 0, WALK_MODE, true, false, false);     // (12,7)|(1,5)
        pCellsHist[2]->force_decision(2, 315, 0, RUN_MODE, true, false, false);     // (14,5)|(3,5)
        pCellsHist[2]->force_decision(3, 0, 0, RUN_MODE, true, false, false);       // (5,7)|(6,5)
        pCellsHist[2]->force_decision(6, 0, 0, IDLE_MODE, true, false, false);      // (5,7)
        pCellsHist[2]->force_decision(3, 0, 0, WALK_MODE, true, false, false);      // (8,7)
        pCellsHist[2]->force_decision(4, 270, 0, WALK_MODE, true, false, false);    // (8,3)
        pCellsHist[2]->force_decision(5, 180, 0, WALK_MODE, true, false, false);    // (3,3)
        pCellsHist[2]->force_decision(8, 0, 0, IDLE_MODE, true, false, false);      // (3,3)
        pCellsHist[2]->force_decision(1000, 0, 0, WALK_MODE, true, false, false);
        automateEnergy = true;
        break;

        case 4:
        // Show a worm losing energy faster and faster despite getting identical sunlight
        // We may need a smaller window size for the remaining simulations
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 40, 200});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(10, 5, 8, 500, 5000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);
        automateEnergy = true;
        break;


        case 5:
        // 1f: Show a plant, worm, and predator cloning themselves as they gain energy
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 40, 200});
        automateEnergy = false;
        // pCellsHist = { plant, worm, predator, plant, worm}
        // All cells want to clone themselves at all times
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(5, 6, 2, 1500, 4000);
        pCellsHist[0]->set_int_stats(varVals);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, true);
        gen_cell();
        varVals.clear(); varVals = gen_std_worm_stats(21, 10, 2, 1000, 4000);
        pCellsHist[1]->set_int_stats(varVals);
        pCellsHist[1]->force_decision(1000, 0, 70, WALK_MODE, false, false, true);
        gen_cell();
        varVals.clear(); varVals = gen_std_predator_stats(15, 17, 2, 1000, 4000);
        pCellsHist[2]->set_int_stats(varVals);
        pCellsHist[2]->force_decision(15, 0, 270, WALK_MODE, true, false, true);
        pCellsHist[2]->force_decision(6, 315, 270, RUN_MODE, true, false, true);
        pCellsHist[2]->force_decision(6, 0, 270, RUN_MODE, true, false, true);
        pCellsHist[2]->force_decision(100, 0, 270, IDLE_MODE, true, false, true);
        automateEnergy = true;
        break;

        
        case 6:
        // 2c - Show a cell being attacked and losing some of their health
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 40, 200});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(18, 10, 7, 11000, 20000);
        varVals["maxHealth"] = 10; varVals["health"] = varVals["maxHealth"];
        pCellsHist[0]->set_int_stats(varVals);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        gen_cell();
        varVals.clear(); varVals = gen_std_predator_stats(5, 1, 2, 1000, 5000);
        varVals["speedRun"] = 3; varVals["attackCooldown"] = 0;
        pCellsHist[1]->set_int_stats(varVals);
        pCellsHist[1]->force_decision(5, 330, 0, RUN_MODE, true, false, false);
        pCellsHist[1]->force_decision(3, 0, 0, WALK_MODE, true, false, false);
        pCellsHist[1]->force_decision(2, 270, 0, WALK_MODE, true, false, false);
        pCellsHist[1]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false);
        gen_cell();
        varVals["posX"] = 20; varVals["posY"] = 7; varVals["speedRun"] = 3;
        pCellsHist[2]->set_int_stats(varVals);
        pCellsHist[2]->force_decision(10, 330, 0, RUN_MODE, true, false, false);
        pCellsHist[2]->force_decision(3, 270, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(13, 180, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(2, 270, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(6, 180, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false);
        gen_cell();
        varVals["posX"] = 0; varVals["posY"] = 0; varVals["speedRun"] = 2;
        pCellsHist[3]->set_int_stats(varVals);
        pCellsHist[3]->force_decision(4, 45, 0, RUN_MODE, true, false, false);
        pCellsHist[3]->force_decision(17, 0, 0, WALK_MODE, true, false, false);
        pCellsHist[3]->force_decision(5, 90, 0, WALK_MODE, true, false, false);
        pCellsHist[3]->force_decision(100, 0, 0, IDLE_MODE, true, false, false);
        automateEnergy = true;
        break;

        case 7:
        // 2b - Show several looped animations
        // Energy - Show a plant gaining energy (loop through this animation quickly)
        // Health - Show a plant losing 1 health at a time (loop through this animation quickly)
        // Age - Show small text "idk how to illustrate"
        //      Immediately after that text appears, replace it with
        //      a plant pointing to a dead cell
        // Size - Show a diameter 2 plant pointing to a diameter 4 plant (use Paint 3d)
        // Remember to show the actual text for the options (energy, health, age, size)
        deallocate_all_cells();
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {6, 4, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 0, 1});
        automateEnergy = false;
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(0, 0, 2, 1000, 10000);
        varVals["maxHealth"] = 10; varVals["health"] = varVals["maxHealth"];
        pCellsHist[0]->set_int_stats(varVals);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        gen_cell();
        varVals.clear(); varVals = gen_std_plant_stats(3, 0, 2, 1, 100000);
        pCellsHist[1]->set_int_stats(varVals);
        pCellsHist[1]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        automateEnergy = true;
        break;


        //default:
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
    static const int kF0 = 0; // key frames
    static const int kF1c = kF0 + 10, kF1d = kF1c + 250, kF1e = kF1d + 70;
    static const int kF1f = kF1e + 55;
    static const int kF2b = kF1f + 70;
    static const int kF2d = kF2b + 40, kF2e = kF2d + 60, kF2f = kF2e + 300;
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
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"Pretend\n this  \nis true");
        break;
        case kF0+3:
        draw_text(x0,y0,4*textWidth,5*textHeight,0,3,"Energy\nHealth\nAge\nSize");
        break;
        case kF0+4:
        draw_text(x0,y0,textWidth,textHeight,0,1,"idk how to represent this");
        break;
        case kF0+5:
        case kF0+6:
        case kF0+7:
        case kF0+8:
        case kF0+9:
        frameNum = kF2b - 1;
        break;

        case kF1c:
        gen_demo_cells_video1(1);
        SDL_draw_frame();
        break;
        case kF1d:
        gen_demo_cells_video1(2);
        SDL_draw_frame();
        break;
        case kF1e:
        gen_demo_cells_video1(3);
        SDL_draw_frame();
        break;
        case kF1f:
        gen_demo_cells_video1(5);
        SDL_draw_frame();
        break;

        case kF2b:
        gen_demo_cells_video1(7);
        SDL_draw_frame();
        break;
        case kF2d:
        gen_demo_cells_video1(6);
        SDL_draw_frame();
        break;
        case kF2e:
        gen_demo_cells_video1(4);
        SDL_draw_frame();
        break;
        


        default:
        if(kF2b <= frameNum && frameNum < kF2d){
            pCellsHist[0]->health--;
            pCellsHist[0]->energy = pCellsHist[0]->maxEnergy / 9;
            if(pCellsHist[1]->energy < pCellsHist[1]->maxEnergy / 10){
                pCellsHist[1]->energy += pCellsHist[1]->maxEnergy / 100;
            } else {
                pCellsHist[1]->energy += pCellsHist[1]->maxEnergy / 25;
            }
        } else if(kF2e <= frameNum && frameNum < kF2f){
            pCellsHist[0]->age += 49;
        }
        
        if (kF2f <= frameNum) {
            text = "end of animations lol\nFrame " + conv_int_to_str(frameNum);
            text += " of " + conv_int_to_str(numFrames);
            draw_text(x0,y0,textWidth,textHeight,0,0,text);
        } else {
            SDL_draw_frame();
        }
    }
    SDL_RenderPresent(P_RENDERER);
}
// Here, I will make a bunch of frames which can be used for a video

#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

#ifndef SIM_H
#include "sim.h"
#define SIM_H
#endif


std::map<std::string, int> gen_std_stats(std::string cellType, int posX, int posY, int dia, int initEnergy, int maxEnergy,
int maxHealth = 1, int initHealthPct = 100, int attack = 1, int attackCooldown = 10, int age = 0, int mutationRate = 0,
int speedIdle = 0, int speedWalk = 1, int speedRun = 2, int visionDist = 0){
    std::map<std::string, int> varVals;
    varVals["age"] = age; varVals["attack"] = max_int(attack, 0); varVals["attackCooldown"] = attackCooldown;
    varVals["dia"] = dia;
    if(cellType == "plant"){
        varVals["EAM[EAM_SUN]"] = 100; varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_CELLS]"] = 0;
    } else if(cellType == "worm"){
        varVals["EAM[EAM_SUN]"] = 0; varVals["EAM[EAM_GND]"] = 100; varVals["EAM[EAM_CELLS]"] = 0;
    } else if(cellType == "predator"){
        varVals["EAM[EAM_SUN]"] = 0; varVals["EAM[EAM_GND]"] = 0; varVals["EAM[EAM_CELLS]"] = 100;
    } else {
        varVals["EAM[EAM_SUN]"] = 34; varVals["EAM[EAM_GND]"] = 33; varVals["EAM[EAM_CELLS]"] = 33;
    }
    varVals["energy"] = initEnergy; varVals["maxEnergy"] = maxEnergy;
    varVals["maxHealth"] = maxHealth; varVals["health"] = varVals["maxHealth"] * initHealthPct / 100;
    varVals["mutationRate"] = mutationRate;
    varVals["posX"] = posX; varVals["posY"] = posY;
    varVals["speedIdle"] = speedIdle; varVals["speedWalk"] = speedWalk; varVals["speedRun"] = speedRun;
    varVals["visionDist"] = visionDist;
    return varVals;
}

// Generate random-ish movement that looks reasonable for a cell to do
void preplan_random_cell_activity(Cell* pCell, int pctChanceToChangeDir, int pctChanceToChangeSpeed, int numFrames,
bool _enableAttack, bool _enableCloning){
    int newSpeedDir = pCell->speedDir, newSpeedMode = pCell->speedMode;
    int numFramesSinceLastDecision = 0;
    bool changedSpeedDir = false;
    while(numFrames > 0){
        if(rand() % 100 < pctChanceToChangeDir){
            while(newSpeedDir == pCell->speedDir) newSpeedDir = rand() % 360;
            changedSpeedDir = true;
        }
        if(rand() % 100 < pctChanceToChangeSpeed){
            while(newSpeedMode == pCell->speedMode) newSpeedMode = rand() % 3;
            changedSpeedDir = true;
        }
        if(changedSpeedDir || numFramesSinceLastDecision >= numFrames){
            changedSpeedDir = false;
            pCell->force_decision(numFramesSinceLastDecision, newSpeedDir, rand() % 360, newSpeedMode, _enableAttack, false, _enableCloning);
            //print_scalar_vals("newSpeedDir", newSpeedDir, "newSpeedMode", newSpeedMode);
            numFrames -= numFramesSinceLastDecision;
            numFramesSinceLastDecision = 0;
        } else {
            numFramesSinceLastDecision++;
        }
    }
}


void preplan_shortest_path_to_point(Cell* pCell, int posX, int posY, int targetX, int targetY, bool enableRunning,
bool _enableAttack, bool _enableCloning){
    int targetDx = targetX - posX, targetDy = targetY - posY;
    int optimalDir = 0;
    float optimalDistance = abs(targetDx) + abs(targetDy) + 1;
    int speed = enableRunning*pCell->speedRun + !enableRunning*pCell->speedWalk;
    int nextPosX = posX, nextPosY = posY;
    for(int testDir = 0; testDir < 360; testDir += 15){
        int testDx = speed * cos_deg(testDir);
        int testDy = speed * sin_deg(testDir);
        float testDistance = calc_distance_between_points(posX + testDx, posY + testDy, targetX, targetY);
        //cout << testDx << ", " << testDy << ", " << speed << ", " << testDistance << endl;
        if(testDistance < optimalDistance){
            optimalDistance = testDistance;
            optimalDir = testDir;
            nextPosX = posX + testDx; nextPosY = posY + testDy;
        }
    }
    int _speedMode = (enableRunning ? RUN_MODE : WALK_MODE);
    float targetDistance = calc_distance_between_points(posX, posY, targetX, targetY);
    float newTargetDistance = calc_distance_between_points(nextPosX, nextPosY, targetX, targetY);
    //print_scalar_vals("newTargetDistance", newTargetDistance, "targetDistance", targetDistance);
    if(newTargetDistance >= targetDistance){
        if(!enableRunning) return;
        preplan_shortest_path_to_point(pCell, nextPosX, nextPosY, targetX, targetY, false, _enableAttack, _enableCloning);
        return;
    }
    pCell->force_decision(1, optimalDir, rand() % 360, _speedMode, _enableAttack, false, _enableCloning);
    preplan_shortest_path_to_point(pCell, nextPosX, nextPosY, targetX, targetY, enableRunning, _enableAttack, _enableCloning);
}

void gen_demo_cells_video1(int scenarioNum){
    // Shorthand functions for these scenarios
    #ifndef scenario_precode
    #define scenario_precode(numCells) { \
        deallocate_all_cells(); \
        automateEnergy = false; \
        randomly_place_new_cells(numCells); \
    }
    #endif
    #ifndef scenario_postcode
    #define scenario_postcode() { \
        automateEnergy = true; \
    }
    #endif
    
    
    std::map<std::string, int> varVals;
    int tmpVar;
    switch(scenarioNum){
        case 10:
        // Plant cell gets sunlight rapidly, then dies at the end of the long night
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy, &dayLenSec},
            {6, 4, DAY_NIGHT_DEFAULT_MODE, 200, 0, 100, 200});
        varVals.clear(); varVals = gen_std_stats("plant", 2, 2, 2, 100, 600, 60);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        scenario_postcode();
        break;

        case 11:
        // A lone worm gains energy from the ground
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {15, 10, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 20, 200});
        varVals.clear(); varVals = gen_std_stats("worm", 3, 4, 5, 200, 2000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);
        scenario_postcode();
        break;

        case 12:
        // A plant and worm exist specifically to be eaten by a lone predator
        scenario_precode(3);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {15, 10, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 10, 100});
        varVals.clear(); varVals = gen_std_stats("plant", 3, 2, 3, 1000, 15000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        varVals.clear(); varVals = gen_std_stats("worm", 7, 6, 2, 1000, 10000);
        pCellsHist[1]->set_int_stats(varVals, 0);
        pCellsHist[1]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);
        varVals.clear(); varVals = gen_std_stats("predator", 3, 8, 2, 1000, 10000);
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
        scenario_postcode();
        break;

        case 13:
        // 1f: Show a plant, worm, and predator cloning themselves as they gain energy
        scenario_precode(3);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 40, 200});
        varVals.clear(); varVals = gen_std_stats("plant", 6, 7, 2, 1500, 4000);
        pCellsHist[0]->set_int_stats(varVals);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, true);
        varVals.clear(); varVals = gen_std_stats("worm", 22, 11, 2, 1000, 4000);
        pCellsHist[1]->set_int_stats(varVals);
        pCellsHist[1]->force_decision(1000, 0, 70, WALK_MODE, false, false, true);
        varVals.clear(); varVals = gen_std_stats("predator", 16, 18, 2, 1000, 4000);
        pCellsHist[2]->set_int_stats(varVals);
        pCellsHist[2]->force_decision(15, 0, 270, WALK_MODE, true, false, true);
        pCellsHist[2]->force_decision(6, 315, 270, RUN_MODE, true, false, true);
        pCellsHist[2]->force_decision(6, 0, 270, RUN_MODE, true, false, true);
        pCellsHist[2]->force_decision(100, 0, 270, IDLE_MODE, true, false, true);
        scenario_postcode();
        break;

        case 20:
        // 2b - Show several looped animations
        // Energy - Show a plant gaining energy (loop through this animation quickly)
        // Health - Show a plant losing 1 health at a time (loop through this animation quickly)
        // Age - Show small text "idk how to illustrate"
        //      Immediately after that text appears, replace it with
        //      a plant pointing to a dead cell
        // Size - Show a diameter 2 plant pointing to a diameter 4 plant (use Paint 3d)
        // Remember to show the actual text for the options (energy, health, age, size)
        scenario_precode(2);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {6, 4, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 0, 1});
        varVals.clear(); varVals = gen_std_stats("plant", 1, 1, 2, 1000, 10000, 10);
        pCellsHist[0]->set_int_stats(varVals);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        varVals.clear(); varVals = gen_std_stats("plant", 4, 1, 2, 1, 100000);
        pCellsHist[1]->set_int_stats(varVals);
        pCellsHist[1]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        scenario_postcode();
        break;

        case 21:
        // 2c - Show a cell being attacked and losing some of their health
        scenario_precode(4);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 40, 200});
        varVals.clear(); varVals = gen_std_stats("plant", 21, 13, 7, 11000, 20000, 10);
        //varVals["maxHealth"] = 10; varVals["health"] = varVals["maxHealth"];
        pCellsHist[0]->set_int_stats(varVals);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        varVals.clear(); varVals = gen_std_stats("predator", 6, 2, 2, 1000, 5000, 1, 100,
        1, 0, 0, 0, 0, 1, 3, 0);
        pCellsHist[1]->set_int_stats(varVals);
        pCellsHist[1]->force_decision(5, 330, 0, RUN_MODE, true, false, false);
        pCellsHist[1]->force_decision(3, 0, 0, WALK_MODE, true, false, false);
        pCellsHist[1]->force_decision(2, 270, 0, WALK_MODE, true, false, false);
        pCellsHist[1]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false);
        varVals["posX"] = 21; varVals["posY"] = 8; varVals["speedRun"] = 3;
        pCellsHist[2]->set_int_stats(varVals);
        pCellsHist[2]->force_decision(10, 330, 0, RUN_MODE, true, false, false);
        pCellsHist[2]->force_decision(3, 270, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(13, 180, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(2, 270, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(6, 180, 0, WALK_MODE, true, false, false);
        pCellsHist[2]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false);
        varVals["posX"] = 1; varVals["posY"] = 1; varVals["speedRun"] = 2;
        pCellsHist[3]->set_int_stats(varVals);
        pCellsHist[3]->force_decision(4, 45, 0, RUN_MODE, true, false, false);
        pCellsHist[3]->force_decision(17, 0, 0, WALK_MODE, true, false, false);
        pCellsHist[3]->force_decision(5, 90, 0, WALK_MODE, true, false, false);
        pCellsHist[3]->force_decision(100, 0, 0, IDLE_MODE, true, false, false);
        scenario_postcode();
        break;

        case 22:
        // Show a worm losing energy faster and faster despite getting identical sunlight
        // We may need a smaller window size for the remaining simulations
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 40, 200});
        varVals.clear(); varVals = gen_std_stats("worm", 14, 9, 8, 500, 5000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);
        scenario_postcode();
        break;

        case 30:
        // (3d) A giant predator is attacked but not killed by a group of several weak predators.
        // The giant predator instantly kills all of them in one attack
        scenario_precode(5);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1});
        #define init_small_predator(varVals, cellNum, dx, dy, attackCooldown, speedDir){ \
            varVals.clear(); varVals = gen_std_stats("predator", 10 + dx, 10 + dy, 2, 1000, 10000, 1, 100, 1, attackCooldown); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
            pCellsHist[cellNum]->force_decision(6, speedDir, 0, WALK_MODE, true, false, false); \
            pCellsHist[cellNum]->force_decision(100, speedDir, 0, IDLE_MODE, true, false, false); \
        }
        varVals.clear(); varVals = gen_std_stats("predator", 10, 10, 6, 4000, 40000, 7);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(10, 0, 0, IDLE_MODE, true, false, false);
        init_small_predator(varVals, 1, -9,  0, 2, 0  );
        init_small_predator(varVals, 2,  0, -9, 2, 90 );
        init_small_predator(varVals, 3,  9,  0, 2, 180);
        init_small_predator(varVals, 4,  0,  9, 2, 270);
        #undef init_small_predator
        scenario_postcode();
        break;

        
        case 31:
        // (3g) 30x20 simulation where a size 6 plant and four adjacent size 2 plants gain energy
        // (none of which are touching each other). But partway through, the size 2 plants clone
        // themselves simultaneously such that the larger plant now has to compete with 4 new size 2 plants.
        // The size 2 plants are quickly suffocated by the size 6 plant.
        scenario_precode(5);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {16, 16, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 0, 1});
        #define init_plant(cellNum, varVals, dx, dy, dia, cloningDir, initEnergy) { \
            varVals.clear(); varVals = gen_std_stats("plant", 8 + dx, 8 + dy, dia, initEnergy, 2*initEnergy, 200); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
            pCellsHist[cellNum]->force_decision(50, 0, cloningDir, IDLE_MODE, false, false, (cellNum != 0)); \
            pCellsHist[cellNum]->force_decision(1000, 0, cloningDir, IDLE_MODE, false, false, false); \
        }
        init_plant(0, varVals,  0,  0, 6, 0  , 2500);
        init_plant(1, varVals, -5,  0, 2, 0  , 2500);
        init_plant(2, varVals,  0, -5, 2, 90 , 2500);
        init_plant(3, varVals,  5,  0, 2, 180, 2500);
        init_plant(4, varVals,  0,  5, 2, 270, 2500);
        #undef init_plant
        scenario_postcode();
        break;
        
        case 32:
        // (3i) 60x40 simulation. Start with twenty-five size 5 worms and twenty-five size 2 worms
        // travelling in random directions and speeds (choosing between remaining idle, moving 1 tile
        // diagonally, or moving 1-2 tiles adjacently) and cloning themselves when they have enough energy.
        // The ground energy has a small max amount per cell and regenerates at the normal rate.
        // As usual, self-destruct and mutations are disabled.
        scenario_precode(50);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {60, 40, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 10, 100});
        set_vals(&enableAutomaticAttack, false, &enableAutomaticCloning, true, &enableAutomaticSelfDestruct, false);
        // Generate random movement patterns in advance
        //  (output a speed mode and direction at random)
        #define init_worm(cellNum, varVals, posX, posY, dia){ \
            varVals.clear(); varVals = gen_std_stats("worm", posX, posY, dia, 1500, 5000*dia); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
        }
        //pCellsHist[cellNum]->force_decision(45, 0, 90, WALK_MODE, false, false, true);
        for(tmpVar = 0; tmpVar < pCellsHist.size(); tmpVar++){
            init_worm(tmpVar, varVals, 13*(tmpVar^2+tmpVar+100) % ubX.val, 17*(tmpVar^2+tmpVar+50) % ubY.val, 2);
            if(++tmpVar >= pCellsHist.size()) break;
            init_worm(tmpVar, varVals, 13*(tmpVar^2+tmpVar+50) % ubX.val, 17*(tmpVar^2+tmpVar+100) % ubY.val, 5);
        }
        #undef init_worm
        scenario_postcode();
        break;

        case 33:
        // (3j) 30x20 simulation showing four size 2 predators and a size 4 predator swarm a size 6 plant,
        // only for the size two predators to get less of the energy (ensure the max energy of each predator
        // is proportional to their diameter)
        //  NOTE: My prediction did NOT occur!
        //  TODO: Figure out why my prediction did not work
        scenario_precode(6);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 0, 1});
        varVals.clear(); varVals = gen_std_stats("plant", 15, 10, 8, 40000, 40000, 16);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        #define init_predator(cellNum, varVals, posX, posY, dia, targetX, targetY){ \
            varVals.clear(); varVals = gen_std_stats("predator", posX, posY, dia, 1000, 10000, dia*dia/2); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
            preplan_shortest_path_to_point(pCellsHist[cellNum], posX, posY, targetX, targetY, true, true, false); \
            pCellsHist[cellNum]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false); \
        }
        init_predator(1, varVals,  6,  9, 4, 16,  8);
        init_predator(2, varVals,  3,  3, 2, 12, 10);
        init_predator(3, varVals,  3, 17, 2, 12, 13);
        init_predator(4, varVals, 27,  3, 2, 15, 13);
        init_predator(5, varVals, 25, 18, 2, 18, 13);
        #undef init_predator
        scenario_postcode();
        break;


        default:
        cout << "NOTE: Scenario " << scenarioNum << " was supposed to be played, but that scenario is not available\n";
        // Need a new scenario to simulate 3d, 3g, 3i, 3j, 3k
        // Need new scenarios to simulate 4a (speed and direction, self-destruct)
        // Need new scenarios to simulate 4d, 4e, 4i, 6b, 6c, 6d, 6e, 6h, 7e, 7jk
        // Need text for 4a, 5a, and section 7.
        // Can most likely use simulations or windows I already have
        //  for the remaining animations and text.
    }

    dayNightCycleTime = 0;
    do_day_night_cycle();
    SDL_draw_frame();
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
    static const int kF0 = 0, kF1start = 10; // key frames
    static const int kF1c = kF1start, kF1d = kF1c + 250, kF1e = kF1d + 70;
    static const int kF1f = kF1e + 55, kF2start = kF1f + 70;
    static const int kF2b = kF2start, kF2d = kF2b + 40, kF2e = kF2d + 60;
    static const int kF3start = kF2e + 300;
    static const int kF3d = kF3start, kF3g = kF3d + 40, kF3i = kF3g + 180;
    static const int kF3j = kF3i + 2000, kF3k = kF3j + 200, kF4start = kF3k + 100;
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
        frameNum = kF3j - 1;
        break;

        case kF1c:
        gen_demo_cells_video1(10);
        break;
        case kF1d:
        gen_demo_cells_video1(11);
        break;
        case kF1e:
        gen_demo_cells_video1(12);
        break;
        case kF1f:
        gen_demo_cells_video1(13);
        break;

        case kF2b:
        gen_demo_cells_video1(20);
        break;
        case kF2d:
        gen_demo_cells_video1(21);
        break;
        case kF2e:
        gen_demo_cells_video1(22);
        break;

        case kF3d:
        gen_demo_cells_video1(30);
        break;
        case kF3g:
        gen_demo_cells_video1(31);
        break;
        case kF3i:
        gen_demo_cells_video1(32);
        break;
        case kF3j:
        gen_demo_cells_video1(33); // TODO
        break;
        case kF3k:
        gen_demo_cells_video1(34); // TODO
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
        } else if(kF2e <= frameNum && frameNum < kF3start){
            pCellsHist[0]->age += 49;
        } else if(kF3i < frameNum && frameNum < kF3j){
            //print_scalar_vals("frameNum", frameNum);
            //for(auto pCell : pAlives) print_scalar_vals("  energy", pCell->energy);
            //print_scalar_vals("frameNum", frameNum);
            for(auto pCell : pAlives){
                if(pCell->age == 0) pCell->clear_forced_decisions();
                //print_scalar_vals("fDQ.size()", pCell->forcedDecisionsQueue.size());
                if(pCell->forcedDecisionsQueue.size() > 0) continue;
                preplan_random_cell_activity(pCell, 10, 10, 100, false, true);
                //print_scalar_vals("numPreplannedDecisions", pCell->forcedDecisionsQueue.size());
            }
        }
        
        if (kF4start <= frameNum) {
            text = "end of animations lol\nFrame " + conv_int_to_str(frameNum);
            text += " of " + conv_int_to_str(numFrames);
            draw_text(x0,y0,textWidth,textHeight,0,0,text);
        } else {
            SDL_draw_frame();
        }
    }
    SDL_RenderPresent(P_RENDERER);
}
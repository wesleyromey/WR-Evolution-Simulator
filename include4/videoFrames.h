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
        varVals["EAM_SUN"] = 100; varVals["EAM_GND"] = 0; varVals["EAM_CELLS"] = 0;
    } else if(cellType == "worm"){
        varVals["EAM_SUN"] = 0; varVals["EAM_GND"] = 100; varVals["EAM_CELLS"] = 0;
    } else if(cellType == "predator"){
        varVals["EAM_SUN"] = 0; varVals["EAM_GND"] = 0; varVals["EAM_CELLS"] = 100;
    } else {
        varVals["EAM_SUN"] = 34; varVals["EAM_GND"] = 33; varVals["EAM_CELLS"] = 33;
    }
    varVals["energy"] = initEnergy; varVals["maxEnergy"] = maxEnergy;
    varVals["maxHealth"] = maxHealth; varVals["health"] = varVals["maxHealth"] * initHealthPct / 100;
    varVals["mutationRate"] = mutationRate;
    defaultMutationChance.set_val(mutationRate);
    defaultMutationAmt.set_val(mutationRate);
    varVals["posX"] = posX; varVals["posY"] = posY;
    varVals["speedIdle"] = speedIdle; varVals["speedWalk"] = speedWalk; varVals["speedRun"] = speedRun;
    varVals["visionDist"] = visionDist;
    return varVals;
}

void gen_demo_cells_video1(int scenarioNum){
    // Shorthand functions for these scenarios
    #ifndef scenario_precode
    #define scenario_precode(numCells) { \
        deallocate_all_cells(); \
        automateEnergy = false; \
        randomly_place_new_cells(numCells, CELL_TYPE_GENERIC); \
    }
    #endif
    #ifndef scenario_postcode
    #define scenario_postcode() { \
        automateEnergy = true; \
    }
    #endif

    std::map<std::string, int> varVals;
    int tmpVar, tmpVar2;
    switch(scenarioNum){
        case 10:
        // Plant cell gets sunlight rapidly, then dies at the end of the long night
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy, &dayLenSec,
            &dayNightExponentPct, &dayNightUbPct},
            {7, 5, DAY_NIGHT_DEFAULT_MODE, 120, 0, 1, 200, 150, 50});
        varVals.clear(); varVals = gen_std_stats("plant", 2, 2, 2, 500, 3000, 500);
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
        varVals.clear(); varVals = gen_std_stats("plant", 3, 2, 3, 3000, 15000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        varVals.clear(); varVals = gen_std_stats("worm", 7, 6, 2, 2000, 10000);
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

        case 14:
        // 1g. Show a ground cell regenerating slowly. Then, show a diameter 1 cell consume its energy to replenish roughly 50% of its energy
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {7, 3, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1000});
        varVals.clear(); varVals = gen_std_stats("worm", 0, 1, 1, 500, 3000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(10, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(5, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 180, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(50, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 180, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        scenario_postcode();
        break;
        
        case 15:
        // 1h. Same as 1g except with a size 2 worm
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {7, 3, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1000});
        varVals.clear(); varVals = gen_std_stats("worm", 1, 1, 2, 600, 3000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(30, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(5, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 180, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(60, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 180, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        scenario_postcode();
        break;

        case 16:
        // 1i. Same as 1g except with a worm of diameter 2 AND a 3x3 group of ground cells will be exposed
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {7, 3, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1000});
        varVals.clear(); varVals = gen_std_stats("worm", 1, 1, 2, 600, 3000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(30, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, WALK_MODE, false, false, false);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(5, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 180, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(20, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 0, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(60, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1, 180, 0, RUN_MODE, false, false, false);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
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
        // Show a worm losing energy faster and faster despite getting identical ground energy
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
        varVals.clear(); varVals = gen_std_stats("predator", 10, 10, 6, 4000, 30000, 7);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(100, 0, 0, IDLE_MODE, true, false, false);
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
        // (3j) 30x20 simulation showing four size 2 predators and a size 4 predator swarm a size 6 plant.
        //  Each should gain identical amounts of energy when the plant dies.
        scenario_precode(6);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 0, 1});
        varVals.clear(); varVals = gen_std_stats("plant", 15, 10, 8, 40000, 40000, 16);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        #define init_predator(cellNum, varVals, posX, posY, dia, targetX, targetY, attackCooldown){ \
            varVals.clear(); varVals = gen_std_stats("predator", posX, posY, dia, 1000, 10000, dia*dia/2, 100, 1, attackCooldown); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
            pCellsHist[cellNum]->preplan_shortest_path_to_point(posX, posY, targetX, targetY, true, true, false); \
            pCellsHist[cellNum]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false); \
        }
        init_predator(1, varVals,  6,  9, 4, 16,  8, 10);
        init_predator(2, varVals,  3,  3, 2, 12, 10, 10);
        init_predator(3, varVals,  3, 17, 2, 12, 13, 10);
        init_predator(4, varVals, 27,  3, 2, 15, 13, 10);
        init_predator(5, varVals, 25, 18, 2, 18, 13, 10);
        #undef init_predator
        scenario_postcode();
        break;

        case 34:
        // (3k) 30x20 simulation where a size 6 predator is swarmed by six size 2 predators, each attacking one after the other.
        // The giant kills the first small predator, but the other predators get their hits in before the giant can recharge.
        // The giant dies.
        scenario_precode(9);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {30, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1});
        #define init_predator(cellNum, varVals, posX, posY, dia, targetX, targetY, delay, maxHealth){ \
            varVals.clear(); varVals = gen_std_stats("predator", posX, posY, dia, 1000*dia, 5000*dia, maxHealth, 100, dia*dia); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
            pCellsHist[cellNum]->force_decision(delay, 0, 0, IDLE_MODE, true, false, false); \
            pCellsHist[cellNum]->preplan_shortest_path_to_point(posX, posY, targetX, targetY, true, true, false); \
            pCellsHist[cellNum]->preplan_shortest_path_to_point(targetX, targetY, posX, posY, true, true, false); \
            pCellsHist[cellNum]->force_decision(10, 0, 0, IDLE_MODE, false, false, false); \
            pCellsHist[cellNum]->preplan_shortest_path_to_point(posX, posY, targetX, targetY, true, true, false); \
            pCellsHist[cellNum]->force_decision(10, 0, 0, IDLE_MODE, false, false, false); \
            pCellsHist[cellNum]->preplan_shortest_path_to_point(targetX, targetY, posX, posY, false, false, false); \
            pCellsHist[cellNum]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false); \
        }
        init_predator(0, varVals, 15, 10, 8, 15, 10, 1000, 56);
        init_predator(1, varVals, 15,  3, 2, 15,  6, 10,    4);
        init_predator(2, varVals, 20,  5, 2, 17,  8, 10,    4);
        init_predator(3, varVals, 22, 10, 2, 19, 10, 12,    4);
        init_predator(4, varVals, 20, 15, 2, 17, 12, 12,    4);
        init_predator(5, varVals, 15, 17, 2, 15, 14, 11,    4);
        init_predator(6, varVals, 10, 15, 2, 13, 12, 11,    4);
        init_predator(7, varVals,  8, 10, 2, 11, 10, 13,    4);
        init_predator(8, varVals, 10,  5, 2, 13,  8, 13,    4);
        #undef init_predator
        scenario_postcode();
        break;

        case 51:
        // 5b
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {6, 3, DAY_NIGHT_ALWAYS_DAY_MODE, 100, 0, 1});
        varVals.clear(); varVals = gen_std_stats("plant", 1, 1, 2, 2500, 5000);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, true);
        scenario_postcode();
        break;

        case 52:
        // 5c
        scenario_precode(1);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {6, 3, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1});
        varVals.clear(); varVals = gen_std_stats("plant", 1, 1, 2, 5000, 5000, 600);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(   5, 0, 0, IDLE_MODE, false, false, false);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, true);
        scenario_postcode();
        break;

        case 53:
        // 5d
        scenario_precode(4);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {80, 40, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1});
        #define gen_worm(cellNum, posX, posY, _runSpeed){ \
            varVals.clear(); varVals = gen_std_stats("worm", posX, posY, 9, 3000, 3000, 1, 100, 1, 10, 0, 0, 0, 0, _runSpeed, 0); \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
            pCellsHist[cellNum]->force_decision(1000, 0, 0, RUN_MODE, false, false, false); \
        }
        for(tmpVar = 0; tmpVar < pCellsHist.size(); tmpVar++){
            gen_worm(tmpVar, 10, 10*tmpVar+5, 3*tmpVar);
        }
        #undef gen_worm
        scenario_postcode();
        break;

        case 54:
        // 5e
        scenario_precode(2);
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {15, 10, DAY_NIGHT_ALWAYS_DAY_MODE, 0, 0, 1});
        varVals.clear(); varVals = gen_std_stats("plant", 4, 4, 6, 10000, 10000, 200);
        pCellsHist[0]->set_int_stats(varVals, 0);
        pCellsHist[0]->force_decision(1000, 0, 0, IDLE_MODE, false, false, false);
        varVals.clear(); varVals = gen_std_stats("predator", 9, 9, 2, 5000, 5000, 1, 100, 50, 0);
        pCellsHist[1]->set_int_stats(varVals, 0);
        pCellsHist[1]->force_decision(3, 225, 0, RUN_MODE, true, false, false);
        pCellsHist[1]->force_decision(1000, 0, 0, IDLE_MODE, true, false, false);
        #undef gen_cell
        scenario_postcode();
        break;

        case 57:
        //stats["rngAi_pctChanceIdle"] = {           33,     0,   100,         0,      0};
        //stats["rngAi_pctChanceWalk"] = {           33,     0,   100,         0,      0};
        //stats["rngAi_pctChanceToChangeDir"] = {     5,     0,   100,         0,      0};
        //stats["rngAi_pctChanceToChangeSpeed"] = {   5,     0,   100,         0,      0};

        // 5h
        scenario_precode(3);
        enableAutomaticCloning = false;
        enableAutomaticAttack = false;
        //pctChanceIdle = 10; pctChanceWalk = 45; pctChanceRun = 45; // 10, 45, 45
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {40, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 20, 200});
        #define gen_cell(cellNum, cellType, posX, posY, dia, initEnergy, maxEnergy){ \
            varVals.clear(); varVals = gen_std_stats(cellType, posX, posY, dia, initEnergy, maxEnergy, dia, 100, dia, 10, 0, 0, 0, 1, 2); \
            varVals["rngAi_pctChanceIdle"] = 10; varVals["rngAi_pctChanceIdle"] = 45; \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
        }
        gen_cell(0, "plant", 5, 5, 6, 10000, 10000);
        gen_cell(1, "worm", 35, 15, 2, 10000, 10000);
        gen_cell(2, "predator", 20, 10, 2, 10000, 10000);
        #undef gen_cell
        scenario_postcode();
        break;

        case 58:
        // 5i
        scenario_precode(21);
        enableAutomaticCloning = false;
        enableAutomaticAttack = true;
        //pctChanceIdle = 10; pctChanceWalk = 45; pctChanceRun = 45;
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {40, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 20, 200});
        #define gen_cell(cellNum, cellType, posX, posY, dia, initEnergy, maxEnergy, visionDist, attack){ \
            varVals.clear(); varVals = gen_std_stats(cellType, posX, posY, dia, initEnergy, maxEnergy, dia, 100, attack, 10, 0, 0, 0, 1, 2, visionDist); \
            varVals["rngAi_pctChanceIdle"] = 10; varVals["rngAi_pctChanceWalk"] = 30; \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
        }
        gen_cell(0, "predator", ubX.val/2, ubY.val/2, 2, 1000, 10000, 5, 1);
        for(tmpVar = 1; tmpVar < pCellsHist.size();){
            gen_cell(tmpVar++, "plant", rand() % (ubX.val-2) + 1, rand() % (ubY.val-2) + 1, 2, 1000, 10000, 0, 0);
            gen_cell(tmpVar++,  "worm", rand() % (ubX.val-2) + 1, rand() % (ubY.val-2) + 1, 2, 1000, 10000, 0, 0);
        }
        #undef gen_cell
        scenario_postcode();
        break;

        case 59:
        // 5j
        scenario_precode(7);
        enableAutomaticCloning = false;
        enableAutomaticAttack = true;
        //pctChanceIdle = 10; pctChanceWalk = 30; pctChanceRun = 60; // 10, 45, 45
        set_sim_params({&ubX, &ubY, &dayNightMode, &maxSunEnergyPerSec, &gndEnergyPerIncrease, &maxGndEnergy},
            {40, 20, DAY_NIGHT_ALWAYS_DAY_MODE, 50, 20, 200});
        #define gen_cell(cellNum, cellType, posX, posY, dia, initEnergy, maxEnergy, visionDist, attack, maxHealth){ \
            varVals.clear(); varVals = gen_std_stats(cellType, posX, posY, dia, initEnergy, maxEnergy, maxHealth, 100, attack, 10, 0, 0, 0, 1, 2, visionDist); \
            varVals["rngAi_pctChanceIdle"] = 90; varVals["rngAi_pctChanceWalk"] = 5; \
            varVals["rngAi_pctChanceToChangeDir"] = 100; varVals["rngAi_pctChanceToChangeSpeed"] = 100; \
            pCellsHist[cellNum]->set_int_stats(varVals, 0); \
        }
        #define force_leftward_movement(cellNum, numFrames, _speedMode) pCellsHist[cellNum]->force_decision(numFrames, 180, 0, _speedMode, false, false, false)
        //gen_cell(0, "worm", 20, 10, 2, 10000, 10000, 0, 0);
        gen_cell(0, "predator", 20, 10, 2, 1000, 10000, 6, 1, 2);
        pCellsHist[0]->force_decision(10, 0, 0, IDLE_MODE, false, false, false);
        gen_cell(1,  "worm", 17, 10, 2, 500, 5000, 0, 0, 2000);
        pCellsHist[1]->force_decision(10, 0, 0, IDLE_MODE, false, false, false);
        gen_cell(2, "worm", 23, 10, 2, 500, 5000, 0, 0, 2);
        pCellsHist[2]->force_decision(100, 0, 0, IDLE_MODE, false, false, false);
        gen_cell(3, "plant", 20,  7, 2, 500, 5000, 0, 0, 2);
        gen_cell(4, "predator", 20, 13, 2, 500, 5000, 0, 0, 2);
        pCellsHist[4]->force_decision(100, 0, 0, IDLE_MODE, false, false, false);
        gen_cell(5, "worm", 35, 10, 2, 500, 5000, 0, 0, 20);
        force_leftward_movement(5, 100, IDLE_MODE);
        force_leftward_movement(5,  50, WALK_MODE);
        force_leftward_movement(5,  10, RUN_MODE);
        force_leftward_movement(5,  10, WALK_MODE);
        force_leftward_movement(5,  50, WALK_MODE); //
        force_leftward_movement(5,  10, WALK_MODE);
        force_leftward_movement(5,  10, RUN_MODE);
        force_leftward_movement(5,  50, WALK_MODE); //
        force_leftward_movement(5,   5, RUN_MODE);
        force_leftward_movement(5,   5, WALK_MODE);
        force_leftward_movement(5, 300, WALK_MODE);
        gen_cell(6, "worm", 35, 16, 2, 2000, 20000, 0, 0, 20);
        force_leftward_movement(6, 100, IDLE_MODE);
        force_leftward_movement(6,  50, WALK_MODE);
        force_leftward_movement(6,  10, WALK_MODE);
        force_leftward_movement(6,  10, RUN_MODE);
        force_leftward_movement(6,  50, WALK_MODE); //
        force_leftward_movement(6,  10, RUN_MODE);
        force_leftward_movement(6,  10, WALK_MODE);
        force_leftward_movement(6,  50, WALK_MODE); //
        force_leftward_movement(6,   5, WALK_MODE);
        force_leftward_movement(6,   5, RUN_MODE);
        force_leftward_movement(6, 300, WALK_MODE);
        //gen_cell(6,  "worm", 1, 1, 2, 500, 5000, 0, 0);
        //pCellsHist[6]->force_decision(1000, 45, 0, RUN_MODE, false, false, false);
        //gen_cell(7,  "worm", 18, 24, 6, 1500, 15000, 0, 0);
        //pCellsHist[7]->force_decision(1000, 0, 0, WALK_MODE, false, false, false);

        #undef gen_cell
        #undef force_leftward_movement
        scenario_postcode();
        break;
        





        default:
        cout << "NOTE: Scenario " << scenarioNum << " was supposed to be played, but that scenario is not available\n";
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
    static const int kF1c = kF1start, kF1d = kF1c + 250, kF1e = kF1d + 70; // Intro
    static const int kF1f = kF1e + 55, kF1g = kF1f + 70, kF1h = kF1g + 200;
    static const int kF1i = kF1h + 200, kF2start = kF1i + 200;
    static const int kF2b = kF2start, kF2d = kF2b + 40, kF2e = kF2d + 60; // Energy, health, age
    static const int kF3start = kF2e + 300;
    static const int kF3d = kF3start, kF3g = kF3d + 40, kF3i = kF3g + 180; // Size
    static const int kF3j = kF3i + 1000, kF3k = kF3j + 50, kF4start = kF3k + 100;
    static const int kF5start = kF4start;
    static const int kF5b = kF5start, kF5c = kF5b + 20, kF5d = kF5c + 100;
    static const int kF5e = kF5d + 70, kF5h = kF5e + 20;
    static const int kF5i = kF5h + 500, kF5j = kF5i + 1000;
    static const int kF6start = kF5j + 1000;
    frameNum %= numFrames;
    int startFrame = kF5j - 1;
    switch(frameNum){
        case kF0:
        deallocate_all_cells();
        set_vals(&doCellAi, true, &automateEnergy, true, &enableAutomaticAttack, false,
            &enableAutomaticCloning, false, &enableAutomaticSelfDestruct, false);
        #if !defined(DO_VIDEO_TEXT) && defined(DO_VIDEO_FRAMES)
        frameNum = startFrame;
        #endif
        draw_text(x0,y0,textWidth,textHeight,0,0,"testing text");
        break;
        case kF0+1:
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"I will talk\nabout  them\n   later");
        break;
        case kF0+2:
        draw_text(x0,y0,3*textWidth,3*textHeight,0,2,"Pretend\n this  \nis true");
        break;
        case kF0+3:
        draw_text(x0,y0,3*textWidth,3*textHeight,0,4,"If you look closely,\nthey each attack the\ngiant cell at slightly\ndifferent times");
        break;
        case kF0+4:
        case kF0+5:
        case kF0+6:
        case kF0+7:
        case kF0+8:
        case kF0+9:
        #ifdef DO_VIDEO_FRAMES
        frameNum = startFrame;
        #else
        frameNum = -1;
        #endif
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
        case kF1g:
        gen_demo_cells_video1(14);
        break;
        case kF1h:
        gen_demo_cells_video1(15);
        break;
        case kF1i:
        gen_demo_cells_video1(16);
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
        gen_demo_cells_video1(33);
        break;
        case kF3k:
        gen_demo_cells_video1(34);
        break;

        case kF5b:
        gen_demo_cells_video1(51);
        break;
        case kF5c:
        gen_demo_cells_video1(52);
        break;
        case kF5d:
        gen_demo_cells_video1(53);
        break;
        case kF5e:
        gen_demo_cells_video1(54);
        break;
        case kF5h:
        gen_demo_cells_video1(57);
        break;
        case kF5i:
        gen_demo_cells_video1(58);
        break;
        case kF5j:
        gen_demo_cells_video1(59);
        break;
        
        default:
        if(kF1g <= frameNum && frameNum < kF1h){
            if(frameNum == kF1g + 1) init_sim_gnd_energy(0);
            else if(frameNum % 5 == 0) simGndEnergy[1][3] += maxGndEnergy.val / 10;
        } else if(kF1h <= frameNum && frameNum < kF1i){
            if(frameNum == kF1h + 1) init_sim_gnd_energy(0);
            else if(frameNum % 5 == 0) simGndEnergy[1][4] += maxGndEnergy.val / 10;
        } else if(kF1i <= frameNum && frameNum < kF2start){
            if(frameNum == kF1i + 1) init_sim_gnd_energy(0);
            else if(frameNum % 5 == 0){
                for(int i = 0; i <= 2; i++){
                    for(int j = 3; j <= 5; j++){
                        simGndEnergy[i][j] += maxGndEnergy.val / 10;
                        simGndEnergy[i][j] = min_int(simGndEnergy[i][j], maxGndEnergy.val);
                    }
                }
            }
        } else if(kF2b <= frameNum && frameNum < kF2d){
            pCellsHist[0]->health--;
            pCellsHist[0]->energy = pCellsHist[0]->stats["maxEnergy"][0] / 9;
            if(pCellsHist[1]->energy < pCellsHist[1]->stats["maxEnergy"][0] / 10){
                pCellsHist[1]->energy += pCellsHist[1]->stats["maxEnergy"][0] / 100;
            } else {
                pCellsHist[1]->energy += pCellsHist[1]->stats["maxEnergy"][0] / 25;
            }
        } else if(kF2e <= frameNum && frameNum < kF3start){
            pCellsHist[0]->age += 49;
        }// else if(kF3i < frameNum && frameNum < kF3j){
            //for(auto pCell : pActives){
                //cout << ".";
                //if(pCell->isAlive == false) continue;
                //if(pCell->age == 0) pCell->clear_forced_decisions();
                //if(pCell->forcedDecisionsQueue.size() > 0) continue;
                //pCell->preplan_random_cell_activity(10, 10, 100, false, true);
            //}
        //}
        
        if (kF6start <= frameNum) {
            text = "end of animations lol\nFrame " + conv_int_to_str(frameNum);
            text += " of " + conv_int_to_str(numFrames);
            draw_text(x0,y0,textWidth,textHeight,0,0,text);
        } else {
            SDL_draw_frame();
        }
    }
    SDL_RenderPresent(P_RENDERER);
}
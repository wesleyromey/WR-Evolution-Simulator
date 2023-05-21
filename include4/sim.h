#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

// This file determines what objects and cells are in the simulation initially
// This file contains global simulation settings
// This file enforces rules such as teleportation (for portals and map wrap-arounds)


// Create an empty vector of pointers to Cells and DeadCells as global variables
std::vector<Cell*> pCellsHist; // A history of all cells in the order they were created
std::vector<Cell*> pAlives; // All cells that are currently alive
std::vector<DeadCell*> pDeads; // All cells that are currently dead


// Render the background, cell positions, etc using SDL
void SDL_draw_frame(){
    draw_texture(P_BKGND_TEX, 0, 0, WINDOW_HEIGHT, WINDOW_WIDTH);
    for(auto pCell : pAlives) pCell->draw_cell();
    for(auto pCell : pDeads) pCell->draw_cell();
    SDL_RenderPresent(P_RENDERER);
}

// NOTE: the input MUST be a pointer to memory which has been allocated
//  for type Cell (using 'new').
//  Recommended to just directly write 'delete ptr' in-line
void deallocate_cell_memory(Cell* pCell) {
    delete pCell;
}

void init_sim_gnd_energy(){
    // We are dealing with global variables
    for(int x = 0; x < UB_X; x++){
        for(int y = 0; y < UB_Y; y++){
            simGndEnergy[x][y] = INIT_GND_ENERGY;
        }
    }
}

void increase_sim_gnd_energy(int increaseAmt){
    for(int x = 0; x < UB_X; x++){
        for(int y = 0; y < UB_Y; y++){
            simGndEnergy[x][y] += increaseAmt;
            if(simGndEnergy[x][y] > MAX_GND_ENERGY){
                simGndEnergy[x][y] = MAX_GND_ENERGY;
            }
        }
    }
}

// If the target is set to a positive time, then set the time.
//  Otherwise, increment the time by 1.
void update_dayNightCycleTime(int target = -1){
    if (target < 0) dayNightCycleTime = ++dayNightCycleTime % DAY_LEN_SEC;
    else dayNightCycleTime = target % DAY_LEN_SEC;
}

// Update the amount of energy each cell gets from the sun per second
const static int DAY_NIGHT_ALWAYS_DAY_MODE = 0;
const static int DAY_NIGHT_BINARY_MODE = 1;
const static int DAY_NIGHT_DEFAULT_MODE = 2;
const static int DAY_NIGHT_MODE = DAY_NIGHT_ALWAYS_DAY_MODE;
const static float DAY_NIGHT_EXPONENT = 2.0; // 0 <= EXPONENT < infinity
const static float DAY_NIGHT_LB = 0.0, DAY_NIGHT_UB = 0.5;
void do_day_night_cycle(){
    // Stick with a simple polynomial equation to approximate a sine wave during the day phase
    //  (the day occurs between times LB and UB) and set to 0 during the night phase
    float timeNormalized = (float)dayNightCycleTime / DAY_LEN_SEC - (DAY_NIGHT_LB + DAY_NIGHT_UB) / 2;
    float energyFrac;
    switch(DAY_NIGHT_MODE){
        case DAY_NIGHT_ALWAYS_DAY_MODE:
        energyFromSunPerSec = MAX_SUN_ENERGY_PER_SEC;
        return;

        case DAY_NIGHT_DEFAULT_MODE:
        //  Arbitrarily large values mean the sun gives maximum energy during the day (but none during the night)
        //  Larger values mean the sun gives more energy for longer throughout the day
        //  Smaller values mean the sun is lower throughout the day
        //  The smallest values mean the sun remains essentially at the horizon during the day
        //      except possibly for the instant it is mid-day
        //  EXPONENT == 1.75 is a good approximation of a sine wave
        energyFrac = abs_float(2 * timeNormalized / (DAY_NIGHT_UB - DAY_NIGHT_LB));
        energyFrac = 1.0 - pow(energyFrac, DAY_NIGHT_EXPONENT);
        energyFrac = saturate_float(energyFrac, 0, 1);
        energyFromSunPerSec = energyFrac * MAX_SUN_ENERGY_PER_SEC; // Round down
        return;

        case DAY_NIGHT_BINARY_MODE:
        int lb = DAY_NIGHT_LB * DAY_LEN_SEC, ub = DAY_NIGHT_UB * DAY_LEN_SEC;
        energyFromSunPerSec = (lb <= dayNightCycleTime && dayNightCycleTime <= ub) ? MAX_SUN_ENERGY_PER_SEC : 0;
        return;
    }
}

// MUST run this before completing frames if the simulation is to run properly
void init_sim_global_vals(){
    do_day_night_cycle();
    init_sim_gnd_energy();
}

void gen_cell(Cell* pParent = NULL, bool randomizeCloningDir = false, int cloningDir = -1){
    if(pParent == NULL){
        Cell *pCell = new Cell;
        pCell->gen_stats_random(pCellsHist.size(), pCell);
        pCell->randomize_pos(0, UB_X-1, 0, UB_Y-1);
        pCellsHist.push_back(pCell);
        pAlives.push_back(pCell);
    } else {
        Cell* pCell = pParent->clone_self(pCellsHist.size(), cloningDir, randomizeCloningDir); // A perfect clone of pParent
        pCell->mutate_stats();
        pCellsHist.push_back(pCell);
        pAlives.push_back(pCell);
    }
}

void kill_cell(Cell* pAlive, int i_pAlive) {
    DeadCell* pDead = new DeadCell;
    pDead->pSelf = pDead;
    pDead->pOldSelf = pAlive;
    pDead->decayPeriod = 10;
    pDead->decayRate = 10;
    pDead->dia = pAlive->dia;
    pDead->energy = pAlive->energy + pAlive->energyCostToClone;
    pDead->timeSinceDead = 1;
    pDead->posX = pAlive->posX;
    pDead->posY = pAlive->posY;
    // Remove pAlive from the list of alive cells
    pAlives.erase(pAlives.begin() + i_pAlive);
    // Add pDead to the list of dead cells
    pDeads.push_back(pDead);
}

void gen_dead_cell(Cell* pAlive = NULL, int i_pAlive = -1) {
    // pAlive is the cell to kill, if applicable
    if(pAlive == NULL){
        DeadCell* pDead = new DeadCell;
        pDead->gen_stats_random(pDead);
        pDead->randomize_pos(0, UB_X-1, 0, UB_Y-1);
        pDeads.push_back(pDead);
    } else {
        kill_cell(pAlive, i_pAlive);
    }
}

void randomly_place_new_cells(int numCells){
    // Spreads random cells equally across the simulation space
    for(int i = 0; i < numCells; i++) {
        gen_cell(); // This function allocates memory for pCell
        //  and appends a pointer to it for both pCellsHist and pAlives
    }
    std::cout << "Placed " << numCells << " new cells\n";
}

void print_cell_coords(std::vector<Cell*> pCells){
    std::cout << "pos: ";
    for(auto pCell : pCells) pCell->print_pos("", false);
    std::cout << std::endl;
}
void print_cell_forces(std::vector<Cell*> pCells){
    std::cout << "forces: ";
    for(auto pCell : pCells) pCell->print_forces("", false);
    std::cout << std::endl;
}

// This function does the frame while preserving the cells' previous decisions
//  i.e. cells are forced to keep their inputs constant (e.g. speed, direction, etc.)
void do_frame_static(int frameNum){
    if(!simIsRunning) return;

    // Cells move to their target positions based on their speed
    for(auto pCell : pAlives) pCell->update_target_pos();

    // Rendering and User Interactions
    SDL_draw_frame();
    //cout << "frameNum: " << frameNum << "a" << endl;
    SDL_event_handler();

    // Cells apply all their non-movement decisions this frame
    //  such as attacking and cloning. Deaths are dealt with after
    for(auto pCell : pAlives){
        pCell->apply_non_movement_decisions(pAlives, pCellsHist);
    }

    // Cells move to new positions if enough force is applied
    for(auto pCell : pAlives) pCell->update_forces(pAlives);
    //print_cell_forces(pAlives);
    for(auto pCell : pAlives) pCell->apply_forces();

    // Increase cell energy based on EAM
    do_day_night_cycle();
    update_dayNightCycleTime();
    for(auto pCell : pAlives) pCell->do_energy_accumulation();
    for(auto pCell : pDeads) pCell->do_energy_decay(pAlives);

    // Consume energy each 'tick'
    for(auto pCell : pAlives) pCell->consume_energy_per_frame();

    // Kill all cells which meet at least one of the conditions for dying
    for(int i = pAlives.size() - 1; i >= 0; i--) {
        if(pAlives[i]->calc_if_cell_is_dead()) kill_cell(pAlives[i], i);
    }

    // Deal with dead cells
    for(int i = pDeads.size() - 1; i >= 0; i--){
        pDeads[i]->remove_this_dead_cell_if_depleted(pDeads, i);
    }

    // Every certain number of frames, the energy levels within the ground should be increased for all ground pixels
    if(frameNum % FRAMES_BETWEEN_GND_ENERGY_ACCUMULATION == 0){
        increase_sim_gnd_energy(GND_ENERGY_PER_INCREASE);
    }

    // Rendering and User Interactions
    SDL_draw_frame();
    //if(frameNum % 100 == 0) cout << "frameNum: " << frameNum << "b" << endl;
    SDL_event_handler();
    
    return;
}

void do_frame(int frameNum){
    if(!simIsRunning) return;

    // The cells each decide what to do (e.g. speed, direction, doAttack, etc.)
    //  (e.g. update their internal state).
    for(auto pCell : pAlives) pCell->decide_next_frame();

    // Static portion of the frame
    do_frame_static(frameNum);
}

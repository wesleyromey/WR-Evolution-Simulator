#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif
#define SIM_H

// This file determines what objects and cells are in the simulation initially
// This file contains global simulation settings
// This file enforces rules such as teleportation (for portals and map wrap-arounds)


// Create an empty vector of pointers to Cells and DeadCells as global variables
std::vector<Cell*> pCellsHist; // A history of all cells in the order they were created
std::vector<Cell*> pAlives; // All cells that are currently alive
std::map<std::pair<int,int>, std::vector<Cell*>> pAlivesRegions;
//  pAlives separated by region
std::vector<DeadCell*> pDeads; // All cells that are currently dead
std::map<std::pair<int,int>, std::vector<DeadCell*>> pDeadsRegions;
//  pDeads separated by region


// Render the background, cell positions, etc using SDL
void SDL_draw_frame(){
    SDL_RenderClear(P_RENDERER);
    #ifdef DEBUG_FRAMES
    draw_bkgnd(100);
    int ds = DRAW_SCALE_FACTOR;
    float symbolWidth = ds*15/16;
    int symbolHeight = ds*32/16;
    int dsSymVert = symbolHeight+2;
    draw_texture(p_0_Symbol, 1*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_1_Symbol, 2*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_2_Symbol, 3*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_3_Symbol, 4*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_4_Symbol, 5*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_5_Symbol, 6*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_6_Symbol, 7*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_7_Symbol, 8*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_8_Symbol, 9*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_9_Symbol, 10*ds, ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_A_Symbol, 1*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_B_Symbol, 2*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_C_Symbol, 3*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_D_Symbol, 4*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_E_Symbol, 5*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_F_Symbol, 6*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_G_Symbol, 7*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_H_Symbol, 8*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_I_Symbol, 9*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_J_Symbol, 10*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_K_Symbol, 11*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_L_Symbol, 12*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_M_Symbol, 13*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_N_Symbol, 14*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_O_Symbol, 15*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_P_Symbol, 16*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_Q_Symbol, 17*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_R_Symbol, 18*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_S_Symbol, 19*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_T_Symbol, 20*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_U_Symbol, 21*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_V_Symbol, 22*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_W_Symbol, 23*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_X_Symbol, 24*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_Y_Symbol, 25*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(p_Z_Symbol, 26*ds, dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(pMinusSymbol, 1*ds, 2*dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(pSpaceSymbol, 2*ds, 2*dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);
    draw_texture(pDashSymbol,  3*ds, 2*dsSymVert + ds, (int)symbolWidth, (int)symbolHeight);

    draw_texture(p_D_Symbol, ds, 4*dsSymVert + ds, (int)symbolWidth*5, (int)symbolHeight*5);

    draw_user_interface();
    #else
    //SDL_RenderClear(P_RENDERER);
    draw_bkgnd(energyFromSunPerSec);
    draw_gnd();
    for(auto pCell : pAlives) pCell->draw_cell();
    for(auto pCell : pDeads) pCell->draw_cell();
    draw_user_interface();
    #endif
    if(simState != SIM_STATE_SKIP_FRAMES) enforce_frame_rate(frameStart, FRAME_DELAY);
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
    dayNightCycleTime = 0;
    do_day_night_cycle();
    init_sim_gnd_energy();
}

void gen_cell(Cell* pParent = NULL, bool randomizeCloningDir = false, int cloningDir = -1){
    if(pParent == NULL){
        Cell *pCell = new Cell;
        pCell->gen_stats_random(pCellsHist.size(), pAlivesRegions, pCell);
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

// Deallocate all cell pointers and then remove the deallocated pointers from
//  the vectors, maps, etc. that they are stored in
void deallocate_all_cells(){
    for(auto pCell : pCellsHist) delete pCell;
    for(auto pCell : pDeads) delete pCell;
    pCellsHist.clear(); pDeads.clear(); pAlives.clear();
    pAlivesRegions.clear(); pDeadsRegions.clear();
}
// Initialize the simulation
void init_sim(int initNumCells){
    assert(simState == SIM_STATE_INIT);
    init_sim_global_vals();
    randomly_place_new_cells(initNumCells);
    simState = SIM_STATE_STEP_FRAMES;
}
// Restart the simulation
void restart_sim(int initNumCells){
    assert(simState == SIM_STATE_RESTART);
    // Deallocate and remove all cells from the simulation
    cout << "  Simulation is about to restart" << endl << "  ";
    deallocate_all_cells();
    // Re-initialize Simulation
    init_sim_global_vals();
    randomly_place_new_cells(initNumCells);
    cout << "  Simulation is restarted!" << endl;
    simState = SIM_STATE_STEP_FRAMES;
}
// Deallocate memory when an exception occurs (ideally) or when the program terminates
int exit_sim(){
    assert(simState == SIM_STATE_QUIT);
    deallocate_all_cells();
    wait_for_user_to_exit_SDL();
    exit_SDL();
    std::cout << "sim is done and finished!\n";
    return 0;
}


void assign_cells_to_correct_regions(){
    // Clear pAlivesRegions and pDeadsRegions
    for(int _x = 0; _x < CELL_REGION_NUM_X; _x++){
        for(int _y = 0; _y < CELL_REGION_NUM_Y; _y++){
            pAlivesRegions[{_x, _y}] = {};
            pDeadsRegions[{_x, _y}] = {};
        }
    }

    // Assign each dead and alive cell to the correct region
    for(auto pCell : pAlives){
        pAlivesRegions[pCell->xyRegion].push_back(pCell);
    }
    for(auto pCell : pDeads){
        pDeadsRegions[pCell->xyRegion].push_back(pCell);
    }
}

// Repeat this function each frame
void do_frame(int frameNum, bool doCellDecisions = true){

    // Initialize, restart, or quit the simulation if needed
    if(simState == SIM_STATE_QUIT) return;
    if(simState == SIM_STATE_INIT) init_sim(initNumCells);
    if(simState == SIM_STATE_RESTART) restart_sim(initNumCells);

    if(doCellDecisions){
        // The cells each decide what to do (e.g. speed, direction,
        //  doAttack, etc.) by updating their internal state
        assign_cells_to_correct_regions();
        for(auto pCell : pAlives) pCell->decide_next_frame(pAlivesRegions);
    }
    
    // Cells move to their target positions based on their speed
    assign_cells_to_correct_regions();
    for(auto pCell : pAlives) pCell->update_target_pos();

    /*
    // Rendering and User Interactions
    SDL_draw_frame();
    SDL_event_handler();
    */

    // Cells apply all their non-movement decisions this frame
    //  such as attacking and cloning. Deaths are dealt with after
    assign_cells_to_correct_regions();
    for(auto pCell : pAlives){
        pCell->apply_non_movement_decisions(pAlives, pCellsHist, pAlivesRegions);
    }

    // Cells move to new positions if enough force is applied
    assign_cells_to_correct_regions();
    for(auto pCell : pAlives) pCell->update_forces(pAlives, pAlivesRegions);
    for(auto pCell : pAlives) pCell->apply_forces();

    // Increase cell energy based on EAM
    do_day_night_cycle();
    update_dayNightCycleTime();
    assign_cells_to_correct_regions();
    for(auto pCell : pAlives) pCell->do_energy_transfer(pAlives, pAlivesRegions);
    for(auto pCell : pDeads) pCell->do_energy_decay(pAlives, pAlivesRegions);

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
    SDL_event_handler();
}

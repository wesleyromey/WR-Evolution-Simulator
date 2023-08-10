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
std::vector<Cell*> pActives; // All the dead and alive cells in the simulation
std::map<std::pair<int,int>, std::vector<Cell*>> pActivesRegions; // pActives separated by region



void do_video1();


int count_all_alive_cells(std::vector<Cell*> pActives){
    int numAliveCells = 0;
    for(auto pCell : pActives) numAliveCells += pCell->isAlive;
    return numAliveCells;
}  


// Render the background, cell positions, etc using SDL
void SDL_draw_frame(){
    SDL_RenderClear(P_RENDERER);
    #ifdef DEBUG_FRAMES
    draw_bkgnd(100);
    int ds = drawScaleFactor;
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

    draw_user_interface(count_all_alive_cells(pActives));
    #else
    //SDL_RenderClear(P_RENDERER);
    draw_bkgnd(energyFromSunPerSec);
    draw_gnd();
    for(auto pCell : pActives) pCell->draw_cell();
    draw_cell_mask();
    draw_user_interface(count_all_alive_cells(pActives));
    #endif
    enforce_frame_rate(frameStart, FRAME_DELAY);
    SDL_RenderPresent(P_RENDERER);
}

// NOTE: the input MUST be a pointer to memory which has been allocated
//  for type Cell (using 'new').
//  Recommended to just directly write 'delete ptr' in-line
void deallocate_cell_memory(Cell* pCell) {
    delete pCell;
}

void init_sim_gnd_energy(int initGndEnergy){
    if(initGndEnergy < 0) initGndEnergy = maxGndEnergy.val / 2;
    simGndEnergy.clear();
    std::vector<int> simGndEnergyRow(ubX.val);
    for(int posX = 0; posX < ubX.val; posX++) simGndEnergyRow[posX] = initGndEnergy;
    for(int posY = 0; posY < ubY.val; posY++) simGndEnergy.push_back(simGndEnergyRow);
}

void increase_sim_gnd_energy(int increaseAmt){
    for(int posY = 0; posY < simGndEnergy.size(); posY++){
        for(int posX = 0; posX < simGndEnergy[posY].size(); posX++){
            simGndEnergy[posY][posX] += increaseAmt;
            simGndEnergy[posY][posX] = min_int(simGndEnergy[posY][posX], maxGndEnergy.val);
        }
    }
}

// If the target is set to a positive time, then set the time.
//  Otherwise, increment the time by 1.
void update_dayNightCycleTime(int target = -1){
    if(target < 0) dayNightCycleTime = ++dayNightCycleTime % dayLenSec.val;
    else dayNightCycleTime = target % dayLenSec.val;
}

// Update the amount of energy each cell gets from the sun per second
void do_day_night_cycle(){
    // Stick with a simple polynomial equation to approximate a sine wave during the day phase
    //  (the day occurs between times LB and UB) and set to 0 during the night phase
    float timeNormalized = (float)dayNightCycleTime / dayLenSec.val - (float)(dayNightLbPct.val + dayNightUbPct.val) / 200;
    float energyFrac;
    int lb, ub;
    switch(dayNightMode.val){
        case DAY_NIGHT_ALWAYS_DAY_MODE:
        energyFromSunPerSec = maxSunEnergyPerSec.val;
        return;

        case DAY_NIGHT_BINARY_MODE:
        lb = (float)dayNightLbPct.val / 100 * dayLenSec.val;
        ub = (float)dayNightUbPct.val / 100 * dayLenSec.val;
        energyFromSunPerSec = (lb <= dayNightCycleTime && dayNightCycleTime <= ub) ? maxSunEnergyPerSec.val : 0;
        return;

        case DAY_NIGHT_DEFAULT_MODE:
        //  Arbitrarily large values mean the sun gives maximum energy during the day (but none during the night)
        //  Larger values mean the sun gives more energy for longer throughout the day
        //  Smaller values mean the sun is lower throughout the day
        //  The smallest values mean the sun remains essentially at the horizon during the day
        //      except possibly for the instant it is mid-day
        //  EXPONENT == 1.75 is a good approximation of a sine wave
        energyFrac = abs_float(200. * timeNormalized / (float)(dayNightUbPct.val - dayNightLbPct.val));
        energyFrac = 1.0 - pow(energyFrac, (float)dayNightExponentPct.val / 100);
        energyFrac = saturate_float(energyFrac, 0, 1);
        energyFromSunPerSec = energyFrac * maxSunEnergyPerSec.val; // Round down
        return;
    }
}

// MUST run this before completing frames if the simulation is to run properly
void init_sim_global_vals(){
    dayNightCycleTime = 0;
    update_global_params();
    do_day_night_cycle();
    init_sim_gnd_energy(maxGndEnergy.val / 2);
}

// This function allocates memory for a new cell and saves a pointer to it in both pCellsHist and pActives
void gen_cell(int cellType, Cell* pParent = NULL, bool randomizeCloningDir = false, int cloningDir = -1){
    if(cellType == CELL_TYPE_PLANT_WORM_PREDATOR_OR_MUTANT) cellType = availableCellTypes(rng);
    //cout << "cellType: " << cellType << endl;
    if(pParent == NULL){
        Cell *pCell = new Cell();
        pCell->define_self(pCellsHist.size(), pCell, NULL);
        pCellsHist.push_back(pCell);
        pActives.push_back(pCell);
        pCell->gen_stats_random(cellType, pActivesRegions, pCellsHist);
        pCell->randomize_pos(0, ubX.val-1, 0, ubY.val-1);
    } else {
        Cell* pCell = pParent->clone_self(pCellsHist.size(), pActivesRegions, pCellsHist, pActives, cloningDir, randomizeCloningDir);
    }
}

/*
void kill_cell(Cell* pAlive) {
    pAlive->kill_self();
}
*/

// Generates a dead cell from nothing
void gen_dead_cell() {
    gen_cell(CELL_TYPE_PLANT);
    pActives[pActives.size()-1]->kill_self();
}

// cellType == -1 means select a random cell type
void randomly_place_new_cells(int numCells, int cellType = CELL_TYPE_PLANT_WORM_PREDATOR_OR_MUTANT){
    // Spreads random cells equally across the simulation space
    for(int i = 0; i < numCells; i++) {
        gen_cell(cellType);
    }
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
    pCellsHist.clear(); pActives.clear();
    pActivesRegions.clear();
}
// Initialize the simulation
void init_sim(){
    assert(simState == SIM_STATE_INIT);
    update_global_params();
    init_sim_global_vals();
    randomly_place_new_cells(initNumCells.val);
    simState = SIM_STATE_STEP_FRAMES;
    frameStart = SDL_GetTicks();
    frameNum = 0;
}
// Restart the simulation
void restart_sim(){
    assert(simState == SIM_STATE_RESTART);
    // Deallocate and remove all cells from the simulation
    deallocate_all_cells();
    simState = SIM_STATE_MAIN_MENU; //SIM_STATE_STEP_FRAMES;
}
// Deallocate memory when an exception occurs (ideally) or when the program terminates
int exit_sim(){
    assert(simState == SIM_STATE_QUIT);
    deallocate_all_cells();
    wait_for_user_to_exit_SDL();
    exit_SDL();
    return 0;
}


void assign_cells_to_correct_regions(){
    // Clear pActivesRegions
    for(int _x = 0; _x < cellRegionNumUbX; _x++){
        for(int _y = 0; _y < cellRegionNumUbY; _y++){
            pActivesRegions[{_x, _y}] = {};
        }
    }
    for(auto pCell : pActives) pActivesRegions[pCell->xyRegion].push_back(pCell);
}

// Display the percentiles of certain cell stats
//  statRestrictions is a map of values which each counted cell must contain
void disp_cell_statistics(std::map<std::string, int> statRestrictions, std::string label){
    std::map<std::string, float> avgStats;
    int numRelevantCells = 0;
    for(auto pCell : pActives){
        if(statRestrictions.count("cellType")){
            if(statRestrictions["cellType"] == CELL_TYPE_PLANT && pCell->stats["EAM_SUN"][0] != 100) continue;
            if(statRestrictions["cellType"] == CELL_TYPE_WORM  && pCell->stats["EAM_GND"][0] != 100) continue;
            if(statRestrictions["cellType"] == CELL_TYPE_PREDATOR && pCell->stats["EAM_CELLS"][0] != 100) continue;
            if(statRestrictions["cellType"] == CELL_TYPE_MUTANT && (pCell->stats["EAM_SUN"][0] < 30
                || pCell->stats["EAM_GND"][0] < 30 || pCell->stats["EAM_CELLS"][0] < 30)) continue;
        }
        bool includeCell = true;
        for(auto stat : statRestrictions){
            std::string statName = stat.first;
            if(pCell->stats.count(statName) == 0) continue;
            if(pCell->stats[statName][0] != statRestrictions[statName]){
                includeCell = false;
                break;
            }
        }
        if(!includeCell) continue;
        numRelevantCells++;
        for(auto stat : pCell->stats){
            std::string statName = stat.first;
            int statVal = stat.second[0];
            avgStats[statName] += statVal;
        }
    }
    if(numRelevantCells == 0) return;
    cout << label << ": frameNum = " << frameNum << ", numRelevantCells = " << numRelevantCells << endl;
    for(auto stat : avgStats){
        std::string statName = stat.first;
        avgStats[statName] /= numRelevantCells;
        if(statName == "rngAi_pctChanceIdle") cout << endl;
        cout << "  (" << statName << ", " << avgStats[statName] << ") ";
        //print_scalar_vals(statName, avgStats[statName]);
    }
    cout << endl;
}

// Repeat this function each frame. Return the frame number
int do_frame(bool doCellDecisions = true){
    frameStart = SDL_GetTicks();

    //cout << "Frame start\n";

    assign_cells_to_correct_regions();
    if(doCellDecisions && doCellAi){
        // The cells each decide what to do (e.g. speed, direction, doAttack, etc.) by updating their internal state
        for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->decide_next_frame(pActivesRegions, pCellsHist);
    }

    //cout << "a";


    // Cells move to their target positions based on their speed
    for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->update_target_pos();
    assign_cells_to_correct_regions();

    //cout << "b";

    // Cells apply all their non-movement decisions this frame
    //  such as attacking and cloning. Deaths are dealt with after
    if(doCellAi){
        // Bug fix: using for(auto pCell : pActives) is a bad idea when pActives changes size during the algorithm
        for(int i = pActives.size()-1; i >= 0; i--) {
            pActives[i]->apply_non_movement_decisions(pActives, pCellsHist, pActivesRegions);
        }
        assign_cells_to_correct_regions();
    }

    //cout << "c";

    // Cells move to new positions if enough force is applied
    for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->update_forces(pActivesRegions);
    for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->apply_forces();
    assign_cells_to_correct_regions();

    do_day_night_cycle();
    update_dayNightCycleTime();

    
    if(automateEnergy){
        for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->do_energy_transfer(pActivesRegions);
        for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->do_energy_decay(pActivesRegions);
        for(int i = pActives.size()-1; i >= 0; i--) pActives[i]->consume_energy_per_frame();
    }


    // Kill all cells which meet at least one of the conditions for dying
    for(int i = pActives.size() - 1; i >= 0; i--) {
        if(pActives[i]->isAlive == false) continue;
        if(pActives[i]->calc_if_cell_is_dead()) pActives[i]->kill_self();
    }

    // Deal with dead cells
    for(int i = pActives.size() - 1; i >= 0; i--) pActives[i]->remove_this_dead_cell_if_depleted(pActives, i);

    //cout << "d";

    // Every certain number of frames, the energy levels within the ground should be increased for all ground pixels
    if(automateEnergy && frameNum % FRAMES_BETWEEN_GND_ENERGY_ACCUMULATION == 0){
        increase_sim_gnd_energy(gndEnergyPerIncrease.val);
    }

    // Rendering and User Interactions
    assign_cells_to_correct_regions();
    #ifdef DO_VIDEO
    do_video1();
    #else
    SDL_draw_frame();
    #endif
    //cout << "\nFrame end\n";
    if(frameNum % 200 == 0 && (frameNum <= 2000 || frameNum % 1000 == 0) && (frameNum <= 10000 || frameNum % 2000 == 0)){
        cout << endl;
        disp_cell_statistics({{"cellType", CELL_TYPE_PLANT}},       "Plant Statistics"      );
        disp_cell_statistics({{"cellType", CELL_TYPE_WORM}},        "Worm Statistics"       );
        disp_cell_statistics({{"cellType", CELL_TYPE_PREDATOR}},    "Predator Statistics"   );
        disp_cell_statistics({{"cellType", CELL_TYPE_MUTANT}},      "Mutant Statistics"     );
    }
    SDL_event_handler();
    return ++frameNum;
}

void do_sim_iteration(bool doCellDecisions = true){
    switch(simState){
        case SIM_STATE_QUIT:
        exit_sim();
        return;
        case SIM_STATE_MAIN_MENU:
        #ifdef DO_VIDEO
        simState = SIM_STATE_INIT;
        #else
        SDL_event_handler();
        #endif
        break;
        case SIM_STATE_INIT:
        init_sim();
        break;
        case SIM_STATE_RESTART:
        restart_sim();
        break;
        default:
        frameNum = do_frame(doCellDecisions);
    }
}

#ifndef SECONDARY_INCLUDES_H
#include "../secondary/secondaryIncludes.h"
#define SECONDARY_INCLUDES_H
#endif



struct DeadCell;

// The main (possibly only) living organisms in the simulator. Their shape will be a circle
struct Cell {
    // Identity
    Cell* pSelf = NULL; // Place a pointer to self here
    //  NOTE: The user MUST define the pointer after placing it into a vector to be permanently kept as data.
    //  Otherwise, it will be the wrong pointer.
    Cell* parent = NULL;
    bool id[ID_LEN]; // An identifier which every creature "knows"
    //  which may change through random mutations, but ultimately
    //  does NOT change for an individual after birth
    int uniqueCellNum = -1;
    //  This is unique because pCellsHist never gets deleted from until the simulation ends

    // Variables the creature can control directly or indirectly
    int speedDir = 0; // Direction of travel in degrees
    char speedMode = IDLE_MODE; // 0 for idle, 1 for walking, 2 for running
    int cloningDirection = 0; // Any number between 0 and 359 (degrees).
    //  This output selects the direction in which the cell generates its clone
    //  Anything outside that range results in a randomly selected cloning direction
    bool doAttack = false; // If this is true and the creature can to attack, it will attack
    bool doSelfDestruct = false; // Kill the creature
    bool doCloning = false; // Clone the creature
    
    // Creature AI (this drives the creature to make decisions)
    std::vector<std::vector<aiNode>> aiNetwork;
    // Leave the first (input) and last (output) blank
    std::vector<int> nodesPerLayer = {-1, 10, -1}; // Fill in the hidden layer (middle) values.
    // Each entry forces a decision on the frame it describes
    //  The first entry in each slot represents the frame number the decision is repeatedly made until
    //  The remaining entries are the decisions the cell makes
    std::vector<std::tuple<int, int,int,int,bool,bool,bool>> forcedDecisionsQueue;

    // Internal timers
    int age = 0; // Relative to birth (limits lifespan)
    int attackCooldown = 0; // When this reaches 0, an attack can be done
    
    // Variables intrinsic to the creature's "genome"
    int speedIdle = 0; // Keep this at 0
    int speedWalk = -1; // The walking speed
    int speedRun = -1; // The running speed
    int visionDist = -1; // How far out the creature can see
    int stickiness = -1; // The tendency to stick to other cells, especially sticky ones
    int mutationRate = -1; // The rate at which genes mutate
    int health = -1; // When this reaches 0, the cell dies
    int maxHealth = -1;
    int attack = -1; // Attack power. Decreases other cells' health
    int dia = -1; // The organism's diameter
    //  This is an enforced constraint
    int EAM[NUM_EAM_ELE]; // Energy accumulation method
    //  There are 3 such methods, so they might as well
    //  be stored in a constant-sized array
    
    // Dependent (calculated) variables (must be updated
    //  if any of their dependent variables are updated)
    int size = -1; // Area (truncated at the decimal); Calculated from dia
    int posX = 0, posY = 0;
    std::pair<int, int> xyRegion = {0, 0}; // Each cell should be placed in their appropriate 'bin' of nearby cells
    int forceX = 0, forceY = 0; // Force moves the cell in the same direction as the force, if there is enough of it
    //  Negative values move the cell in the opposite direction
    int initEnergy = -1;
    int energy = -1; // Needed to do various abilities or keep oneself alive.
    //  Creature dies if energy <= 0.
    std::map<std::string, int> energyCostToCloneMap;
    int energyCostToClone = 0;
    std::map<std::string, int> energyCostPerUse;
    std::map<std::string, int> energyCostPerSecMap;
    int energyCostPerFrame = 0;


    // Stats
    //  TODO: Define them as a map of vectors formatted as:
    std::map<std::string, std::vector<int>> stats;

    // Bounds
    //  TODO: Make the lb, ub, and starting position of each parameter configurable
    //  inside the simulator
    int maxEnergy = 10000; // = Const * size
    int maxAttackCooldown = 10;
    int lbAttack = 0, ubAttack = 10000;
    int lbDia = 2, ubDia = 10;
    int lbEAM = 0, ubEAM = 100;
    int lbHealth = 1, ubHealth = 10000;
    int lbMutationRate = 0, ubMutationRate = 0; // 10000
    int lbSpeed = 0, ubSpeed = 100;
    int lbStickiness = 0, ubStickiness = 0;
    int lbVisionDist = 0, ubVisionDist = 1000;


    // Constructor
    Cell(){
        // Initialize all stats
        // Notation: Pct1k == one thousandth of the entire value
        // {{"stat1", {val, lb, ub, mutationPct1kChance, mutationMaxPct1kChange}}, ...}
        stats["attack"]         = {   -1,     0, 10000,  50, 100};
        stats["dex"]            = {    0,     0,     0,   0,   0}; // TODD: Add this as an actual stat
        stats["dia"]            = {   -1,     0, 10,     50, 100};
        stats["EAM[EAM_SUN]"]   = {   -1,     0, 100,    50, 100};
        stats["EAM[EAM_GND]"]   = {   -1,     0, 100,    50, 100};
        stats["EAM[EAM_CELLS]"] = {   -1,     0, 100,    50, 100};
        stats["maxAtkCooldown"] = {   10,     0,     0,   0,   0};
        stats["maxEnergy"]      = {10000, 10000, 10000,   0,   0};
        stats["maxHealth"]      = {   -1,     1, 10000,  50, 100};
        stats["mutationRate"]   = {   -1,     0,  1000,  50, 100};
        // TODO: change speedIdle, speedWalk, and speedRun to a "maxSpeed" stat and change speed to a continuously varying decision
        stats["speedIdle"]      = {    0,     0,     0,   0,   0};
        stats["speedWalk"]      = {   -1,     0,   100,  50, 100};
        stats["speedRun"]       = {   -1,     0,   100,  50, 100};
        stats["stickiness"]     = {    0,     0,     0,   0,   0};
        stats["visionDist"]     = {   -1,     0,  1000,  50, 100};
    }
    // Struct-specific methods
    void mutate_stats(){
        // Random mutation based on parent's mutation rate
        float probDefault = (float)parent->mutationRate / (float)parent->ubMutationRate;
        speedWalk = gen_normal_int_dist_special(rng, probDefault, parent->speedWalk, 1, 0, INT_MAX); // lb used to be 1
        // BUG FIXED: speedRun = ...; Used to have lb = parent->speedRun + 1
        speedRun = gen_normal_int_dist_special(rng, probDefault, parent->speedRun, 1, speedWalk, INT_MAX);
        if(speedRun < speedWalk) speedRun = speedWalk;
        visionDist = gen_normal_int_dist_special(rng, probDefault, parent->visionDist, 1 + mutationRate/50, 0, INT_MAX);
        stickiness = 0;
        mutationRate = gen_normal_int_dist_special(rng, 1, parent->mutationRate, 5 + parent->mutationRate/50, 0, ubMutationRate);
        maxHealth = gen_normal_int_dist_special(rng, probDefault, parent->maxHealth, 1 + parent->maxHealth/20, lbHealth, INT_MAX);
        health = maxHealth;
        attack = gen_normal_int_dist_special(rng, probDefault, parent->attack, 1 + parent->attack/20, lbAttack, INT_MAX);
        dia = gen_normal_int_dist_special(rng, probDefault, parent->dia, 1 + parent->dia/20, lbDia, ubDia);
        set_initEnergy(gen_normal_int_dist_special(rng, probDefault, parent->initEnergy, 10 + parent->initEnergy/20, 100, maxEnergy), false);
        update_size();
        for(int i = 0; i < NUM_EAM_ELE; i++){
            EAM[i] = gen_normal_int_dist_special(rng, probDefault, parent->EAM[i], 2 + mutationRate/100, 0, REQ_EAM_SUM);
        }
        enforce_EAM_constraints();
        for(int i = 0; i < ID_LEN; i++){
            if(std_uniform_dist(rng) < probDefault) id[i] = !id[i];
        }
        mutate_ai();
        update_energy_costs();

        enforce_valid_cell(true);
    }
    void gen_stats_random(int cellNum, std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions, 
    Cell* pCell = NULL){
        // Random generation from scratch
        pSelf = pCell;
        parent = NULL;
        uniqueCellNum = cellNum;
        speedWalk = 1; //gen_uniform_int_dist(rng, 1, 3);
        speedRun = 2; //gen_uniform_int_dist(rng, speedWalk + 1, 3*speedWalk);
        visionDist = 0;
        stickiness = 0;
        mutationRate = ubMutationRate / 10;
        maxHealth = lbHealth; //gen_uniform_int_dist(rng, lbHealth, 10);
        health = maxHealth;
        attack = lbAttack; //gen_uniform_int_dist(rng, lbAttack, 10);
        dia = lbDia; //gen_uniform_int_dist(rng, lbDia, ubDia);
        set_initEnergy(1000); //set_initEnergy(gen_uniform_int_dist(rng, 500, 2000), true);
        update_size();
        //for(int i = 0; i < NUM_EAM_ELE; i++) EAM[i] = 33;
        int cellType = gen_uniform_int_dist(rng, 0, 3); // Each type corresponds to its element number in EAM. The last type is "balanced"
        for(int i = 0; i < NUM_EAM_ELE; i++) EAM[i] = 0;
        
        switch(cellType){
            case EAM_SUN:
            EAM[EAM_SUN] = 100;
            break;
            case EAM_GND:
            EAM[EAM_GND] = 100;
            break;
            case EAM_CELLS:
            EAM[EAM_CELLS] = 100;
            attack = 1;
            visionDist = 5;
            break;
            default:
            EAM[EAM_SUN] = 33; EAM[EAM_GND] = 34; EAM[EAM_CELLS] = 33;
            visionDist = 5;
        }
        //for(int i = 0; i < NUM_EAM_ELE; i++) EAM[i] = gen_uniform_int_dist(rng, 0, REQ_EAM_SUM);
        enforce_EAM_constraints();
        for(int i = 0; i < ID_LEN; i++) id[i] = 0; //std_uniform_dist(rng) < 0.5;
        init_ai(pAlivesRegions);
        update_energy_costs();

        enforce_valid_cell(true);
    }
    void print_main_stats(){
        print_id();
        print_pos_speed("  ");
        std::cout << "  speedWalk: " << speedWalk << std::endl;
        std::cout << "  speedRun: " << speedRun << std::endl;
        std::cout << "  visionDist: " << visionDist << std::endl;
        std::cout << "  stickiness: " << stickiness << std::endl;
        std::cout << "  mutationRate: " << mutationRate << std::endl;
        std::cout << "  health: " << health << " / " << maxHealth << std::endl;
        std::cout << "  attack: " << attack << std::endl;
        std::cout << "  dia: " << dia << std::endl;
        std::cout << "  size: " << size << std::endl;
        std::cout << "  pos: { " << posX << ", " << posY << "}\n";
        print_EAM();
    }
    void print_stat(std::string statName, int updateMask = 0x1F){
        cout << "  " << statName << ": ";
        for(int i = statName.size(); i < 20; i++) cout << " ";
        if(updateMask & 0x01) cout << stats[statName][0];
        for(int i = 1, nextStatMask = 0x01; i < stats[statName].size(); i++){
            if(updateMask > nextStatMask) cout << ", ";
            nextStatMask = nextStatMask << 1;
            if(updateMask & nextStatMask) cout << stats[statName][i];
        }
        cout << endl;
    }
    void print_stats(int updateMask = 0x1F){
        print_id();
        print_pos_speed("  ");
        for(auto item : stats){
            print_stat(item.first, updateMask);
        }
    }
    void set_int_stats(std::map<std::string, int>& varVals, int aiPreset = -1){
        // TODO: Include the ability to set the aiNetwork and nodesPerLayer
        // TODO: Since I removed the decisions from this function, I may have to edit the unit tests
        // Only contains functionality for the more important stats
        int lenVarVals = 0;
        if(varVals.count("age"))            {lenVarVals++; age = varVals["age"];}
        if(varVals.count("attack"))         {lenVarVals++; attack = varVals["attack"];}
        if(varVals.count("attackCooldown")) {lenVarVals++; attackCooldown = varVals["attackCooldown"];}
        if(varVals.count("dia"))            {lenVarVals++; dia = varVals["dia"];}
        if(varVals.count("EAM[EAM_CELLS]")) {lenVarVals++; EAM[EAM_CELLS] = varVals["EAM[EAM_CELLS]"];}
        if(varVals.count("EAM[EAM_GND]"))   {lenVarVals++; EAM[EAM_GND] = varVals["EAM[EAM_GND]"];}
        if(varVals.count("EAM[EAM_SUN]"))   {lenVarVals++; EAM[EAM_SUN] = varVals["EAM[EAM_SUN]"];}
        if(varVals.count("energy"))         {lenVarVals++; energy = varVals["energy"];}
        if(varVals.count("maxEnergy"))      {lenVarVals++; maxEnergy = varVals["maxEnergy"];}
        if(varVals.count("health"))         {lenVarVals++; health = varVals["health"];}
        if(varVals.count("maxHealth"))      {lenVarVals++; maxHealth = varVals["maxHealth"];}
        if(varVals.count("mutationRate"))   {lenVarVals++; mutationRate = varVals["mutationRate"];}
        if(varVals.count("posX"))           {lenVarVals++; posX = varVals["posX"];}
        if(varVals.count("posY"))           {lenVarVals++; posY = varVals["posY"];}
        if(varVals.count("speedRun"))       {lenVarVals++; speedRun = varVals["speedRun"];}
        if(varVals.count("speedWalk"))      {lenVarVals++; speedWalk = varVals["speedWalk"];}
        if(varVals.count("speedIdle"))      {lenVarVals++; speedIdle = varVals["speedIdle"];}
        if(varVals.count("stickiness"))     {lenVarVals++; stickiness = varVals["stickiness"];}
        if(varVals.count("visionDist"))     {lenVarVals++; visionDist = varVals["visionDist"];}

        // AI weights
        if(aiPreset >= 0) apply_ai_preset(aiPreset);

        // Ensure that varVals does NOT contain values not accounted for in this function
        if(lenVarVals != varVals.size()) print_scalar_vals("lenVarVals: ", lenVarVals, "varVals.size()", varVals.size());
        assert(lenVarVals == varVals.size());
        
        // Ensure we update all dependent variables
        enforce_valid_cell(true);
    }
    // updateMask = entries in the dictionary to update (index 0 of the map entry refers to index 0 of the entry and so on)
    //  For example, 0x1F: Update all, 0x01: Only update val, 0x06: Only update lb and ub
    void copy_stat_to_map(std::string statName, int updateMask, int val, int lb, int ub, int mutationPct1kChance, int mutationMaxPct1kChange){
        if(updateMask & 0x01) stats[statName][0] = val;
        if(updateMask & 0x02) stats[statName][1] = lb;
        if(updateMask & 0x04) stats[statName][2] = ub;
        if(updateMask & 0x08) stats[statName][3] = mutationPct1kChance;
        if(updateMask & 0x10) stats[statName][4] = mutationMaxPct1kChange;
    }
    void copy_stats_to_map(int updateMask, std::set<std::string> statsToNotCopy = {}){
        updateMask &= 0x01;
        if(!statsToNotCopy.count("attack")) copy_stat_to_map("attack", updateMask, attack, 0, 0, 0, 0);
        //if(!statsToNotCopy.count("dex")) copy_stat_to_map("dex", updateMask, dex, 0, 0, 0, 0);
        if(!statsToNotCopy.count("dia")) copy_stat_to_map("dia", updateMask, dia, 0, 0, 0, 0);
        if(!statsToNotCopy.count("EAM[EAM_SUN]")) copy_stat_to_map("EAM[EAM_SUN]", updateMask, EAM[EAM_SUN], 0, 0, 0, 0);
        if(!statsToNotCopy.count("EAM[EAM_GND]")) copy_stat_to_map("EAM[EAM_GND]", updateMask, EAM[EAM_GND], 0, 0, 0, 0);
        if(!statsToNotCopy.count("EAM[EAM_CELLS]")) copy_stat_to_map("EAM[EAM_CELLS]", updateMask, EAM[EAM_CELLS], 0, 0, 0, 0);
        if(!statsToNotCopy.count("maxAtkCooldown")) copy_stat_to_map("maxAtkCooldown", updateMask, maxAttackCooldown, 0, 0, 0, 0);
        if(!statsToNotCopy.count("maxEnergy")) copy_stat_to_map("maxEnergy", updateMask, maxEnergy, 0, 0, 0, 0);
        if(!statsToNotCopy.count("maxHealth")) copy_stat_to_map("maxHealth", updateMask, maxHealth, 0, 0, 0, 0);
        if(!statsToNotCopy.count("mutationRate")) copy_stat_to_map("mutationRate", updateMask, mutationRate, 0, 0, 0, 0);
        if(!statsToNotCopy.count("speedIdle")) copy_stat_to_map("speedIdle", updateMask, speedIdle, 0, 0, 0, 0);
        if(!statsToNotCopy.count("speedWalk")) copy_stat_to_map("speedWalk", updateMask, speedWalk, 0, 0, 0, 0);
        if(!statsToNotCopy.count("speedRun")) copy_stat_to_map("speedRun", updateMask, speedRun, 0, 0, 0, 0);
        if(!statsToNotCopy.count("stickiness")) copy_stat_to_map("stickiness", updateMask, stickiness, 0, 0, 0, 0);
        if(!statsToNotCopy.count("visionDist")) copy_stat_to_map("visionDist", updateMask, visionDist, 0, 0, 0, 0);
    }
    std::vector<Cell*> find_touching_cells(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        std::set<int> checkedCells = {uniqueCellNum};
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
                if(checkedCells.count(pCell->uniqueCellNum)) continue;
                if(pCell->calc_distance_from_point(posX, posY) > (float)(dia + pCell->dia + 0.1) / 2) continue;
                ans.push_back(pCell);
                checkedCells.insert(pCell->uniqueCellNum);
            }
        }
        return ans;
    }
    int get_id_similarity(Cell* pCellOther){
        int ans = ID_LEN;
        for(int i = 0; i < ID_LEN; i++){
            if(id[i] != pCellOther->id[i]) ans--;
        }
        return ans;
    }
    std::vector<std::pair<int, int>> get_neighboring_xyRegions(){
        int xReg = xyRegion.first, yReg = xyRegion.second;
        std::vector<std::pair<int, int>> neighboringRegions = {
            xyRegion,
            {xReg-1, yReg}, {xReg+1, yReg}, {xReg, yReg-1}, {xReg, yReg+1},
            {xReg-1, yReg-1}, {xReg-1, yReg+1}, {xReg+1, yReg-1}, {xReg+1, yReg+1}
        };
        for(int i = 0; i < neighboringRegions.size(); i++){
            while(neighboringRegions[i].first < 0) neighboringRegions[i].first += cellRegionNumUbX;
            while(neighboringRegions[i].second < 0) neighboringRegions[i].second += cellRegionNumUbY;
            neighboringRegions[i].first %= cellRegionNumUbX;
            neighboringRegions[i].second %= cellRegionNumUbY;
        }
        return neighboringRegions;
    }
    // NOTE: this function does NOT have access to the entire list of cells,
    //  so a separate function needs to be run to organize the cells into
    //  region-based lists
    void assign_self_to_xyRegion(){
        xyRegion.first = saturate_int(posX / CELL_REGION_SIDE_LEN, 0, cellRegionNumUbX-1);
        xyRegion.second = saturate_int(posY / CELL_REGION_SIDE_LEN, 0, cellRegionNumUbY-1);
    }
    void teleport_self(int target_x, int target_y){
        posX = target_x;
        posY = target_y;
        enforce_valid_xyPos();
    }
    void enforce_wrap_around_x(){
        while(posX < 0) posX += ubX.val;
        posX %= ubX.val;
        assign_self_to_xyRegion();
    }
    void enforce_wrap_around_y(){
        while(posY < 0) posY += ubY.val;
        posY %= ubY.val;
        assign_self_to_xyRegion();
    }
    void enforce_valid_xyPos(){
        if(WRAP_AROUND_X) enforce_wrap_around_x();
        else posX = saturate_int(posX, 0, ubX.val);
        if(WRAP_AROUND_Y) enforce_wrap_around_y();
        else posY = saturate_int(posY, 0, ubY.val);
        assign_self_to_xyRegion();
    }
    void update_max_energy(){
        maxEnergy = 5000*size;
    }
    void update_size(){
        size = PI*dia*dia/4 + 0.5; // size is an int
        if(automateEnergy) update_max_energy();
    }
    int calc_EAM_sum(){
        int EAM_sum = 0;
        for(int i = 0; i < NUM_EAM_ELE; i++){
            EAM_sum += EAM[i];
        }
        return EAM_sum;
    }
    bool verify_EAM_constraints(){
        // Constraint: 0 <= EAM[i] && EAM[i] <= 100
        for(int i = 0; i < NUM_EAM_ELE; i++){
            if(EAM[i] < 0 || 100 < EAM[i]) return false;
        }
        // Constraint: EAM_sum == REQ_EAM_SUM
        int EAM_sum = 0;
        for(int i = 0; i < NUM_EAM_ELE; i++){
            EAM_sum += EAM[i];
        }
    }
    void enforce_EAM_constraints(){
        // Enforce the EAM constraints such that all elements are >= 0
        //  and they add to 100
        // Ensure each element of EAM >= 0
        for(int i = 0; i < NUM_EAM_ELE; i++){
            if(EAM[i] < 0) EAM[i] = 0;
        }
        // Ensure that EAM_sum == EAM_SUM as defined in this struct
        int EAM_sum = calc_EAM_sum();
        if(EAM_sum == 0) {
            for(int i = 0; i < NUM_EAM_ELE; i++) {
                EAM[i] = REQ_EAM_SUM / NUM_EAM_ELE;
            }
        } else {
            // Divide each entry by their sum
            for(int i = 0; i < NUM_EAM_ELE; i++) {
                EAM[i] = EAM[i] * REQ_EAM_SUM / EAM_sum;
            }
        }
        int remainder = REQ_EAM_SUM - calc_EAM_sum();
        for(int i = 0; remainder > 0; i++){
            if(i >= NUM_EAM_ELE) i = 0;
            EAM[i]++;
            remainder--;
        }
        // To ensure bugs don't occur
        EAM_sum = calc_EAM_sum();
        assert(EAM_sum == REQ_EAM_SUM);
    }
    int enforce_bounds(int val, int lb, int ub){
        assert(lb <= ub);
        if(val < lb) val = lb;
        if(val > ub) val = ub;
        return val;
    }
    // Only consider the nearest cells within the cell's field of view
    std::vector<Cell*> get_nearest_cells(int maxNumCellsToReturn,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // visionDist: The distance the cell can see
        int xReg = xyRegion.first, yReg = xyRegion.second;
        int reg_dX = CELL_REGION_SIDE_LEN, reg_dY = CELL_REGION_SIDE_LEN;
        //int _lbX = posX - visionDist, _ubX = posX + visionDist;
        //int _lbY = posY - visionDist, _ubY = posY + visionDist;
        
        // A region counts as too far if it its nearest point is out of range
        int visionDist_NumRegX = visionDist / reg_dX + 1;
        int visionDist_NumRegY = visionDist / reg_dY + 1;
        // A crude estimate so I don't need to rely on precise calculations
        int visionDist_NumRegXY = sqrt(visionDist_NumRegX*visionDist_NumRegX
            + visionDist_NumRegY*visionDist_NumRegY + 1);
        
        // Sort every region in a copy of pAlivesRegions based on the distance
        //  from the current region
        std::vector<std::pair<int,int>> nearest_xyReg;
        // Only evaluate the regions nearest to the current cell
        for(auto pReg : pAlivesRegions){
            int distX = (pReg.first.first - xReg);
            int distY = (pReg.first.second - yReg);
            int distXY = sqrt(distX*distX + distY*distY);
            if(distXY <= visionDist_NumRegXY) nearest_xyReg.push_back(pReg.first);
        }

        // Go through all the regions within visionDist from the current cell
        std::vector<Cell*> nearestCells;
        for(auto pReg : nearest_xyReg){
            for(auto pCell : pAlivesRegions[pReg]){
                int distX = pCell->posX - posX;
                int distY = pCell->posY - posY;
                int distXY = sqrt(distX*distX + distY*distY);
                if(distXY <= visionDist && pCell != pSelf){
                    nearestCells.push_back(pCell);
                }
            }
        }
        // Temporarily subtract xReg from the x values and yReg from the y values
        //  of nearestCells
        for(int i = 0; i < nearest_xyReg.size(); i++){
            nearest_xyReg[i].first -= xReg;
            nearest_xyReg[i].second -= yReg;
        }
        // Sort the nearest cells by distance
        std::sort(nearest_xyReg.begin(), nearest_xyReg.end(),
            [](auto &left, auto &right){
                int distX = left.first, distY = left.second;
                float distXY_left = sqrt(distX*distX + distY*distY);
                distX = right.first; distY = right.second;
                float distXY_right = sqrt(distX*distX + distY*distY);

                return distXY_right > distXY_left;
            }
        );
        // Add xReg and yReg back to the regional coordinates of each cell
        for(int i = 0; i < nearest_xyReg.size(); i++){
            nearest_xyReg[i].first += xReg;
            nearest_xyReg[i].second += yReg;
        }

        // Remove cells which occur too late in the list
        while(nearestCells.size() > maxNumCellsToReturn) nearestCells.pop_back();
        return nearestCells;
    }
    // TODO: ensure that each cell id similarity value only gets up to 3 cells each
    // NOTE: This function also determines what the AI inputs are
    std::vector<float> get_ai_inputs(
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<float> ans;
        int _numAiInputs = 0;
        // Internal Timers
        ans.push_back((float)age); _numAiInputs++;
        ans.push_back((float)attackCooldown); _numAiInputs++;
        // controlable stats
        ans.push_back((float)speedDir); _numAiInputs++;
        ans.push_back((float)get_speed()); _numAiInputs++;
        ans.push_back((float)cloningDirection); _numAiInputs++;
        // genome
        ans.push_back((float)visionDist); _numAiInputs++;
        ans.push_back((float)stickiness); _numAiInputs++;
        ans.push_back((float)mutationRate); _numAiInputs++;
        ans.push_back((float)maxHealth); _numAiInputs++;
        ans.push_back((float)attack); _numAiInputs++;
        ans.push_back((float)dia); _numAiInputs++;
        for(int i = 0; i < NUM_EAM_ELE; i++) {
            ans.push_back((float)EAM[i]); _numAiInputs++;
        }
        // status or state
        ans.push_back((float)health); _numAiInputs++;
        ans.push_back((float)forceX); _numAiInputs++;
        ans.push_back((float)forceY); _numAiInputs++;
        ans.push_back((float)energy); _numAiInputs++;
        // Stats of nearest cells
        //  (a) Find out which 10 cells are closest
        //  (b) For now, we just care about their id similarity
        int maxNumCellsSeen = 10;
        std::vector<Cell*> pNearestCells = get_nearest_cells(maxNumCellsSeen, pAlivesRegions);
        for(int i = 0; i < maxNumCellsSeen; i++){
            int relDist = 0, relDirOther = 0;
            int relSpeedRadial = 0, relSpeedTangential = 0;
            int healthOther = 0, energyOther = 0, ageOther = 0;
            int idSimilarity = 0;
            if(i < pNearestCells.size()){
                Cell* pCell = pNearestCells[i];
                relDist = calc_distance_from_point(pCell->posX, pCell->posY);
                // Standard angle of linepointing from other cell to current cell
                relDirOther = (pCell->speedDir + 180) % 360; // degrees
                // Positive indicates movement toward the cell
                relSpeedRadial = pCell->get_speed() * cos_deg(relDirOther);
                // Positive indicates clockwise movement around the cell
                relSpeedTangential = pCell->get_speed() * sin_deg(relDirOther);
                healthOther = pCell->health;
                energyOther = pCell->energy;
                ageOther = pCell->age;
                idSimilarity = get_id_similarity(pCell);
            }
            // Distance, tangential and radial speed, direction, health,
            //  energy, age, id similarity
            ans.push_back(relDist); _numAiInputs++;
            ans.push_back(relSpeedRadial); _numAiInputs++;
            ans.push_back(relSpeedTangential); _numAiInputs++;
            ans.push_back(healthOther); _numAiInputs++;
            ans.push_back(energyOther); _numAiInputs++;
            ans.push_back(ageOther); _numAiInputs++;
            ans.push_back(idSimilarity); _numAiInputs++;
        }
        // Ensure the neural network input layer is valid
        if(nodesPerLayer[0] < 0) nodesPerLayer[0] = ans.size();
        else assert(ans.size() == nodesPerLayer[0]);
        assert(_numAiInputs == nodesPerLayer[0]);
        return ans;
    }
    void set_ai_outputs(int _speedDir, int _cloningDirection, int _speedMode,
            bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        int _numAiOutputs = 0;
        assert(0 <= _speedDir && _speedDir < 360);
        speedDir = _speedDir; _numAiOutputs++;
        assert(0 <= _cloningDirection && _cloningDirection < 360);
        cloningDirection = _cloningDirection; _numAiOutputs++;
        assert(_speedMode == IDLE_MODE || _speedMode == WALK_MODE || _speedMode == RUN_MODE);
        speedMode = _speedMode; _numAiOutputs++;
        doAttack = _doAttack; _numAiOutputs++;
        doSelfDestruct = _doSelfDestruct; _numAiOutputs++;
        doCloning = _doCloning; _numAiOutputs++;
        assert(_numAiOutputs == nodesPerLayer[nodesPerLayer.size() - 1]);
        enforce_valid_ai();
    }
    std::tuple<std::vector<int>, std::vector<bool>> get_ai_outputs(){
        std::vector<int> intVec;
        int _numAiOutputs = 0;
        intVec.push_back(speedDir); _numAiOutputs++;
        intVec.push_back(cloningDirection); _numAiOutputs++;
        intVec.push_back(speedMode); _numAiOutputs++;
        std::vector<bool> boolVec;
        boolVec.push_back(doAttack); _numAiOutputs++;
        boolVec.push_back(doSelfDestruct); _numAiOutputs++;
        boolVec.push_back(doCloning); _numAiOutputs++;
        if(nodesPerLayer[nodesPerLayer.size()-1] < 0) {
            nodesPerLayer[nodesPerLayer.size()-1] = _numAiOutputs;
        }
        return {intVec, boolVec};
    }
    void update_energy_costs(){
        // Cloning
        energyCostToClone = 0;
        energyCostToCloneMap["base"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["base"], {{"x", 2*initEnergy}});
        if(visionDist) energyCostToCloneMap["visionDist"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["visionDist"], {{"x", visionDist}, {"size", size}});
        else energyCostToCloneMap["visionDist"] = 0;
        if(stickiness) energyCostToCloneMap["stickiness"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["stickiness"], {{"x", stickiness}, {"size", size}});
        else energyCostToCloneMap["stickiness"] = 0;
        if(attack) energyCostToCloneMap["attack"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["attack"], {{"x", attack}, {"size", size}});
        else energyCostToCloneMap["attack"] = 0;
        energyCostToCloneMap["size"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["size"], {{"x", size}, {"size", size}});
        for(auto item : energyCostToCloneMap) energyCostToClone += item.second;

        // Surviving (per second)
        energyCostPerFrame = 0;
        energyCostPerSecMap["base"] = StrExprInt::solve(ENERGY_COST_PER_USE["base"],
            {{"x", -1}, {"size", size}});
        energyCostPerSecMap["visionDist"] = StrExprInt::solve(ENERGY_COST_PER_USE["visionDist"],
            {{"x", visionDist}, {"size", size}});
        energyCostPerSecMap["stickiness"] = StrExprInt::solve(ENERGY_COST_PER_USE["stickiness"],
            {{"x", stickiness}, {"size", size}});
        energyCostPerSecMap["mutationRate"] = StrExprInt::solve(ENERGY_COST_PER_USE["mutationRate"],
            {{"x", mutationRate}, {"size", size}});
        energyCostPerSecMap["age"] = StrExprInt::solve(ENERGY_COST_PER_USE["age"],
            {{"x", age}, {"size", size}});
        energyCostPerSecMap["maxHealth"] = StrExprInt::solve(ENERGY_COST_PER_USE["maxHealth"],
            {{"x", maxHealth}, {"size", size}});
        for(auto item : energyCostPerSecMap) energyCostPerFrame += item.second;
        energyCostPerFrame /= TICKS_PER_SEC;

        // Surviving (per frame) and using abilities
        energyCostPerUse["speedRun"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],
            {{"x", speedRun}, {"size", size}});
        energyCostPerUse["speedWalk"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],
            {{"x", speedWalk}, {"size", size}});
        energyCostPerUse["speedIdle"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],
            {{"x", speedIdle}, {"size", size}});
        energyCostPerUse["attack"] = StrExprInt::solve(ENERGY_COST_PER_USE["attack"],
            {{"x", attack}, {"size", size}});
    }
    void consume_energy_per_frame(){
        update_energy_costs();
        energy -= energyCostPerFrame;
        if(speedMode == IDLE_MODE) energy -= energyCostPerUse["speedIdle"] / TICKS_PER_SEC;
        if(speedMode == WALK_MODE) energy -= energyCostPerUse["speedWalk"] / TICKS_PER_SEC;
        if(speedMode == RUN_MODE)  energy -= energyCostPerUse["speedRun"] / TICKS_PER_SEC;
    }
    void enforce_valid_ai(){
        assert(aiNetwork.size() == nodesPerLayer.size() - 1);
        for(auto num : nodesPerLayer) assert(0 < num);
        for(int layerNum = 1; layerNum < aiNetwork.size(); layerNum++){
            assert(aiNetwork[layerNum-1].size() == nodesPerLayer[layerNum]);
            //  aiNetwork[layerNum-1].size() is the number of nodes in layerNum,
            //  where layer 0 is the input layer
        }
    }
    // If the cell isn't valid, change the variables so it is valid.
    // Also, update the dependent variables
    void enforce_valid_cell(bool enforceStats){
        assert(uniqueCellNum >= 0);
        while(speedDir < 0) speedDir += 360*100;
        speedDir %= 360;
        if(enforceStats){
            saturate_int(dia, lbDia, ubDia);
            update_size();
        }
        saturate_int(energy, 0, maxEnergy);
        if(enforceStats) saturate_int(mutationRate, 0, ubMutationRate);
        while(cloningDirection < 0) cloningDirection += 360*100;
        cloningDirection %= 360;
        assert(speedMode == IDLE_MODE || speedMode == WALK_MODE || speedMode == RUN_MODE);
        enforce_valid_xyPos();
        if(enforceStats){
            // Only run this if the stats were changed
            enforce_EAM_constraints();
            enforce_valid_ai();
            update_energy_costs();
        }
        copy_stats_to_map(0x01);
    }
    // TODO: Create a function to set the weights of the AI (partially done)
    void apply_ai_preset(int aiPreset = -1){
        //aiNode node = aiNetwork[layer][nodeNum];
        int numWeightsPerNode;
        std::vector<int> nodeWeightsAndBiases; // {bias, w1, ..., wLast}
        int layer, node, i;
        switch(aiPreset){
            case 0:
            for(layer = 0; layer < aiNetwork.size(); layer++){
                for(node = 0; node < aiNetwork[layer].size(); node++){
                    nodeWeightsAndBiases.clear();
                    numWeightsPerNode = aiNetwork[layer][node].inputWeights.size();
                    for(i = 0; i < numWeightsPerNode; i++){
                        nodeWeightsAndBiases.push_back(0);
                    }
                    aiNetwork[layer][node].set_node_weights_and_biases(nodeWeightsAndBiases);
                }
            }
            break;

            default:
            break;
        }

    }
    void init_ai(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // NOTE: Do NOT use this function until all the inputs are initialized
        std::vector<float> aiInputs = get_ai_inputs(pAlivesRegions);
        std::tuple<std::vector<int>, std::vector<bool>> aiOutputs = get_ai_outputs();

        // Start with the (first) hidden layer, doing more of them if needed
        for(int i = 1; i < nodesPerLayer.size(); i++){
            assert(0 < nodesPerLayer[i] && nodesPerLayer[i] < 1000);
        }
        for(int layerNum = 1; layerNum < nodesPerLayer.size(); layerNum++){
            std::vector<aiNode> layerNodes;
            //std::cout << layerNum << "," << nodesPerLayer[layerNum] << "|";
            for(int nodeNum = 0; nodeNum < nodesPerLayer[layerNum]; nodeNum++){
                aiNode nextNode;
                nextNode.init_node(1000 * layerNum + nodeNum, false, nodesPerLayer[layerNum - 1]);
                layerNodes.push_back(nextNode);
            }
            aiNetwork.push_back(layerNodes);
        }
        //for(auto num : nodesPerLayer) std::cout << num << ","; std::cout << std::endl;
        //for(auto ele : aiNetwork) std::cout << ele.size() << ","; std::cout << std::endl;
    }
    std::vector<float> do_forward_prop_1_layer(std::vector<float> layerInputs, int layerNum){
        // layerNum == 0 means the input layer
        std::vector<float> ans;
        for(int nodeNum = 0; nodeNum < aiNetwork[layerNum-1].size(); nodeNum++){
            aiNetwork[layerNum-1][nodeNum].do_forward_propagation(layerInputs);
            ans.push_back(aiNetwork[layerNum-1][nodeNum].outputVal);
        }
        assert(ans.size() == nodesPerLayer[layerNum]);
        return ans;
    }
    void update_timers(){
        age++;
        if(attackCooldown > 0) attackCooldown--;
    }
    void force_decision(int numFrames, int _speedDir, int _cloningDir, int _speedMode, bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        forcedDecisionsQueue.push_back({numFrames, _speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning});
    }
    void clear_forced_decisions(){
        forcedDecisionsQueue.clear();
    }
    // To override the ai, append an entry to forcedDecisionsQueue
    //  TODO: Allow for only certain decisions to be forced
    void decide_next_frame(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Modify the values the creature can directly control based on the ai
        //  i.e. the creature decides what to do based on this function
        //std::cout << aiNetwork.size() << ", " << nodesPerLayer.size() << std::endl;
        //enforce_valid_ai();
        int _speedDir = speedDir, _cloningDir = cloningDirection, _speedMode = speedMode;
        bool _doAttack = doAttack, _doSelfDestruct = doSelfDestruct, _doCloning = doCloning;
        update_timers();

        // If a decision is forced, then follow that decision
        if(forcedDecisionsQueue.size() > 0){
            #define x(i) std::get<i>(forcedDecisionsQueue[0])
            x(0)--;
            if(0 <= x(1) && x(1) < 360) _speedDir = x(1);
            if(0 <= x(2) && x(2) < 360) _cloningDir = x(2);
            if(x(3) == IDLE_MODE || x(3) == WALK_MODE || x(3) == RUN_MODE) _speedMode = x(3);
            _doAttack = x(4) && attackCooldown == 0;
            _doSelfDestruct = x(5);
            _doCloning = x(6);
            set_ai_outputs(_speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
            if(x(0) <= 0) forcedDecisionsQueue.erase(forcedDecisionsQueue.begin());
            #undef x
            return;
        }
        // If the AI is free to decide, then decide what to do
        if(aiMode == EVOLUTIONARY_NEURAL_NETWORK_AI_MODE && forcedDecisionsQueue.size() == 0){
            std::vector<float> layerInputs = get_ai_inputs(pAlivesRegions);
            for(int layerNum = 1; layerNum < aiNetwork.size(); layerNum++){
                layerInputs = do_forward_prop_1_layer(layerInputs, layerNum);
            }
            _speedDir = saturate_int((int)layerInputs[0], 0, 359);
            _cloningDir = saturate_int((int)layerInputs[2], 0, 359);
            _speedMode = (char)saturate_int((char)layerInputs[3], IDLE_MODE, RUN_MODE);
            _doAttack = (layerInputs[4] >= 0 && enableAutomaticAttack && attackCooldown == 0);
            _doSelfDestruct = (layerInputs[5] >= 1 && enableAutomaticSelfDestruct); // If this condition is too easy to trigger, then cells die too easily
            _doCloning = (layerInputs[6] >= 0 && enableAutomaticCloning);
        } else if(aiMode == RNG_BASED_AI_MODE){
            _cloningDir = rand() % 360;
            _doAttack = enableAutomaticAttack && attackCooldown == 0;
            _doSelfDestruct = enableAutomaticSelfDestruct;
            _doCloning = enableAutomaticCloning;
            if(EAM[EAM_SUN] == 100){
                _speedDir = 0; _speedMode = IDLE_MODE; _doAttack = false;
            } else if(EAM[EAM_GND] == 100){
                preplan_random_cell_activity(5, 5, 200, false, _doCloning);
            } else {
                std::vector<Cell*> pNearestCells = get_nearest_cells(20, pAlivesRegions);
                if(pNearestCells.size() == 0 || !_doAttack){
                    preplan_random_cell_activity(5, 5, 1, _doAttack, _doCloning);
                } else {
                    chase_optimal_cell(pAlivesRegions, pNearestCells, _doAttack, false, _doCloning);
                }
            }
        }
        set_ai_outputs(_speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
        enforce_valid_cell(false);
    }
    void mutate_ai(){
        float prob = (float)mutationRate / ubMutationRate;
        for(int i = 0; i < aiNetwork.size(); i++){
            for(int j = 0; j < aiNetwork[i].size(); j++){
                aiNetwork[i][j].mutate_node(mutationRate, prob);
            }
        }
        //enforce_valid_cell(true);
        enforce_valid_ai();
    }
    void define_self(Cell* pCell){
        // NOTE: If I push a copy of the cell into a new location, I need to update self
        pSelf = pCell;
    }
    void set_initEnergy(int val, bool setEnergy = true){
        initEnergy = val;
        if(setEnergy) energy = initEnergy;
    }
    // NOTE: The full energy accumulation can only be done after this function is applied to every cell
    //  in the local area.
    // This function causes cells to accumulate energy from the sun, ground, and dead cells.
    //  Also, energy loss due to overcrowding leads is applied by this function
    void do_energy_transfer(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Energy from the sun
        //  First, calculate which cells are touching the current cell
        std::vector<Cell*> touchingCells = find_touching_cells(pAlives, pAlivesRegions);
        //  TODO: Give each cell a dexterity (dex) stat, which will help them NOT be blocked by other cells.
        //  Bigger cells get more of the energy and will receive most of the energy if competing with smaller cells.
        int sumOfCellSizes = size;
        for(auto cell : touchingCells) sumOfCellSizes += cell->size;
        energy += (float)energyFromSunPerSec * EAM[EAM_SUN] * size / 100 / sumOfCellSizes;
        
        // Energy from the ground -> Energy may be shared between cells,
        //  so it is better to add a pointer to the cell to each applicable ground cell's
        //  list of cells to which it will distribute energy
        enforce_valid_xyPos();
        if(simGndEnergy[posY][posX] < EAM[EAM_GND]){
            energy += simGndEnergy[posY][posX];
            simGndEnergy[posY][posX] = 0;
        } else {
            energy += EAM[EAM_GND];
            simGndEnergy[posY][posX] -= EAM[EAM_GND];
        }
        
        // Energy from cells which just died -> Add a pointer to the cell to the list
        //  associated with the cell that just died last frame AND is within range
        //  NOTE: This is dealt with in the DeadCell struct

        // Energy loss from overcrowding directly
        // TODO: Create a dex stat to resist this overcrowding
        energy -= StrExprInt::solve(ENERGY_COST_PER_USE["overcrowding"],
            {{"overcrowdingEnergyCoef", overcrowdingEnergyCoef.val},
            {"x", sumOfCellSizes-size}, {"size", size}});
        //cout << StrExprInt::solve(ENERGY_COST_PER_USE["overcrowding"],
        //    {{"overcrowdingEnergyCoef", overcrowdingEnergyCoef.val},
        //    {"x", sumOfCellSizes-size}, {"size", size}}) << ", ";

        // Enforce energy constraints
        if(energy > maxEnergy) energy = maxEnergy;

        enforce_valid_cell(false);
    }
    void randomize_pos(int _lbX, int _ubX, int _lbY, int _ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, _lbX, _ubX);
        posY = gen_uniform_int_dist(rng, _lbY, _ubY);
        enforce_valid_xyPos();
    }
    Cell* clone_self(int cellNum, int targetCloningDir = -1, bool randomizeCloningDir = false, bool doMutation = true){
        // The clone's position will be roughly the cell's diameter plus 1 away from the cell
        Cell* pClone = new Cell;
        *pClone = *pSelf; // Almost all quantities should be copied over perfectly
        // However, the pointers referring to the cells' identities stored within the cloned cell
        //  must be changed
        assert(pSelf != NULL);
        pClone->uniqueCellNum = cellNum;
        pClone->pSelf = pClone;
        pClone->parent = pSelf;
        pSelf->energy -= pSelf->energyCostToClone;
        pClone->energy = initEnergy;
        // Update the new cell's position
        // Determine the cloning direction
        if(targetCloningDir < 0 || 360 <= targetCloningDir) {
            assert(randomizeCloningDir == false);
            cloningDirection = targetCloningDir;
        } else if (randomizeCloningDir) {
            cloningDirection = gen_uniform_int_dist(rng, 0, 359);
        }
        int tmp = (dia + pClone->dia + 3) / 2;
        pClone->update_pos(posX + tmp*cos_deg(cloningDirection), posY + tmp*sin_deg(cloningDirection));
        // Reset all internal timers
        pClone->age = 0;
        pClone->attackCooldown = 0;
        //cout << "A cell cloned itself! id = "; for(auto digit : pClone->id) cout << digit; cout << endl;
        if(doMutation) pClone->mutate_stats();
        enforce_valid_cell(false);
        return pClone;
    }
    void print_id(){
        std::cout << "id: "; for(bool b : id) std::cout << b; std::cout << std::endl;
    }
    void print_EAM(){
        std::cout << "  EAM: { ";
        for(int i = 0; i < NUM_EAM_ELE; i++) std::cout << EAM[i] << " ";
        std::cout << "}\n";
    }
    int get_speed(){
        switch(speedMode){
            case IDLE_MODE:
            return speedIdle;
            case WALK_MODE:
            return speedWalk;
            case RUN_MODE:
            return speedRun;
            default:
            return -1;
        }
    }
    void print_pos_speed(std::string startStr = ""){
        std::cout << startStr << "pos: {" << posX << ", " << posY << "} with ";
        std::cout << "speed of " << get_speed() << " in direction of " << speedDir << " degrees\n";
    }
    void print_pos(std::string startStr = "", bool newLine = true){
        if(newLine) std::cout << startStr << "pos: {" << posX << ", " << posY << "}\n";
        else std::cout << startStr << "{" << posX << ", " << posY << "} ";
    }
    void print_forces(std::string startStr = "", bool newLine = true){
        if(newLine) std::cout << startStr << "Net Force: {" << forceX << ", " << forceY << "}\n";
        else std::cout << startStr << "{" << forceX << ", " << forceY << "} ";
    }
    void update_pos(int targetX, int targetY){
        posX = targetX;
        posY = targetY;
        enforce_valid_xyPos();
    }
    void increment_pos(int dX, int dY){
        posX += dX;
        posY += dY;
        enforce_valid_xyPos();
    }
    void correct_speedDir(){
        // Make speedDir granular because otherwise,
        //  if speed is 1 and direction is not exactly 0, 90, 180, or 270, it will NOT go anywhere!
        speedDir = speedDir % 360 - speedDir % 15; // degrees
    }
    void update_target_pos(){
        // Ignore external forces
        correct_speedDir();
        int tmp = get_speed();
        if(tmp == 0) return;
        increment_pos(tmp * cos_deg(speedDir), tmp * sin_deg(speedDir));
        enforce_valid_xyPos();
    }
    void update_forces(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // First, calculate which cells are nearby
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();

        // Check each nearby cell for any forces
        // TODO: apply an inward force for stickiness such that the force applied
        //  exactly matches the amount needed to snap the cells to the right distance from each other.
        // TODO: apply a force due to nonliving objects and walls, if applicable
        //for(auto pCell : pAlives){
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
                if(pCell == pSelf) continue;
                // Calculate the other cell's distance from pSelf (account for screen wrapping)
                int dX = pCell->posX - posX, dY = pCell->posY - posY;
                assert(WRAP_AROUND_X && WRAP_AROUND_Y);
                if (WRAP_AROUND_X) {
                    int dX2 = (ubX.val - abs(dX)) * -sign(dX); // Result has opposite sign vs dX
                    dX = (abs(dX) < abs(dX2) ? dX : dX2);
                }
                if (WRAP_AROUND_Y) {
                    int dY2 = (ubY.val - abs(dY)) * -sign(dY); // Result has opposite sign vs dX
                    dY = (abs(dY) < abs(dY2) ? dY : dY2);
                }
                int dist = sqrt(dX*dX + dY*dY) + 0.5;
                int targetDist = (pCell->dia + dia + 1) / 2; 
                if (dist < targetDist) {
                    // apply repulsive force based on the square of the differential distance
                    int forceMagnitude = 10*(targetDist - dist)*(targetDist - dist);
                    // Get the x and y components forceX and forceY
                    if (dist == 0) {
                        // Set the force direction randomly
                        int forceDirection = gen_uniform_int_dist(rng, 0, 359);
                        forceX += forceMagnitude * cos_deg(forceDirection);
                        forceY += forceMagnitude * sin_deg(forceDirection);
                    } else if (dX == 0 || dY == 0) {
                        int force_dX = forceMagnitude * -sign(dX); //(dX < 0 ? 1 : -1);
                        int force_dY = forceMagnitude * -sign(dY);
                        forceX += force_dX;
                        forceY += force_dY;
                    } else {
                        // dX != 0, dY != 0
                        float dY_div_dX = (dY / dX);
                        int force_dX = forceMagnitude / sqrt( 1 + dY_div_dX * dY_div_dX ) * -sign(dY); //(dX < 0 ? 1 : -1);
                        int force_dY = force_dX * dY_div_dX;
                        /*
                        float dX_div_dY = (dX / dY);
                        int force_dX = forceMagnitude / sqrt( 1 + dX_div_dY * dX_div_dY ) * -sign(dX); //(dX < 0 ? 1 : -1);
                        int force_dY = force_dX * dX_div_dY;
                        */
                        forceX += force_dX;
                        forceY += force_dY;
                    }
                }
            }
        }
        //enforce_valid_cell(false);
        return;
    }
    void apply_forces(){
        // Apply the forces which should already calculated
        increment_pos(forceX / forceDampingFactor.val, forceY / forceDampingFactor.val);
        forceX = 0;
        forceY = 0;
        //enforce_valid_cell(false);
    }
    bool calc_if_cell_is_dead(){
        // Check if cell should be killed
        //  NOTE: Due to how the program is structured, I have to actually kill each dead cell outside of this struct
        return (health <= 0 || energy <= 0 || doSelfDestruct);
    }
    int calc_direction_to_point(int targetX, int targetY){
        float distanceToTarget = calc_distance_from_point(targetX, targetY);
        int dx = targetX - posX, dy = targetY - posY;
        if(dx == 0 && dy == 0) return 0;
        if(dx == 0 && dy > 0) return 90;
        if(dx == 0 && dy < 0) return 270;
        int acuteAngle = ((int)arc_tan_deg(abs(dy), abs(dx)) + 360) % 360;
        if(dx > 0 && dy >= 0) return acuteAngle;
        if(dx > 0 && dy < 0) return (360 - acuteAngle) % 360;
        if(dx < 0 && dy >= 0) return 180 - acuteAngle;
        if(dx < 0 && dy < 0) return 180 + acuteAngle;
        cout << "WARNING: An angle between 0 and 359 should have been returned! Returning 0\n";
        return 0;
    }
    std::pair<int, int> calc_new_pos_given_target_speed_and_dir(int targetDirectionDeg, int targetSpeed){
        // Return the new x and y position travelled to
        int dx = targetSpeed * cos_deg(targetDirectionDeg);
        int dy = targetSpeed * sin_deg(targetDirectionDeg);
        return {dx, dy};
    }
    float calc_distance_from_point(int xCoord, int yCoord){
        // Assume -ubX.val < posX and -ubY.val < posY
        int xDist = xCoord - posX;
        if(WRAP_AROUND_X && abs(xDist) > abs(xCoord + ubX.val - posX)){
            xDist = xCoord + ubX.val - posX;
        }
        int yDist = yCoord - posY;
        if(WRAP_AROUND_Y && abs(yDist) > abs(yCoord + ubY.val - posY)){
            yDist = yCoord + ubY.val - posY;
        }
        float ans = std::sqrt(xDist * xDist + yDist * yDist);
        return ans;
    }
    void attack_cell(Cell* pAttacked){
        pAttacked->health -= attack;
        attackCooldown = maxAttackCooldown;
        energy -= energyCostPerUse["attack"];
        enforce_valid_cell(false);
    }
    void apply_non_movement_decisions(std::vector<Cell*>& pAlives, std::vector<Cell*>& pCellsHist,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        if(doAttack && attackCooldown == 0 && energy > energyCostPerUse["attack"]){
            // Find out which regions neighbor the cell's region
            std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
            
            // Damage all cells that this cell touches excluding the cell itself
            // If health < 0, the cell will die when the death conditions are checked
            for(auto reg : neighboringRegions){
                for(auto pCell : pAlivesRegions[reg]){
                    if(uniqueCellNum == pCell->uniqueCellNum) continue;
                    if(calc_distance_from_point(pCell->posX, pCell->posY) <= (float)(dia + pCell->dia + 0.1) / 2){
                        attack_cell(pCell);
                    }
                }
            }
        }
        // TODO: ensure there is surplus energy beyond energyCostToClone
        if(doCloning && energy > 1.2*energyCostToClone && pAlives.size() < cellLimit.val){
            Cell* pCell = clone_self(pCellsHist.size(), cloningDirection); // A perfect clone of pSelf
            pCell->mutate_stats();
            pCellsHist.push_back(pCell);
            pAlives.push_back(pCell);
        }
        enforce_valid_cell(false);
    }
    // Generate random-ish movement that looks reasonable for a cell to do
    void preplan_random_cell_activity(int pctChanceToChangeDir, int pctChanceToChangeSpeed, int numFrames,
    bool _enableAttack, bool _enableCloning){
        int newSpeedDir = pSelf->speedDir, newSpeedMode = pSelf->speedMode;
        int numFramesSinceLastDecision = 0;
        bool changedSpeedDir = false;
        while(numFrames > 0){
            //cout << "-";
            if(rand() % 100 < pctChanceToChangeDir){
                while(newSpeedDir == pSelf->speedDir) newSpeedDir = rand() % 360;
                changedSpeedDir = true;
            }
            if(rand() % 100 < pctChanceToChangeSpeed){
                while(newSpeedMode == pSelf->speedMode) newSpeedMode = rand() % 3;
                changedSpeedDir = true;
            }
            if(changedSpeedDir || numFramesSinceLastDecision >= numFrames){
                changedSpeedDir = false;
                pSelf->force_decision(numFramesSinceLastDecision, newSpeedDir, rand() % 360, newSpeedMode, _enableAttack, false, _enableCloning);
                numFrames -= numFramesSinceLastDecision;
                numFramesSinceLastDecision = 0;
            } else {
                numFramesSinceLastDecision++;
            }
        }
    }
    int get_optimal_speedDir_to_point(int targetX, int targetY){
        int dx = targetX - posX, dy = targetY - posY;
        float distance = calc_distance_from_point(targetX, targetY);
        if(distance == 0) return 0;
        int ans = (int)arc_cos_deg(abs(dx), distance);
        if(dx >= 0){
            if(dy >= 0) ans = ans;
            ans = 360 - ans;
        } else {
            if(dy >= 0) ans = 180 - ans;
            ans = 180 + ans;
        }
        // If speed is low, we may only be able to travel in certain directions
        if(speedWalk > speedRun) cout << "WARNING: Walk speed exceeds run speed!";
        switch(speedRun){
            case 0:
            return 0;
            case 1:
            return find_closest_value(ans, {0,90,180,270,360}) % 360;
            case 2:
            return find_closest_value(ans, {0,30,45,60, 90,120,135,150, 180,210,225,240, 270,300,315,330,360}) % 360;
            default:
            return ans; // The final speedDir may always be rounded down to the nearest 15 degrees
        }
    }
    void preplan_shortest_path_to_point(int x0, int y0, int targetX, int targetY, bool enableRunning,
    bool _enableAttack, bool _enableCloning){
        int targetDx = targetX - x0, targetDy = targetY - y0;
        int optimalDir = 0;
        float optimalDistance = abs(targetDx) + abs(targetDy) + 1;
        int speed = enableRunning*pSelf->speedRun + !enableRunning*pSelf->speedWalk;
        int nextPosX = x0, nextPosY = y0;
        for(int testDir = 0; testDir < 360; testDir += 15){
            int testDx = speed * cos_deg(testDir);
            int testDy = speed * sin_deg(testDir);
            float testDistance = calc_distance_between_points(x0 + testDx, y0 + testDy, targetX, targetY);
            if(testDistance < optimalDistance){
                optimalDistance = testDistance;
                optimalDir = testDir;
                nextPosX = x0 + testDx; nextPosY = y0 + testDy;
            }
        }
        int _speedMode = (enableRunning ? RUN_MODE : WALK_MODE);
        float targetDistance = calc_distance_between_points(x0, y0, targetX, targetY);
        float newTargetDistance = calc_distance_between_points(nextPosX, nextPosY, targetX, targetY);
        if(newTargetDistance >= targetDistance){
            if(!enableRunning) return;
            preplan_shortest_path_to_point(nextPosX, nextPosY, targetX, targetY, false, _enableAttack, _enableCloning);
            return;
        }
        force_decision(1, optimalDir, rand() % 360, _speedMode, _enableAttack, false, _enableCloning);
        preplan_shortest_path_to_point(nextPosX, nextPosY, targetX, targetY, enableRunning, _enableAttack, _enableCloning);
    }
    void chase_optimal_cell(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions, std::vector<Cell*>& pNearestCells,
    bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        clear_forced_decisions();
        // First, mark each cell with a number and record the smallest (best) number
        std::vector<int> cellRanks;
        for(auto pCell : pNearestCells){
            int rank = 0;
            int speedCoef = 1, dirCoef = 1, distanceCoef = 0, doAttackCoef = 0;
            int predatorCoef = 10000, balancedCoef = 10000, gndCoef = 5000, plantCoef = 2500;

            int xNext = pCell->posX + pCell->get_speed()*cos_deg(pCell->speedDir);
            int yNext = pCell->posY + pCell->get_speed()*sin_deg(pCell->speedDir);
            int distance = calc_distance_from_point(xNext, yNext);
            int optimalSpeedDir = get_optimal_speedDir_to_point(xNext, yNext);
            int speedDirDiff = min_int(abs(optimalSpeedDir - speedDir), abs(optimalSpeedDir - speedDir - 360));

            // Update Rank
            rank += speedCoef * pCell->get_speed();
            rank += distanceCoef * distance;
            rank += dirCoef * speedDirDiff;
            rank += doAttackCoef * pCell->doAttack;
            rank += predatorCoef * (pCell->EAM[EAM_CELLS] == 100);
            rank += balancedCoef * (pCell->EAM[EAM_SUN] < 100 && pCell->EAM[EAM_GND] < 100 && pCell->EAM[EAM_CELLS] < 100);
            rank += gndCoef * (pCell->EAM[EAM_GND] == 100);
            rank += plantCoef * (pCell->EAM[EAM_SUN] == 100);
            cellRanks.push_back(rank);
        }
        int minRank = cellRanks[0];
        for(auto rank : cellRanks) minRank = min_int(minRank, rank);
        for(int i = pNearestCells.size()-1; i >= 0; i--){
            if(cellRanks[i] != minRank) pNearestCells.erase(pNearestCells.begin() + i);
        }
        // Get the optimal speedDir and speed for the first cell in the list
        Cell* pCellToChase = pNearestCells[0];
        int xNext = pCellToChase->posX + pCellToChase->get_speed()*cos_deg(pCellToChase->speedDir);
        int yNext = pCellToChase->posY + pCellToChase->get_speed()*sin_deg(pCellToChase->speedDir);
        int optimalSpeedDir = get_optimal_speedDir_to_point(xNext, yNext);
        // Pursue the relevant cell for 1 more frame
        int _speedDir = optimalSpeedDir;
        int targetDistance = calc_distance_from_point(pNearestCells[0]->posX, pNearestCells[0]->posY);
        int targetSpeed = find_closest_value(targetDistance, {speedIdle, speedWalk, speedRun});
        int _speedMode;
        if(targetSpeed == speedIdle) _speedMode = speedIdle;
        if(targetSpeed == speedWalk) _speedMode = speedWalk;
        if(targetSpeed == speedRun)  _speedMode = speedRun;
        force_decision(1, _speedDir, rand() % 360, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
    }
    std::vector<int> findWeighting(int numSlots, int* arr, int arrSize){
        int sum = 0;
        for(int i = 0; i < arrSize; i++) sum += arr[i];
        std::vector<int> weights(arrSize);
        std::vector<int> remainders(arrSize);
        for(int i = 0; i < arrSize; i++){
            weights[i] = numSlots * arr[i] / sum;
            remainders[i] = numSlots * arr[i] % sum;
        }
        // Calculate the amount which needs to be added to the weights
        int remaining = numSlots;
        for(auto num : weights) remaining -= num;
        // Add 1 to the indices with the largest remainder until sum(weights) == numSlots
        //  (or remaining == 0)
        while(remaining > 0){
            int mostErroneousIndex = 0;
            for(int i = 0; i < arrSize; i++){
                if(remainders[i] > remainders[mostErroneousIndex]){
                    mostErroneousIndex = i;
                }
            }
            weights[mostErroneousIndex]++;
            remaining--;
            remainders[mostErroneousIndex] -= sum;
        }
        //cout << "weights: "; for(auto num: weights) cout << num << ", "; cout << endl;
        //cout << "EAM:     " << EAM[0] << ", " << EAM[1] << ", " << EAM[2] << endl;
        return weights;
    }
    std::vector<SDL_Texture*> findEAMTex(){
        std::vector<SDL_Texture*> ans;
        // First, check if everything is balanced
        int minEAM = EAM[EAM_SUN], maxEAM = EAM[EAM_SUN];
        for(int i = 0; i < NUM_EAM_ELE; i++){
            if(EAM[i] < minEAM) minEAM = EAM[i];
            if(EAM[i] > maxEAM) maxEAM = EAM[i];
        }
        if(maxEAM <= 2*minEAM){
            ans.push_back(P_EAM_TEX["balanced"]);
            return ans;
        }
        int numPixelsInEAMTex = 4;
        int EAMPerPixel = REQ_EAM_SUM / numPixelsInEAMTex;
        if(EAM[EAM_GND] < EAMPerPixel) ans.push_back(P_EAM_TEX["balanced"]);
        else ans.push_back(P_EAM_TEX["g4"]); // Ground (or balanced) will fill in the empty spaces
        // NOTE: there are 4 pixels to color in
        // We only have to worry about the sun and predation textures, since the remaining
        //  is already taken care of
        if(EAM[EAM_SUN] / EAMPerPixel > 0){
            std::string nextFile = "s" + std::to_string(EAM[EAM_SUN] / EAMPerPixel);
            ans.push_back(P_EAM_TEX[nextFile]);
        }
        if(EAM[EAM_CELLS] / EAMPerPixel > 0){
            std::string nextFile = "c" + std::to_string(EAM[EAM_CELLS] / EAMPerPixel);
            ans.push_back(P_EAM_TEX[nextFile]);
        }
        return ans;
    }
    void draw_cell(){
        // TODO: After the video, draw the cells so they are centered on their center pixel
        // TODO: If part of a cell is not fully rendered, render a copy of it on the other side
        int drawX = drawScaleFactor*(posX - dia/2);
        int drawY = drawScaleFactor*(posY - dia/2);
        int drawSize = drawScaleFactor*dia;
        draw_texture(pCellSkeleton, drawX, drawY, drawSize, drawSize);
        // Draw the health and energy on top of this
        SDL_Texture* energyTex = findSDLTex(energy * 100 / maxEnergy, P_CELL_ENERGY_TEX);
        draw_texture(energyTex, drawX, drawY, drawSize, drawSize);
        SDL_Texture* healthTex = findSDLTex(100*health/maxHealth, P_CELL_HEALTH_TEX);
        draw_texture(healthTex, drawX, drawY, drawSize, drawSize);
        if(doAttack && attack > 0)  draw_texture(pDoAttackTex,  drawX, drawY, drawSize, drawSize);
        if(doCloning) draw_texture(pDoCloningTex, drawX, drawY, drawSize, drawSize);
        //std::vector<int> EAM_weights = findWeighting(4, EAM, NUM_EAM_ELE);
        std::vector<SDL_Texture*> EAM_Tex = findEAMTex();
        for(auto tex : EAM_Tex) draw_texture(tex, drawX, drawY, drawSize, drawSize);
    }
};












struct DeadCell {
    // Identity
    DeadCell* pSelf = NULL; // Place a pointer to self here
    //  NOTE: The user MUST define the pointer after placing it into a vector to be permanently kept as data.
    //  Otherwise, it will be the wrong pointer.
    Cell* pOldSelf = NULL; // This cell when it was alive

    // Internal timers
    int timeSinceDead = -1; // Relative to birth (limits lifespan)

    // Dependent (calculated) variables (must be updated
    //  if any of their dependent variables are updated)
    int posX = -1, posY = -1;
    std::pair<int, int> xyRegion = {0, 0};
    int energy = -1; // Energy which can be distributed to the cells that consume it
    //  (or to the ground if time ticks long enough)
    int dia = -1; // Diameter
    int decayRate = -1;     // The percentage of energy drained to the ground each decay period.
    int decayPeriod = -1;   // This many frames pass between the dead cell giving some energy to the ground.

    // Struct-specific methods
    void doNothing(){
        return;
    }
    void kill_cell(Cell* pAlive, DeadCell* pDead, int i_pAlive) {
        //DeadCell* pDead = new DeadCell;
        pSelf = pDead;
        pOldSelf = pAlive;
        decayPeriod = 20;
        decayRate = 5;
        dia = pAlive->dia;
        energy = pAlive->energy + pAlive->energyCostToClone;
        timeSinceDead = 1;
        posX = pAlive->posX;
        posY = pAlive->posY;
    }
    // NOTE: this function does NOT have access to the entire list of cells,
    //  so a separate function needs to be run to organize the cells into
    //  region-based lists
    void assign_self_to_xyRegion(){
        xyRegion.first = saturate_int(posX / CELL_REGION_SIDE_LEN, 0, cellRegionNumUbX-1);
        xyRegion.second = saturate_int(posY / CELL_REGION_SIDE_LEN, 0, cellRegionNumUbY-1);
    }
    std::vector<std::pair<int, int>> get_neighboring_xyRegions(){
        int xReg = xyRegion.first, yReg = xyRegion.second;
        std::vector<std::pair<int, int>> neighboringRegions = {
            xyRegion,
            {xReg-1, yReg}, {xReg+1, yReg}, {xReg, yReg-1}, {xReg, yReg+1},
            {xReg-1, yReg-1}, {xReg-1, yReg+1}, {xReg+1, yReg-1}, {xReg+1, yReg+1}
        };
        for(int i = 0; i < neighboringRegions.size(); i++){
            while(neighboringRegions[i].first < 0) neighboringRegions[i].first += cellRegionNumUbX;
            while(neighboringRegions[i].second < 0) neighboringRegions[i].second += cellRegionNumUbY;
            neighboringRegions[i].first %= cellRegionNumUbX;
            neighboringRegions[i].second %= cellRegionNumUbY;
        }
        return neighboringRegions;
    }
    void enforce_wrap_around_x(){
        while(posX < 0) posX += ubX.val;
        posX %= ubX.val;
        assign_self_to_xyRegion();
    }
    void enforce_wrap_around_y(){
        while(posY < 0) posY += ubY.val;
        posY %= ubY.val;
        assign_self_to_xyRegion();
    }
    void enforce_valid_xyPos(){
        if(WRAP_AROUND_X) enforce_wrap_around_x();
        else posX = saturate_int(posX, 0, ubX.val);
        if(WRAP_AROUND_Y) enforce_wrap_around_y();
        else saturate_int(posY, 0, ubY.val);
        assign_self_to_xyRegion();
    }
    void enforce_valid_cell(){
        saturate_int(decayRate, 0, 100);
        if(decayPeriod <= 0) decayPeriod = 1;
        enforce_valid_xyPos();
    }
    void set_int_stats(std::map<std::string, int>& varVals){
        // Only contains functionality for the more important stats
        int lenVarVals = 0;
        if(varVals.count("timeSinceDead"))  {lenVarVals++; timeSinceDead = varVals["timeSinceDead"];}
        if(varVals.count("dia"))            {lenVarVals++; dia = varVals["dia"];}
        if(varVals.count("energy"))         {lenVarVals++; energy = varVals["energy"];}
        if(varVals.count("posX"))           {lenVarVals++; posX = varVals["posX"];}
        if(varVals.count("posY"))           {lenVarVals++; posY = varVals["posY"];}
        if(varVals.count("decayRate"))      {lenVarVals++; decayRate = varVals["decayRate"];}
        if(varVals.count("decayPeriod"))    {lenVarVals++; decayPeriod = varVals["decayPeriod"];}

        // Ensure that varVals does NOT contain values not accounted for in this function
        assert(lenVarVals == varVals.size());
        
        // Ensure we update all dependent variables
        enforce_valid_cell();
    }
    void update_timers(){
        timeSinceDead++;
    }
    void decide_next_frame(){
        update_timers();
    }
    void define_self(DeadCell* pCell){
        // NOTE: If I push a copy of the cell into a new location, I need to update self
        pSelf = pCell;
    }
    void gen_stats_random(DeadCell* pDead = NULL){
        // Random generation from scratch
        pSelf = pDead;
        pOldSelf = NULL;
        pDead->decayPeriod = 20;
        pDead->decayRate = 5;
        pDead->dia = 2;
        pDead->energy = 1000;
        pDead->timeSinceDead = 1;
        pDead->randomize_pos(0, ubX.val, 0, ubY.val);
    }
    std::vector<Cell*> find_touching_cells(std::vector<Cell*>& pAlives,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        std::set<int> checkedCells;
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
                if(checkedCells.count(pCell->uniqueCellNum)) continue;
                if(pCell->calc_distance_from_point(posX, posY) > (float)(dia + pCell->dia + 0.1) / 2) continue;
                ans.push_back(pCell);
                checkedCells.insert(pCell->uniqueCellNum);
            }
        }
        return ans;
    }
    // The dead cells and ground transfer energy to the living cells and / or the environment
    void do_energy_decay(std::vector<Cell*>& pAlives,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Energy to cells which are touching the dead cell
        std::vector<Cell*> touchingCells = find_touching_cells(pAlives, pAlivesRegions);
        int rmEnergy = 0; // Energy to give to other cells
        std::vector<int> energyWeight(touchingCells.size());
        for(int i = 0; i < energyWeight.size(); i++){
            energyWeight[i] = touchingCells[i]->EAM[EAM_CELLS] * energy / 1000;
            rmEnergy += energyWeight[i];
        }
        if (rmEnergy > energy) {
            float multiplyBy = (float)energy / (float)rmEnergy;
            for(int i = 0; i < touchingCells.size(); i++){
                Cell* pCell = touchingCells[i];
                pCell->energy += multiplyBy * energyWeight[i] * pCell->EAM[EAM_CELLS] / 100;
            }
            energy = 0;
            return;
        }
        for(int i = 0; i < touchingCells.size(); i++) {
            Cell* pCell = touchingCells[i];
            pCell->energy += energyWeight[i] * pCell->EAM[EAM_CELLS] / 100;
        }
        energy -= rmEnergy;

        // Energy to ground
        rmEnergy = 0;
        if(timeSinceDead % decayPeriod) rmEnergy = decayRate * energy / 100 + 10;
        energy -= rmEnergy;

        enforce_valid_cell();
    }
    void randomize_pos(int _lbX, int _ubX, int _lbY, int _ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, _lbX, _ubX);
        posY = gen_uniform_int_dist(rng, _lbY, _ubY);
        enforce_valid_xyPos();
    }
    void print_pos(std::string startStr = "", bool newLine = true){
        if(newLine) std::cout << startStr << "pos: {" << posX << ", " << posY << "}\n";
        else std::cout << startStr << "{" << posX << ", " << posY << "} ";
    }
    void print_main_stats(){
        print_pos("  ");
        std::cout << "  dia: " << dia << std::endl;
    }
    void update_pos(int targetX, int targetY){
        posX = targetX;
        posY = targetY;
        enforce_valid_xyPos();
    }
    void increment_pos(int dX, int dY){
        posX += dX;
        posY += dY;
        enforce_valid_xyPos();
    }
    void remove_this_dead_cell_if_depleted(std::vector<DeadCell*>& pDeads, int iDead){
        assert(pDeads[iDead] == pSelf);
        if(energy > 0) return;
        pDeads.erase(pDeads.begin() + iDead);
        delete pSelf;
    }
    void draw_cell(){
        int drawX = drawScaleFactor*(posX - dia/2);
        int drawY = drawScaleFactor*(posY - dia/2);
        int drawSize = drawScaleFactor*dia;
        draw_texture(pDeadCellTex, drawX, drawY, drawSize, drawSize);
    }
};





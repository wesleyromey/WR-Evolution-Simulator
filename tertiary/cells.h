#ifndef SECONDARY_INCLUDES_H
#include "../secondary/secondaryIncludes.h"
#define SECONDARY_INCLUDES_H
#endif


int count_all_alive_cells(std::vector<Cell*> pActives);

// The main (possibly only) living organisms in the simulator. Their shape will be a circle
struct Cell {
    // Identity
    Cell* pSelf = NULL; // Place a pointer to self here
    //  NOTE: The user MUST define the pointer after placing it into a vector to be permanently kept as data.
    //  Otherwise, it will be the wrong pointer.
    Cell* pParent = NULL;
    bool id[ID_LEN]; // An identifier which every creature "knows"
    //  which may change through random mutations, but ultimately
    //  does NOT change for an individual after birth
    int uniqueCellNum = -1;
    //  This is unique because pCellsHist never gets deleted from until the simulation ends

    // Specific to a dead cell
    int timeSinceDead = -1;
    int decayRate = -1;
    int decayPeriod = -1;

    // Decisions (i.e. inputs)
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
    
    // State
    int health = -1; // When this reaches 0, the cell dies
    // Creature dies if energy <= 0.
    //  Needed to do various abilities or keep oneself alive.
    int energy = -1;

    // Physics, position, etc.
    int posX = 0, posY = 0;
    std::pair<int, int> xyRegion = {0, 0}; // Each cell should be placed in their appropriate 'bin' of nearby cells
    // Negative values move the cell in the opposite direction
    //  Force moves the cell in the same direction as the force, if there is enough of it
    int forceX = 0, forceY = 0;


    // Dependent (calculated) variables (must be updated
    //  if any of the variables they depend on are updated)
    int size = -1; // Area (truncated at the decimal); Calculated from dia
    std::map<std::string, int> energyCostToCloneMap;
    int energyCostToClone = 0;
    std::map<std::string, int> energyCostPerUse;
    std::map<std::string, int> energyCostPerSecMap;
    int energyCostPerFrame = 0;
    bool isAlive = true;


    // Stats
    std::map<std::string, std::vector<int>> stats;
    bool drawVisionRadius = false;

    // Constructor
    Cell(){}

    // Struct-specific methods
    void update_stat(std::string statName, int mask, int val, int lb, int ub, int mutationPct1kChance, int mutationMaxPct1kChange){
        // Default: mask == 0x1F
        if(mask & 0x01) stats[statName][0] = val;
        if(mask & 0x02) stats[statName][1] = lb;
        if(mask & 0x04) stats[statName][2] = ub;
        if(mask & 0x08) stats[statName][3] = mutationPct1kChance;
        if(mask & 0x10) stats[statName][4] = mutationMaxPct1kChange;
    }
    void mutate_stat(std::string statName){
        int mean = stats[statName][0];
        int lb = stats[statName][1], ub = stats[statName][2];
        int maxMutationAmt = (int)((long long)stats[statName][4] * (long long)stats[statName][0] / 1000);
        stats[statName][0] = gen_uniform_int_dist(rng, mean - maxMutationAmt, mean + maxMutationAmt);
    }
    void mutate_stats(){
        // Random mutation based on parent's mutation rate
        for(auto item : stats){
            std::string statName = item.first;
            int pct1kChanceOfMutation = stats[statName][3]; // Probability of mutation
            if(rand() % 1000 < pct1kChanceOfMutation) mutate_stat(statName);
        }
        enforce_valid_cell(true);
    }
    void gen_stats_random(int _cellType, std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*>& pCellsHist){
        // Random generation from scratch
        assert(pSelf != NULL);

        #define stat_init(lb, ub) gen_uniform_int_dist(rng, lb, ub)
        #define mutChance defaultMutationChance.val
        #define mutAmt defaultMutationAmt.val
        // Initialize all stats
        // Notation: Pct1k == one thousandth of the entire value
        // {{"stat1", {val, lb, ub, mutationPct1kChance, mutationMaxPct1kChange}}, ...}
        //  Whenever a mutation occurs, the stat must be able to change by at least 1 (unless lb == ub)
        stats["attack"]         = { stat_init(1,   1),     0, 10000, mutChance, mutAmt}; // (0,3)
        stats["dex"]            = {                 0,     0,     0,         0,      0}; // TODD: Add this as an actual stat
        stats["dia"]            = { stat_init(2,   2),     1,    10, mutChance, mutAmt}; // (2,10)
        stats["EAM_SUN"]        = { stat_init(0, 100),     0,   100,         0,      0}; // (0,100)
        stats["EAM_GND"]        = { stat_init(0, 100),     0,   100,         0,      0}; // (0,100)
        stats["EAM_CELLS"]      = { stat_init(0, 100),     0,   100,         0,      0}; // (0,100)
        stats["initEnergy"]     = {              1000,   500,  2000, mutChance, mutAmt}; // (0,10000)
        stats["maxAtkCooldown"] = {                10,    10,    10,         0,      0}; // 10
        stats["maxEnergy"]      = { 5000*stats["dia"][0],  1, 90000,         0,      0}; // 5000*stats["dia"]
        stats["maxHealth"]      = { stat_init(1,   1),     1, 10000, mutChance, mutAmt}; // (1,10)
        // TODO: Add stats for the AI and its relevant mutation rate
        stats["mutationRate"]   = { stat_init(0,   0),     0,  1000,         0,      0}; // (0,1000)
        // TODO: change speedIdle, speedWalk, and speedRun to a "maxSpeed" stat and change speed to a continuously varying decision
        stats["speedIdle"]      = {                 0,     0,     0,         0,      0}; // 0
        stats["speedWalk"]      = {                 1,     0,    10, mutChance, mutAmt}; // (0, 1)
        stats["speedRun"]       = {                 2,     0,   100, mutChance, mutAmt}; // (0, 100)
        stats["visionDist"]     = { stat_init(5,   5),     0,  1000,         0,      0}; // (0, 10)
        stats["rngAi_pctChanceIdle"] = {           33,     0,   100,         0,      0};
        stats["rngAi_pctChanceWalk"] = {           33,     0,   100,         0,      0};
        stats["rngAi_pctChanceToChangeDir"] = {     5,     0,   100,         0,      0};
        stats["rngAi_pctChanceToChangeSpeed"] = {   5,     0,   100,         0,      0};
        switch(_cellType){
            case CELL_TYPE_PLANT:
            update_stat("attack"    , 0x1F, 0, 0, 0, 0, 0);
            update_stat("EAM_SUN"   , 0x1F, 100, 100, 100, 0, 0);
            update_stat("EAM_GND"   , 0x1F, 0, 0, 0, 0, 0);
            update_stat("EAM_CELLS" , 0x1F, 0, 0, 0, 0, 0);
            update_stat("speedWalk" , 0x1F, 0, 0, 0, 0, 0);
            update_stat("speedRun"  , 0x1F, 0, 0, 0, 0, 0);
            update_stat("visionDist", 0x1F, 0, 0, 0, 0, 0);
            break;
            case CELL_TYPE_WORM:
            update_stat("attack"    , 0x1F, 0, 0, 0, 0, 0);
            update_stat("EAM_SUN"   , 0x1F, 0, 0, 0, 0, 0);
            update_stat("EAM_GND"   , 0x1F, 100, 100, 100, 0, 0);
            update_stat("EAM_CELLS" , 0x1F, 0, 0, 0, 0, 0);
            update_stat("visionDist", 0x1F, 0, 0, 0, 0, 0);
            break;
            case CELL_TYPE_PREDATOR:
            update_stat("EAM_SUN"   , 0x1F, 0, 0, 0, 0, 0);
            update_stat("EAM_GND"   , 0x1F, 0, 0, 0, 0, 0);
            update_stat("EAM_CELLS" , 0x1F, 100, 100, 100, 0, 0);
            break;
            case CELL_TYPE_MUTANT:
            update_stat("EAM_SUN"   , 0x1F, 33, 33, 33, 0, 0);
            update_stat("EAM_GND"   , 0x1F, 34, 34, 34, 0, 0);
            update_stat("EAM_CELLS" , 0x1F, 33, 33, 33, 0, 0);
            break;
            case CELL_TYPE_GENERIC:
            break;
        }
        initialize_cell(pActivesRegions, pCellsHist);
        enforce_valid_cell(true);
        #undef stat_init
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
    void print_stats(int updateMask = 0x1F, std::set<std::string> statsToShow = {}){
        print_id();
        print_pos_speed("  ");
        for(auto item : stats){
            std::string statName = item.first;
            if(statsToShow.size() > 0 && !statsToShow.count(statName)) continue;
            print_stat(statName, updateMask);
        }
    }
    void set_int_stats(std::map<std::string, int>& varVals, int aiPreset = -1, bool _enableMutations = false,
    bool expandBounds = false){
        // TODO: Include the ability to set the aiNetwork and nodesPerLayer
        // Only contains functionality for the more important stats
        int lenVarVals = 0;
        for(auto item : stats){
            std::string statName = item.first;
            if(varVals.count(statName)){
                lenVarVals++;
                stats[statName][0] = varVals[statName];
                if(expandBounds){
                    stats[statName][1] = 0;
                    stats[statName][2] = INT_MAX;
                }
                if(!_enableMutations){
                    stats[statName][3] = 0;
                }
            }
        }
        if(varVals.count("age"))            {lenVarVals++; age = varVals["age"];}
        if(varVals.count("attackCooldown")) {lenVarVals++; attackCooldown = varVals["attackCooldown"];}
        if(varVals.count("energy"))         {lenVarVals++; energy = varVals["energy"];}
        if(varVals.count("health"))         {lenVarVals++; health = varVals["health"];}
        if(varVals.count("posX"))           {lenVarVals++; posX = varVals["posX"];}
        if(varVals.count("posY"))           {lenVarVals++; posY = varVals["posY"];}

        // AI weights
        if(aiPreset >= 0) apply_ai_preset(aiPreset);

        // Ensure that varVals does NOT contain values not accounted for in this function
        if(lenVarVals != varVals.size()) print_scalar_vals("lenVarVals: ", lenVarVals, "varVals.size()", varVals.size());
        assert(lenVarVals == varVals.size());
        
        // Ensure we update all dependent variables
        enforce_valid_cell(true);
    }
    // Count alive cells only!
    std::vector<Cell*> find_touching_cells(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        std::set<int> checkedCells = {uniqueCellNum};
        for(auto reg : neighboringRegions){
            for(auto pCell : pActivesRegions[reg]){
                if(pCell->isAlive == false) continue;
                if(checkedCells.count(pCell->uniqueCellNum)) continue;
                if(pCell->calc_distance_from_point(posX, posY) > (float)(stats["dia"][0] + pCell->stats["dia"][0] + 0.1) / 2) continue;
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
    // Format: xRegLb, xRegUb, yRegLb, yRegUb
    std::tuple<int, int, int, int> get_xyRegion_neighborhoodBounds(int radiusInRegions){
        int xReg = xyRegion.first, yReg = xyRegion.second;
        int xRegLb = (xReg - radiusInRegions + cellRegionNumUbX) % cellRegionNumUbX;
        int xRegUb = (xReg + radiusInRegions) % cellRegionNumUbX;
        int yRegLb = (yReg - radiusInRegions + cellRegionNumUbY) % cellRegionNumUbY;
        int yRegUb = (yReg + radiusInRegions) % cellRegionNumUbY;
        if(cellRegionNumUbX / 2 <= radiusInRegions){ xRegLb = 0; xRegUb = cellRegionNumUbX - 1; }
        if(cellRegionNumUbY / 2 <= radiusInRegions){ yRegLb = 0; yRegUb = cellRegionNumUbY - 1; }
        return {xRegLb, xRegUb, yRegLb, yRegUb};
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
        stats["maxEnergy"][0] = 5000*size;
    }
    void update_size(){
        size = PI*stats["dia"][0]*stats["dia"][0]/4 + 0.5; // size is an int
        //if(automateEnergy) update_max_energy();
    }
    int calc_EAM_sum(){
        int EAM_sum = 0;
        EAM_sum += stats["EAM_SUN"][0];
        EAM_sum += stats["EAM_GND"][0];
        EAM_sum += stats["EAM_CELLS"][0];
        return EAM_sum;
    }
    void enforce_EAM_constraints(){
        // Enforce the EAM constraints such that all elements >= 0
        //  and they add to 100
        stats["EAM_SUN"][0] = max_int(stats["EAM_SUN"][0], 0);
        stats["EAM_GND"][0] = max_int(stats["EAM_GND"][0], 0);
        stats["EAM_CELLS"][0] = max_int(stats["EAM_CELLS"][0], 0);
        // Ensure that EAM_sum == EAM_SUM as defined in this struct
        int EAM_sum = calc_EAM_sum();
        if(EAM_sum != REQ_EAM_SUM) {
            stats["EAM_SUN"][0] = stats["EAM_SUN"][0] * REQ_EAM_SUM / EAM_sum;
            stats["EAM_GND"][0] = stats["EAM_GND"][0] * REQ_EAM_SUM / EAM_sum;
            stats["EAM_CELLS"][0] = stats["EAM_CELLS"][0] * REQ_EAM_SUM / EAM_sum;
        }
        EAM_sum = calc_EAM_sum();
        int increment = sign(REQ_EAM_SUM - calc_EAM_sum());
        while(calc_EAM_sum() != REQ_EAM_SUM){
            switch(rand() % NUM_EAM_ELE){
                case 0:
                stats["EAM_SUN"][0] += increment;
                stats["EAM_SUN"][0] = saturate_int(stats["EAM_SUN"][0], stats["EAM_SUN"][1], stats["EAM_SUN"][2]);
                break;
                case 1:
                stats["EAM_GND"][0] += increment;
                stats["EAM_GND"][0] = saturate_int(stats["EAM_GND"][0], stats["EAM_GND"][1], stats["EAM_GND"][2]);
                break;
                case 2:
                stats["EAM_CELLS"][0] += increment;
                stats["EAM_CELLS"][0] = saturate_int(stats["EAM_CELLS"][0], stats["EAM_CELLS"][1], stats["EAM_CELLS"][2]);
                break;
            }
        }
        assert(calc_EAM_sum() == REQ_EAM_SUM);
    }
    void enforce_bounds(int& val, int lb, int ub){
        assert(lb <= ub);
        val = saturate_int(val, lb, ub);
    }
    // Only consider the nearest cells within the cell's field of view
    //  Return the cell ids starting with the cells most relevant to predators
    //  Used to return std::vector<Cell*>
    std::vector<int> get_nearest_cell_ids(int maxNumCellsToReturn,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*> pCellsHist){
        // visionDist: The distance the cell can see
        int xReg = xyRegion.first, yReg = xyRegion.second;
        
        // A region counts as too far if it its nearest point is out of range
        //      NOTE: Assume that a cell's diameter is smaller than the size of each cell region
        //      NOTE: Also, some regions may be larger than CELL_REGION_SIDE_LEN
        float maxDetectableCellDistance = (float)stats["visionDist"][0] + CELL_REGION_SIDE_LEN; //(float)stats["dia"][2] / 2;
        int visionDist_NumReg = maxDetectableCellDistance / CELL_REGION_SIDE_LEN;

        // Get the rectangular box in which all relevant regions appear
        std::tuple<int, int, int, int> xyRegionNeighborhoodBounds = get_xyRegion_neighborhoodBounds(visionDist_NumReg);
        int xRegLb = std::get<0>(xyRegionNeighborhoodBounds);
        int xRegUb = std::get<1>(xyRegionNeighborhoodBounds);
        int yRegLb = std::get<2>(xyRegionNeighborhoodBounds);
        int yRegUb = std::get<3>(xyRegionNeighborhoodBounds);
        //if(stats["EAM_CELLS"][0] == 100) cout << endl;
        //if(stats["EAM_CELLS"][0] == 100) print_scalar_vals("xRegLb", xRegLb, "xRegUb", xRegUb, "yRegLb", yRegLb, "yRegUb", yRegUb, "radiusInRegions", visionDist_NumReg);
        
        // Go through all the regions within visionDist from the current cell
        #define add_nearby_cells_to_distance_map(pCellVec){ \
            for(auto pCell : pCellVec){ \
                if(pCell == pSelf) continue; \
                int cellId = pCell->uniqueCellNum; \
                if(nearbyCellDistances.count(cellId) > 0) continue; \
                float distXY = calc_distance_from_point(pCell->posX, pCell->posY); \
                float distToTravel = max_float(distXY - (float)stats["dia"][0]/2 - (float)pCell->stats["dia"][0]/2, 0); \
                float effectiveVisionRadius = stats["visionDist"][0] + (float)pCell->stats["dia"][0]/2; \
                if(false) print_scalar_vals("cellId", cellId, "posX", pCell->posX, "posY", pCell->posY, "distXY", distXY, "distToTravel", distToTravel, "effectiveVisionRadius", effectiveVisionRadius); \
                if(distXY <= effectiveVisionRadius) nearbyCellDistances[cellId] = distToTravel; \
                if(false && distXY <= effectiveVisionRadius) cout << "  added cellId " << cellId << " to the list of nearby cell distances\n"; \
            } \
        }
        #define increment_iX_or_iY(iX, iY){ \
            iX++; \
            if(iX == xRegUb + 1){ \
                iX = xRegLb; iY++; \
                if(iY == yRegUb + 1) break; \
            } \
            iX %= cellRegionNumUbX; \
            iY %= cellRegionNumUbY; \
        }
        std::map<int, float> nearbyCellDistances;
        //xRegLb = 0; yRegLb = 0; xRegUb = cellRegionNumUbX - 1; yRegUb = cellRegionNumUbY - 1; // DEBUG: Just to get all the regions
        //print_scalar_vals("xRegLb", xRegLb, "yRegLb", yRegLb, "xRegUb", xRegUb, "yRegUb", yRegUb);
        int iX = xRegLb, iY = yRegLb;
        while(true){
            // Search the alive and dead cells in the region and map their cell ids to their distance from the current cell
            std::pair<int, int> pReg = {iX, iY};
            add_nearby_cells_to_distance_map(pActivesRegions[pReg]);
            increment_iX_or_iY(iX, iY);
        }

        // Transfer nearbyCellDistances to a vector
        std::vector<std::pair<int, float>> nearbyCellDistancesVec;
        for(auto ele : nearbyCellDistances){
            int cellId = ele.first;
            float distXY = ele.second;
            nearbyCellDistancesVec.push_back({cellId, distXY});
        }

        // Sort this based on their distances from the current cell
        std::sort(nearbyCellDistancesVec.begin(), nearbyCellDistancesVec.end(),
            [](auto &left, auto &right){
                float distXY_left = left.second;
                float distXY_right = right.second;
                return distXY_right > distXY_left;
            }
        );

        // Add the first few closest cells' unique ids to the list to be returned
        //  (don't return more than the specified upper limit)
        std::vector<int> nearestCellIds;
        std::vector<float> nearestDistXY;
        for(int i = 0; i < min_int(nearbyCellDistancesVec.size(), maxNumCellsToReturn); i++){
            int cellId = nearbyCellDistancesVec[i].first;
            nearestCellIds.push_back(cellId);
            float distXY = nearbyCellDistancesVec[i].second;
            nearestDistXY.push_back(distXY);
        }
        //if(nearestCellIds.size() > 0 && stats["EAM_CELLS"][0] == 100){
        //    print_1d_vec("  nearestCellIds", nearestCellIds);
        //    print_1d_vec("  nearestDistXY" , nearestDistXY );
        //}
        return nearestCellIds;
    }
    // NOTE: This function also determines what the AI inputs are
    std::vector<float> get_ai_inputs(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*> pCellsHist){
        std::vector<float> aiInputs;
        int _numAiInputs = 0;
        #define add_to_neural_net(aiInputs, property, _numAiInputs) {aiInputs.push_back(property); _numAiInputs++;}

        // Internal Timers
        aiInputs.push_back((float)age); _numAiInputs++;
        aiInputs.push_back((float)attackCooldown); _numAiInputs++;
        // State and Cell Interactions
        aiInputs.push_back((float)health); _numAiInputs++;
        aiInputs.push_back((float)energy); _numAiInputs++;
        aiInputs.push_back((float)forceX); _numAiInputs++;
        aiInputs.push_back((float)forceY); _numAiInputs++;
        // Also need the 
        //  (a) Find out which 100 cells are closest
        //  (b) For now, we just care about their id similarity
        int maxNumCellsSeen = 10;
        #define get_pCell(cellId) pCellsHist[cellId]
        std::vector<int> nearestCellIds = get_nearest_cell_ids(maxNumCellsSeen, pActivesRegions, pCellsHist);
        for(int i = 0; i < maxNumCellsSeen; i++){
            float ageOther = 0, attackCooldownOther = 0;
            float healthOther = 0, energyOther = 0;
            float idSimilarity = 0;
            float relDist = 0, relDirOther = 0, relSpeedRadial = 0, relSpeedTangential = 0;
            if(i < nearestCellIds.size()){
                int cellId = nearestCellIds[i];
                Cell* pCell = get_pCell(cellId);
                ageOther = pCell->age; attackCooldownOther = pCell->attackCooldown;
                healthOther = pCell->health; energyOther = pCell->energy;
                idSimilarity = get_id_similarity(pCell);
                relDist = calc_distance_from_point(pCell->posX, pCell->posY);
                // Standard angle of linepointing from other cell to current cell
                relDirOther = (pCell->speedDir + 180) % 360; // degrees
                // Positive indicates movement toward the cell
                relSpeedRadial = pCell->get_speed() * cos_deg(relDirOther);
                // Positive indicates clockwise movement around the cell
                relSpeedTangential = pCell->get_speed() * sin_deg(relDirOther);
            }
            // Other cells' internal timers
            add_to_neural_net(aiInputs, ageOther, _numAiInputs);
            add_to_neural_net(aiInputs, attackCooldownOther, _numAiInputs);
            // Other cells' states
            add_to_neural_net(aiInputs, healthOther, _numAiInputs);
            add_to_neural_net(aiInputs, energyOther, _numAiInputs);
            // Other cells' similarity
            add_to_neural_net(aiInputs, idSimilarity, _numAiInputs);
            // Other cells' relative distance, speed, and direction
            add_to_neural_net(aiInputs, relDist, _numAiInputs);
            add_to_neural_net(aiInputs, relDirOther, _numAiInputs);
            add_to_neural_net(aiInputs, relSpeedRadial, _numAiInputs);
            add_to_neural_net(aiInputs, relSpeedTangential, _numAiInputs);
        }
        // Ensure the neural network input layer is valid
        if(nodesPerLayer[0] < 0) nodesPerLayer[0] = aiInputs.size();
        else assert(aiInputs.size() == nodesPerLayer[0]);
        assert(_numAiInputs == nodesPerLayer[0]);

        #undef get_pCell
        return aiInputs;
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
        // Note: For stats and variables that never change, I only need to update this once
        if(!isAlive){ energyCostPerFrame = 0; return; }

        // Cloning
        energyCostToClone = 0;
        energyCostToCloneMap["base"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["base"], {{"x", 2*stats["initEnergy"][0]}});
        energyCostToCloneMap["visionDist"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["visionDist"], {{"x", stats["visionDist"][0]}, {"size", size}});
        if(stats["attack"][0]) energyCostToCloneMap["attack"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["attack"], {{"x", stats["attack"][0]}, {"size", size}});
        else energyCostToCloneMap["attack"] = 0;
        energyCostToCloneMap["size"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["size"], {{"x", size}, {"size", size}});
        for(auto item : energyCostToCloneMap) energyCostToClone += item.second;

        // Surviving (per second)
        energyCostPerFrame = 0;
        energyCostPerSecMap["base"] =         StrExprInt::solve(ENERGY_COST_PER_USE["base"], {{"x", -1}, {"size", size}});
        energyCostPerSecMap["visionDist"] =   StrExprInt::solve(ENERGY_COST_PER_USE["visionDist"], {{"x", stats["visionDist"][0]}, {"size", size}});
        energyCostPerSecMap["maxHealth"] =    StrExprInt::solve(ENERGY_COST_PER_USE["maxHealth"], {{"x", stats["maxHealth"][0]}, {"size", size}});
        energyCostPerSecMap["age"] = StrExprInt::solve(ENERGY_COST_PER_USE["age"], {{"x", age}, {"size", size}});
        for(auto item : energyCostPerSecMap) energyCostPerFrame += item.second;
        energyCostPerFrame /= TICKS_PER_SEC;

        // Surviving (per frame) and using abilities
        energyCostPerUse["speedRun"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],  {{"x", stats["speedRun"][0]}, {"size", size}});
        energyCostPerUse["speedWalk"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"], {{"x", stats["speedWalk"][0]}, {"size", size}});
        energyCostPerUse["speedIdle"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"], {{"x", stats["speedIdle"][0]}, {"size", size}});
        energyCostPerUse["attack"] = StrExprInt::solve(ENERGY_COST_PER_USE["attack"],   {{"x", stats["attack"][0]}, {"size", size}});
    }
    void consume_energy_per_frame(){
        if(!isAlive) return;
        update_energy_costs();
        energy -= energyCostPerFrame;
        if(speedMode == IDLE_MODE) energy -= energyCostPerUse["speedIdle"] / TICKS_PER_SEC;
        if(speedMode == WALK_MODE) energy -= energyCostPerUse["speedWalk"] / TICKS_PER_SEC;
        if(speedMode == RUN_MODE)  energy -= energyCostPerUse["speedRun"] / TICKS_PER_SEC;
    }
    void enforce_valid_ai(){
        enforce_valid_ai_structure();
        enforce_valid_ai_inputs();
    }
    void enforce_valid_ai_structure(){
        if(aiNetwork.size() != nodesPerLayer.size() - 1) print_scalar_vals("aiNetwork.size()", aiNetwork.size(), "nodesPerLayer.size()", nodesPerLayer.size());
        assert(aiNetwork.size() == nodesPerLayer.size() - 1);
        for(auto num : nodesPerLayer) assert(0 < num);
        for(int layerNum = 1; layerNum < aiNetwork.size(); layerNum++){
            assert(aiNetwork[layerNum-1].size() == nodesPerLayer[layerNum]);
            //  aiNetwork[layerNum-1].size() is the number of nodes in layerNum,
            //  where layer 0 is the input layer
        }
    }
    void enforce_valid_ai_inputs(){
        // Cell Inputs
        assert(speedMode == IDLE_MODE || speedMode == WALK_MODE || speedMode == RUN_MODE);
        while(speedDir < 0) speedDir += 360;
        speedDir = speedDir % 360 - speedDir % 15; // Originally: speedDir %= 360;
        while(cloningDirection < 0) cloningDirection += 360;
        cloningDirection %= 360;
        // doAttack, doSelfDestruct, doCloning
    }
    // If the cell isn't valid, change the variables so it is valid.
    // Also, update the dependent variables
    void enforce_valid_cell(bool enforceStats){
        assert(pSelf != NULL);
        assert(uniqueCellNum >= 0);

        // Cell State
        enforce_bounds(health, 0, stats["maxHealth"][0]);
        enforce_bounds(energy, 0, stats["maxEnergy"][0]);

        enforce_valid_ai_inputs();

        // Physics, position, etc.
        enforce_valid_xyPos();

        // Stats
        if(enforceStats){
            for(auto item : stats){
                std::string statName = item.first;
                int lb = stats[statName][1], ub = stats[statName][2];
                enforce_bounds(stats[statName][0], lb, ub);
            }
            enforce_bounds(stats["initEnergy"][0], 0, stats["maxEnergy"][0]);
            stats["speedWalk"][0] = max_int(stats["speedIdle"][0], stats["speedWalk"][0]);
            stats["speedRun"][0]  = max_int(stats["speedWalk"][0], stats["speedRun"][0] );
            update_size();
            enforce_EAM_constraints();
            enforce_valid_ai();
            update_energy_costs();
        }
        

        // Constraints related to whether the cell is dead or alive
        if(isAlive){
            timeSinceDead = -1;
            decayPeriod = -1;
            decayRate = -1;
        } else {
            if(forcedDecisionsQueue.size() == 0) force_decision(10000, 0, 0, IDLE_MODE, false, false, false);
            decayRate = saturate_int(decayRate, 0, 100);
            if(decayPeriod <= 0) decayPeriod = 1;
            if(timeSinceDead < 0) timeSinceDead = 0;
        }
    }
    void initialize_cell(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*>& pCellsHist){
        assert(pSelf != NULL);
        age = 0;
        attackCooldown = stats["maxAtkCooldown"][0];
        energy = stats["initEnergy"][0];
        health = stats["maxHealth"][0];
        if(pParent == NULL) init_ai(pActivesRegions, pCellsHist);
        // Sort out initial decisions
        if(aiMode == RNG_BASED_AI_MODE){
            //force_decision(1, 0, 0, IDLE_MODE, doAttack, false, doCloning);
            //speedDir = rand() % 360;
            //cloningDirection = rand() % 360;
            int _speedMode = speedMode;
            int _rngPct = rand() % 100; 
            if(_rngPct < stats["rngAi_pctChanceIdle"][0]) _speedMode = IDLE_MODE;
            else if(_rngPct < stats["rngAi_pctChanceIdle"][0] + stats["rngAi_pctChanceWalk"][0]) _speedMode = WALK_MODE;
            else _speedMode = RUN_MODE;
            //doAttack = enableAutomaticAttack;
            //doSelfDestruct = false;
            //doCloning = enableAutomaticCloning;
            set_ai_outputs(rand() % 360, rand() % 360, _speedMode, enableAutomaticAttack, false, enableAutomaticCloning);
        }
        drawVisionRadius = true;
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
    void init_ai(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*>& pCellsHist){
        // NOTE: Do NOT use this function until all the inputs are initialized
        std::vector<float> aiInputs = get_ai_inputs(pActivesRegions, pCellsHist);
        std::tuple<std::vector<int>, std::vector<bool>> aiOutputs = get_ai_outputs();

        // Start with the (first) hidden layer, doing more of them if needed
        for(int i = 1; i < nodesPerLayer.size(); i++){
            assert(0 < nodesPerLayer[i] && nodesPerLayer[i] < 1000);
        }
        for(int layerNum = 1; layerNum < nodesPerLayer.size(); layerNum++){
            std::vector<aiNode> layerNodes;
            for(int nodeNum = 0; nodeNum < nodesPerLayer[layerNum]; nodeNum++){
                aiNode nextNode;
                nextNode.init_node(1000 * layerNum + nodeNum, false, nodesPerLayer[layerNum - 1]);
                layerNodes.push_back(nextNode);
            }
            aiNetwork.push_back(layerNodes);
        }
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
        if(isAlive) age++;
        else timeSinceDead++;
        if(attackCooldown > 0) attackCooldown--;
    }
    void force_decision(int numFrames, int _speedDir, int _cloningDir, int _speedMode, bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        forcedDecisionsQueue.push_back({numFrames, _speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning});
    }
    // i.e. force_reaction(...)
    void force_immediate_decision(int numFrames, int _speedDir, int _cloningDir, int _speedMode, bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        forcedDecisionsQueue.insert(forcedDecisionsQueue.begin(), {numFrames, _speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning});
    }
    void clear_forced_decisions(){
        forcedDecisionsQueue.clear();
    }
    // To override the ai, append an entry to forcedDecisionsQueue
    void decide_next_frame(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*>& pCellsHist){
        // Modify the values the creature can directly control based on the ai
        //  i.e. the creature decides what to do based on this function
        int _speedDir = speedDir, _cloningDir = cloningDirection, _speedMode = speedMode;
        bool _doAttack = doAttack, _doSelfDestruct = doSelfDestruct, _doCloning = doCloning;
        update_timers();
        // If a decision is forced, then follow that decision
        if(forcedDecisionsQueue.size() > 0){
            #define x(i) std::get<i>(forcedDecisionsQueue[0])
            x(0)--;
            if(0 <= x(1) && x(1) < 360) _speedDir = x(1);
            if(0 <= x(2) && x(2) < 360) _cloningDir = x(2);
            assert(x(3) == IDLE_MODE || x(3) == WALK_MODE || x(3) == RUN_MODE);
            _speedMode = x(3);
            _doAttack = x(4) && attackCooldown == 0;
            _doSelfDestruct = x(5);
            _doCloning = x(6);
            set_ai_outputs(_speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
            //cout << "forcedDecisionsQueue[0]: " << x(0) << ", " << x(1) << ", " << x(2) << ", " << x(3) << ", " << x(4) << ", " << x(5) << ", " << x(6) << endl;
            if(x(0) <= 0) forcedDecisionsQueue.erase(forcedDecisionsQueue.begin());
            #undef x
            enforce_valid_cell(false);
            return;
        }

        if(!isAlive){
            set_ai_outputs(0, 0, IDLE_MODE, false, false, false);
            enforce_valid_cell(false);
            return;
        }

        if(aiMode == EVOLUTIONARY_NEURAL_NETWORK_AI_MODE){
            // If the AI is free to decide, then decide what to do
            std::vector<float> layerInputs = get_ai_inputs(pActivesRegions, pCellsHist);
            for(int layerNum = 1; layerNum < aiNetwork.size(); layerNum++){
                layerInputs = do_forward_prop_1_layer(layerInputs, layerNum);
            }
            _speedDir = saturate_int((int)layerInputs[0], 0, 359);
            _cloningDir = saturate_int((int)layerInputs[2], 0, 359);
            _speedMode = (char)saturate_int((char)layerInputs[3], IDLE_MODE, RUN_MODE);
            _doAttack = (layerInputs[4] >= 0 && enableAutomaticAttack && attackCooldown == 0);
            _doSelfDestruct = (layerInputs[5] >= 1 && enableAutomaticSelfDestruct); // If this condition is too easy to trigger, then cells die too easily
            _doCloning = (layerInputs[6] >= 0 && enableAutomaticCloning);
            set_ai_outputs(_speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
            return;
        }
        
        if(aiMode == RNG_BASED_AI_MODE){
            _cloningDir = rand() % 360;
            _doAttack = enableAutomaticAttack && attackCooldown == 0;
            _doSelfDestruct = enableAutomaticSelfDestruct;
            _doCloning = enableAutomaticCloning;
            if(stats["EAM_SUN"][0] == 100){
                set_ai_outputs(0, rand() % 360, IDLE_MODE, false, false, _doCloning);
                return;
            }
            if(stats["EAM_GND"][0] == 100){
                do_random_cell_activity(5, 5, false, _doCloning);
                return;
            }
            std::vector<int> nearestCellIds = get_nearest_cell_ids(100, pActivesRegions, pCellsHist);
            if(nearestCellIds.size() == 0){
                do_random_cell_activity(5, 5, _doAttack, _doCloning);
                return;
            }
            chase_optimal_cell(pCellsHist, nearestCellIds, _doAttack, false, _doCloning);
            return;
        }
    }
    void mutate_ai(){
        float prob = (float)stats["mutationRate"][0] / stats["mutationRate"][2];
        for(int i = 0; i < aiNetwork.size(); i++){
            for(int j = 0; j < aiNetwork[i].size(); j++){
                aiNetwork[i][j].mutate_node(stats["mutationRate"][0], prob);
            }
        }
        enforce_valid_ai();
    }
    // Define the identity of the cell (in relation to the rest of the simulator)
    void define_self(int _cellNum, Cell* _pSelf, Cell* _pParent){
        // NOTE: If I push a copy of the cell into a new location, I need to update the new cell's identity
        pSelf = _pSelf;
        pParent = _pParent;
        uniqueCellNum = _cellNum;
    }
    void set_initEnergy(int val, bool setEnergy = true){
        stats["initEnergy"][0] = val;
        if(setEnergy) energy = stats["initEnergy"][0];
    }
    // NOTE: The full energy accumulation can only be done after this function is applied to every cell
    //  in the local area.
    // This function causes cells to accumulate energy from the sun, ground, and dead cells.
    //  Also, energy loss due to overcrowding leads is applied by this function
    void do_energy_transfer(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions){
        if(!isAlive) return;
        //cout << "  energy: " << energy << " ----> ";
        // Energy from the sun
        //  First, calculate which cells are touching the current cell
        std::vector<Cell*> touchingCells = find_touching_cells(pActivesRegions);
        for(int i = touchingCells.size()-1; i >= 0; i--){
            // Dead cells don't affect energy transfer here
            if(touchingCells[i]->isAlive == false) touchingCells.erase(touchingCells.begin() + i);
        }
        //  TODO: Give each cell a dexterity (dex) stat, which will help them NOT be blocked by other cells.
        //  Bigger cells get more of the energy and will receive most of the energy if competing with smaller cells.
        int sumOfCellSizes = size;
        for(auto cell : touchingCells) sumOfCellSizes += cell->size;
        energy += (float)energyFromSunPerSec * stats["EAM_SUN"][0] * size / 100 / sumOfCellSizes;
        
        // Energy from the ground -> Energy may be shared between cells,
        //  so it is better to add a pointer to the cell to each applicable ground cell's
        //  list of cells to which it will distribute energy
        enforce_valid_xyPos();
        if(simGndEnergy[posY][posX] < stats["EAM_GND"][0]){
            energy += simGndEnergy[posY][posX];
            simGndEnergy[posY][posX] = 0;
        } else {
            energy += stats["EAM_GND"][0];
            simGndEnergy[posY][posX] -= stats["EAM_GND"][0];
        }
        
        // Energy from cells which just died -> Add a pointer to the cell to the list
        //  associated with the cell that just died last frame AND is within range
        //  NOTE: This is dealt with in the DeadCell struct

        // Energy loss from overcrowding directly
        // TODO: Create a dex stat to resist this overcrowding
        energy -= StrExprInt::solve(ENERGY_COST_PER_USE["overcrowding"],
            {{"overcrowdingEnergyCoef", overcrowdingEnergyCoef.val},
            {"x", sumOfCellSizes-size}, {"size", size}});

        // Enforce energy constraints
        energy = min_int(energy, stats["maxEnergy"][0]);
        enforce_valid_cell(false);
        //cout << energy << endl;
    }
    // The dead cells and ground transfer energy to the living cells and / or the environment
    void do_energy_decay(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions){
        // Alive cells don't decay
        if(isAlive) return;
        // Energy to cells which are touching the dead cell
        std::vector<Cell*> touchingCells = find_touching_cells(pActivesRegions);
        for(int i = touchingCells.size()-1; i >= 0; i--){
            // Remove dead cells from this list, as they don't receive any of the energy
            if(touchingCells[i]->isAlive == false) touchingCells.erase(touchingCells.begin() + i);
        }
        // Calculate the amount of energy to give to other cells
        int rmEnergy = 0; // Energy to give to other cells
        std::vector<int> energyWeight(touchingCells.size());
        for(int i = 0; i < energyWeight.size(); i++){
            energyWeight[i] = touchingCells[i]->stats["EAM_CELLS"][0] * (energy + 200) / 1000;
            rmEnergy += energyWeight[i];
        }
        // Ensure only the dead cell's total amount of energy can be given away at most 
        if (rmEnergy > energy) {
            float multiplyBy = (float)energy / (float)rmEnergy;
            for(int i = 0; i < touchingCells.size(); i++){
                Cell* pCell = touchingCells[i];
                pCell->energy += multiplyBy * energyWeight[i] * pCell->stats["EAM_CELLS"][0] / 100;
            }
            energy = 0;
            enforce_valid_cell(false);
            //cout << "  energy of dead cell is fully depleted\n";
            return;
        }
        // Distribute the energy to the cells eating the dead cell
        for(int i = 0; i < touchingCells.size(); i++) {
            Cell* pCell = touchingCells[i];
            int cellEnergyGain = energyWeight[i];// * pCell->stats["EAM_CELLS"][0] / 100;
            pCell->energy += cellEnergyGain;
            //if(cellEnergyGain >= 100) pCell->force_immediate_decision(1, pCell->speedDir, pCell->cloningDirection, IDLE_MODE, pCell->doAttack, pCell->doSelfDestruct, pCell->doCloning);
        }
        energy -= rmEnergy;
        //print_scalar_vals("  dead cell energy consumed by predators", rmEnergy);

        if(decayPeriod <= 0) return;

        // Energy to ground
        rmEnergy = 0;
        int _efficiencyPct = 0; // TODO: Ensure this is non-zero after the first video is published
        if(timeSinceDead % decayPeriod == 0) rmEnergy = decayRate * energy / 100 + 20;
        simGndEnergy[posY][posX] = min_int(maxGndEnergy.val, simGndEnergy[posY][posX] + rmEnergy * _efficiencyPct / 100);
        energy -= rmEnergy;
        //print_scalar_vals("  decayed energy", rmEnergy, "decayRate", decayRate, "decayPeriod", decayPeriod, "timeSinceDead", timeSinceDead, "Remaining energy", energy);

        enforce_valid_cell(false);
    }
    void randomize_pos(int _lbX, int _ubX, int _lbY, int _ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, _lbX, _ubX);
        posY = gen_uniform_int_dist(rng, _lbY, _ubY);
        enforce_valid_xyPos();
    }
    Cell* clone_self(int cellNum, std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions,
    std::vector<Cell*>& pCellsHist, std::vector<Cell*>& pActives,
    int targetCloningDir = -1, bool randomizeCloningDir = false, bool doMutation = true){
        // The clone's position will be roughly the cell's diameter plus 1 away from the cell
        //Cell* pClone = new Cell(cellNum, CELL_TYPE_GENERIC, pAlivesRegions, pSelf);
        assert(pSelf != NULL);
        if(pSelf->isAlive == false){
            cout << "WARNING: A dead cell just tried to clone itself! This attempt failed!\n";
            print_scalar_vals("frameNum", frameNum);
            assert(pSelf->isAlive);
        }
        energy -= energyCostToClone; //energy -= pSelf->energyCostToClone;

        // Cloning the cell
        Cell* pClone = new Cell();
        pCellsHist.push_back(pClone);
        pActives.push_back(pClone);
        *pClone = *pSelf; // Almost all quantities should be copied over perfectly
        pClone->define_self(cellNum, pClone, pSelf);
        pClone->initialize_cell(pActivesRegions, pCellsHist);
        assert(uniqueCellNum != pClone->uniqueCellNum);

        // Update the new cell's position and determine the cloning direction
        if(targetCloningDir < 0 || 360 <= targetCloningDir) {
            assert(randomizeCloningDir == false);
            cloningDirection = targetCloningDir;
        } else if (randomizeCloningDir) {
            cloningDirection = gen_uniform_int_dist(rng, 0, 359);
        }
        int cloningRadius = (stats["dia"][0] + pClone->stats["dia"][0] + 3) / 2;
        pClone->update_pos(posX + cloningRadius*cos_deg(cloningDirection), posY + cloningRadius*sin_deg(cloningDirection));

        // Possible mutations
        if(doMutation) pClone->mutate_stats();
        pClone->enforce_valid_cell(true);

        return pClone;
    }
    void print_id(){
        std::cout << "id: "; for(bool b : id) std::cout << b; std::cout << std::endl;
    }
    int get_speed(){
        switch(speedMode){
            case IDLE_MODE:
            return stats["speedIdle"][0];
            case WALK_MODE:
            return stats["speedWalk"][0];
            case RUN_MODE:
            return stats["speedRun"][0];
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
        int _speed = get_speed();
        if(_speed == 0) return;
        increment_pos(_speed * cos_deg(speedDir), _speed * sin_deg(speedDir));
        enforce_valid_xyPos();
    }
    void update_forces(std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions){
        // Dead cells aren't affected by force
        if(!isAlive) return;

        // First, calculate which cells are nearby
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();

        // Check each nearby cell for any forces
        // TODO: apply a force due to nonliving objects and walls, if applicable
        for(auto reg : neighboringRegions){
            for(auto pCell : pActivesRegions[reg]){
                if(pCell->isAlive == false) continue;
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
                int targetDist = (pCell->stats["dia"][0] + stats["dia"][0] + 1) / 2; 
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
                        forceX += force_dX;
                        forceY += force_dY;
                    }
                }
            }
        }
        return;
    }
    void apply_forces(){
        if(!isAlive) return;

        // Apply the forces, which should already calculated
        increment_pos(forceX / forceDampingFactor.val, forceY / forceDampingFactor.val);
        forceX = 0;
        forceY = 0;
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
    int calc_distX_from_point(int xCoord){
        // Assume -ubX.val < posX && posx < 2*ubX.val
        if(!WRAP_AROUND_X) return xCoord - posX;
        // The shortest path may involve warping between x = 0 and x = ubX.val-1
        std::vector<int> dxPossibilities = {xCoord - posX, xCoord - posX + ubX.val, xCoord - posX - ubX.val};
        int dx = xCoord - posX;
        for(auto val : dxPossibilities){
            if(abs(dx) > abs(val)) dx = val;
        }
        //if(uniqueCellNum == 0) cout << "dx: " << dx << endl;
        return dx;
    }
    int calc_distY_from_point(int yCoord){
        // Assume -ubY.val < posY && posY < 2*ubY.val
        if(!WRAP_AROUND_Y) return yCoord - posY;
        // The shortest path may involve warping between y = 0 and y = ubY.val-1
        std::vector<int> dyPossibilities = {yCoord - posY, yCoord - posY + ubY.val, yCoord - posY - ubY.val};
        int dy = yCoord - posY;
        for(auto val : dyPossibilities){
            if(abs(dy) > abs(val)) dy = val;
        }
        //if(uniqueCellNum == 0) cout << "dy: " << dy << endl;
        return dy;
    }
    float calc_distance_from_point(int xCoord, int yCoord){
        // Screen wrapping applies, if set that way in the simulation
        int xDist = calc_distX_from_point(xCoord);
        int yDist = calc_distY_from_point(yCoord);
        float ans = std::sqrt(xDist * xDist + yDist * yDist);
        return ans;
    }
    void attack_cell(Cell* pAttacked){
        pAttacked->health -= stats["attack"][0];
        attackCooldown = stats["maxAtkCooldown"][0];
        energy -= energyCostPerUse["attack"];
        enforce_valid_cell(false);
    }
    void apply_non_movement_decisions(std::vector<Cell*>& pActives, std::vector<Cell*>& pCellsHist,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pActivesRegions){

        if(doAttack && attackCooldown == 0 && energy > energyCostPerUse["attack"]){
            // Find out which regions neighbor the cell's region
            std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
            
            // Damage all cells that this cell touches excluding the cell itself
            // If health < 0, the cell will die when the death conditions are checked
            std::set<int> attackedCellNums;
            for(auto reg : neighboringRegions){
                for(auto pCell : pActivesRegions[reg]){
                    if(pCell->isAlive == false) continue;
                    if(uniqueCellNum == pCell->uniqueCellNum) continue;
                    if(attackedCellNums.count(pCell->uniqueCellNum)) continue;
                    float distXY = calc_distance_from_point(pCell->posX, pCell->posY);
                    float distanceThreshold = (stats["dia"][0] + pCell->stats["dia"][0] + 0.1) / 2;
                    //print_scalar_vals("distXY", distXY, "distanceThreshold", distanceThreshold);
                    if(distXY <= distanceThreshold){
                        attack_cell(pCell);
                        //forcedDecisionsQueue.insert(forcedDecisionsQueue.begin(), {1, speedDir, cloningDirection, speedMode, doAttack, doSelfDestruct, doCloning});
                        attackedCellNums.insert(pCell->uniqueCellNum);
                    }
                }
            }
            //print_scalar_vals("attackedCellNums.size()", attackedCellNums.size());
        }
        int numAliveCells = count_all_alive_cells(pActives);
        if(doCloning && energy > 1.2*energyCostToClone && pActives.size() < cellLimit.val){
            Cell* pCell = clone_self(pCellsHist.size(), pActivesRegions, pCellsHist, pActives, cloningDirection);
        }
        enforce_valid_cell(true);
    }
    void do_random_cell_activity(int pctChanceToChangeDir, int pctChanceToChangeSpeed, 
    bool _enableAttack, bool _enableCloning){
        int _speedDir = speedDir, _speedMode = speedMode;
        if(rand() % 100 < pctChanceToChangeDir){
            while(_speedDir == speedDir){
                _speedDir = rand() % 360;
                correct_speedDir();
            }
        }
        if(rand() % 100 < pctChanceToChangeSpeed){
            while(_speedMode == speedMode){
                int _rngPct = rand() % 100;
                if(_rngPct < stats["rngAi_pctChanceIdle"][0]) _speedMode = IDLE_MODE;
                else if(_rngPct < stats["rngAi_pctChanceIdle"][0] + stats["rngAi_pctChanceWalk"][0]) _speedMode = WALK_MODE;
                else _speedMode = RUN_MODE;
            }
        }
        set_ai_outputs(_speedDir, rand() % 360, _speedMode, _enableAttack, false, _enableCloning);
        enforce_valid_cell(false);
    }
    // Generate random-ish movement that looks reasonable for a cell to do
    void preplan_random_cell_activity(int pctChanceToChangeDir, int pctChanceToChangeSpeed, int numFrames,
    bool _enableAttack, bool _enableCloning){
        #define lastDecision(i) std::get<i>(forcedDecisionsQueue[forcedDecisionsQueue.size()-1])
        //{numFrames, _speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning})
        int newSpeedDir = speedDir, newSpeedMode = speedMode;
        if(forcedDecisionsQueue.size() > 0) newSpeedDir = lastDecision(1), newSpeedMode = lastDecision(3);
        int numFramesSinceLastDecision = 0;
        bool changedMovement = false;
        while(numFrames > 0){
            int rngDir = rand() % 100, rngSpeed = rand() % 100;
            if(rngDir < pctChanceToChangeDir){
                newSpeedDir = rand() % 360; // May still be going in the same direction
                changedMovement = true;
            }
            if(rngSpeed < pctChanceToChangeSpeed){
                while(newSpeedMode == lastDecision(3)){
                    int _rngPct = rand() % 100;
                    if(_rngPct < stats["rngAi_pctChanceIdle"][0]) newSpeedMode = IDLE_MODE;
                    else if(_rngPct < stats["rngAi_pctChanceIdle"][0] + stats["rngAi_pctChanceWalk"][0]) newSpeedMode = WALK_MODE;
                    else newSpeedMode = RUN_MODE;
                }
                changedMovement = true;
            }
            //print_scalar_vals("  rngDir", rngDir, "rngSpeed", rngSpeed);
            if(changedMovement || numFramesSinceLastDecision >= numFrames){
                changedMovement = false;
                force_decision(numFramesSinceLastDecision, newSpeedDir, rand() % 360, newSpeedMode, _enableAttack, false, _enableCloning);
                numFrames -= numFramesSinceLastDecision;
                //print_scalar_vals("numFramesSinceLastDecision", numFramesSinceLastDecision, "newSpeedDir", newSpeedDir, "newSpeedMode", newSpeedMode);
                numFramesSinceLastDecision = 0;
                changedMovement = false;
            } else {
                numFramesSinceLastDecision++;
            }
        }
        #undef lastDecision
    }
    int get_optimal_speedDir_to_point(int targetX, int targetY){
        int dx = calc_distX_from_point(targetX);
        int dy = calc_distY_from_point(targetY);
        //int dx = targetX - posX, dy = targetY - posY;
        float distance = calc_distance_from_point(targetX, targetY);
        //print_scalar_vals("posX", posX, "posY", posY, "targetX", targetX, "targetY", targetY, "dx", dx, "dy", dy, "distance", distance);

        if(distance == 0) return 0;
        int ans = (int)arc_cos_deg(abs(dx), distance);
        //print_scalar_vals("  distance", distance, "dx", dx, "dy", dy, "acos(|dx|, distance)", ans);
        if(dx >= 0){
            if(dy >= 0) ans = ans;
            else ans = 360 - ans;
            //print_scalar_vals("  (dx >= 0) ans", ans);
        } else {
            if(dy >= 0) ans = 180 - ans;
            else ans = 180 + ans;
            //print_scalar_vals("  (dx < 0) ans", ans);
        }
        // If speed is low, we may only be able to travel in certain directions
        if(stats["speedWalk"][0] > stats["speedRun"][0]) cout << "WARNING: Walk speed exceeds run speed!";
        switch(stats["speedRun"][0]){
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
        int speed = enableRunning*pSelf->stats["speedRun"][0] + !enableRunning*pSelf->stats["speedWalk"][0];
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
    // Modify each decision
    void chase_optimal_cell(std::vector<Cell*> pCellsHist,
    std::vector<int>& nearestCellIds, bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        //clear_forced_decisions();
        #define get_pCell(cellId) pCellsHist[cellId];

        // First, mark each cell with a number and record the largest (best) number
        std::vector<int> cellRanks;
        for(auto cellId : nearestCellIds){
            if(cellId == 0) cout << "abcdefg\n";
            Cell* pCell = get_pCell(cellId);
            int rank = 0;
            int deadCoef = 16000, plantCoef = 8000, gndCoef = 4000, balancedCoef = 2000, predatorCoef = 1000;
            int speedCoef = -10, distanceCoef = -1, doAttackCoef = 0;

            // Assume the dead cell hasn't moved since dying
            if(pCell->isAlive == false) assert(pCell->speedMode == IDLE_MODE);
            int xNext = pCell->posX + pCell->get_speed()*cos_deg(pCell->speedDir);
            int yNext = pCell->posY + pCell->get_speed()*sin_deg(pCell->speedDir);
            int distance = calc_distance_from_point(xNext, yNext);
            int optimalSpeedDir = get_optimal_speedDir_to_point(xNext, yNext);
            int speedDirDiff = min_int(abs(optimalSpeedDir - speedDir), abs(optimalSpeedDir - speedDir - 360));

            // Update Rank
            rank += deadCoef * !(pCell->isAlive);
            rank += plantCoef * (pCell->stats["EAM_SUN"][0] == 100);
            rank += gndCoef * (pCell->stats["EAM_GND"][0] == 100);
            rank += balancedCoef * (pCell->stats["EAM_SUN"][0] < 100 && pCell->stats["EAM_GND"][0] < 100 && pCell->stats["EAM_CELLS"][0] < 100);
            rank += predatorCoef * (pCell->stats["EAM_CELLS"][0] == 100);
            rank += speedCoef * pCell->get_speed();
            rank += distanceCoef * distance;
            //rank += dirCoef * speedDirDiff / 180;
            rank += doAttackCoef * pCell->doAttack;
            cellRanks.push_back(rank);
        }
        int maxRank = cellRanks[0];
        for(auto rank : cellRanks) maxRank = max_int(maxRank, rank);
        for(int i = cellRanks.size()-1; i >= 0; i--){
            if(cellRanks[i] != maxRank) nearestCellIds.erase(nearestCellIds.begin() + i);
        }
        // Get the optimal speedDir and speed for the first cell in the list
        int cellIdToChase = nearestCellIds[0];
        Cell* pTarget = get_pCell(cellIdToChase);
        float effectiveDistFromTarget = calc_distance_from_point(pTarget->posX, pTarget->posY) - (float)(stats["dia"][0] + pTarget->stats["dia"][0]) / 2;
        float targetDistance = calc_distance_from_point(pTarget->posX, pTarget->posY);
        bool isTouchingTarget = ( targetDistance - (float)(stats["dia"][0] + pTarget->stats["dia"][0]) / 2 ) <= 0;
        if(pTarget->isAlive == false && isTouchingTarget){
            set_ai_outputs(0, rand() % 360, IDLE_MODE, false, false, _doCloning);
            return;
        }
        int xNext = pTarget->posX + pTarget->get_speed()*cos_deg(pTarget->speedDir);
        int yNext = pTarget->posY + pTarget->get_speed()*sin_deg(pTarget->speedDir);
        int _speedDir = get_optimal_speedDir_to_point(xNext, yNext);
        // Pursue the relevant cell for 1 more frame
        //int targetDistance = calc_distance_from_point(pTarget->posX, pTarget->posY);
        int targetSpeed = find_closest_value(targetDistance, {stats["speedIdle"][0], stats["speedWalk"][0], stats["speedRun"][0]});
        int _speedMode = speedMode;
        if(targetSpeed == stats["speedIdle"][0]) _speedMode = stats["speedIdle"][0];
        if(targetSpeed == stats["speedWalk"][0]) _speedMode = stats["speedWalk"][0];
        if(targetSpeed == stats["speedRun"][0])  _speedMode = stats["speedRun"][0];
        set_ai_outputs(_speedDir, rand() % 360, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
        enforce_valid_cell(false);
        //print_scalar_vals("  cellId", uniqueCellNum, "_speedDir", _speedDir, "_speedMode", _speedMode, "xNext", xNext, "yNext", yNext,
        //    "_speedDir", _speedDir, "targetDistance", targetDistance, "targetSpeed", targetSpeed, "posX", posX, "posY", posY);

        #undef get_pCell
    }
    void kill_self(){
        assert(isAlive);
        decayPeriod = 1;
        decayRate = 2;
        energy += energyCostToClone;
        timeSinceDead = 0;
        isAlive = false;
        speedMode = IDLE_MODE;
        clear_forced_decisions();
        //pActives.erase(pActives.begin() + i_pAlive);
        //pActives.push_back(pSelf);
    }
    void remove_this_dead_cell_if_depleted(std::vector<Cell*>& pActives, int iDead){
        if(isAlive || energy > 0) return;
        assert(pActives[iDead] == pSelf);
        pActives.erase(pActives.begin() + iDead);
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
        return weights;
    }
    std::vector<SDL_Texture*> findEAMTex(){
        std::vector<SDL_Texture*> ans;
        // First, check if everything is balanced
        int minEAM = stats["EAM_SUN"][0];
        minEAM = min_int(minEAM, stats["EAM_GND"][0]);
        minEAM = min_int(minEAM, stats["EAM_CELLS"][0]);
        int maxEAM = stats["EAM_SUN"][0];
        maxEAM = max_int(maxEAM, stats["EAM_GND"][0]);
        maxEAM = max_int(maxEAM, stats["EAM_CELLS"][0]);
        if(maxEAM <= 2*minEAM){
            ans.push_back(P_EAM_TEX["balanced"]);
            return ans;
        }
        int numPixelsInEAMTex = 4;
        int EAMPerPixel = REQ_EAM_SUM / numPixelsInEAMTex;
        if(stats["EAM_GND"][0] < EAMPerPixel) ans.push_back(P_EAM_TEX["balanced"]);
        else ans.push_back(P_EAM_TEX["g4"]); // Ground (or balanced) will fill in the empty spaces
        // NOTE: there are 4 pixels to color in
        // We only have to worry about the sun and predation textures, since the remaining
        //  is already taken care of
        if(stats["EAM_SUN"][0] / EAMPerPixel > 0){
            std::string nextFile = "s" + std::to_string(stats["EAM_SUN"][0] / EAMPerPixel);
            ans.push_back(P_EAM_TEX[nextFile]);
        }
        if(stats["EAM_CELLS"][0] / EAMPerPixel > 0){
            std::string nextFile = "c" + std::to_string(stats["EAM_CELLS"][0] / EAMPerPixel);
            ans.push_back(P_EAM_TEX[nextFile]);
        }
        return ans;
    }
    void draw_cell(){
        int drawX = drawScaleFactor*(posX + 0.5 - (float)stats["dia"][0]/2);
        int drawY = drawScaleFactor*(posY + 0.5 - (float)stats["dia"][0]/2);
        int drawSize = drawScaleFactor*stats["dia"][0];
        if(!isAlive){ draw_texture(pDeadCellTex, drawX, drawY, drawSize, drawSize); return; }
        draw_texture(pCellSkeleton, drawX, drawY, drawSize, drawSize, true);
        // Draw the health and energy on top of this
        SDL_Texture* energyTex = findSDLTex(energy * 100 / stats["maxEnergy"][0], P_CELL_ENERGY_TEX);
        draw_texture(energyTex, drawX, drawY, drawSize, drawSize, true);
        SDL_Texture* healthTex = findSDLTex(100*health/stats["maxHealth"][0], P_CELL_HEALTH_TEX);
        draw_texture(healthTex, drawX, drawY, drawSize, drawSize, true);
        if(doAttack && stats["attack"][0] > 0)  draw_texture(pDoAttackTex,  drawX, drawY, drawSize, drawSize, true);
        if(doCloning) draw_texture(pDoCloningTex, drawX, drawY, drawSize, drawSize, true);
        std::vector<SDL_Texture*> EAM_Tex = findEAMTex();
        for(auto tex : EAM_Tex) draw_texture(tex, drawX, drawY, drawSize, drawSize, true);
        if(drawVisionRadius && stats["visionDist"][0] > 0){
            int drawCenterX = drawScaleFactor*(posX + 0.5);
            int drawCenterY = drawScaleFactor*(posY + 0.5);
            int drawRadius = stats["visionDist"][0]*drawScaleFactor;
            SDL_Color white = {0xff, 0xff, 0xff, 0x40};
            if(drawRadius >= min_int(ubX_px, ubY_px) / 2) white = {0xff, 0xff, 0xff, 0x05};
            draw_regular_polygon(drawCenterX, drawCenterY, drawRadius, 32, white);
            if(drawCenterY < drawRadius)                                                draw_regular_polygon(drawCenterX         , drawCenterY + ubY_px, drawRadius, 32, white);
            if(drawCenterY > ubY_px - drawRadius)                                       draw_regular_polygon(drawCenterX         , drawCenterY - ubY_px, drawRadius, 32, white);
            if(drawCenterX < drawRadius && true)                                        draw_regular_polygon(drawCenterX + ubX_px, drawCenterY         , drawRadius, 32, white);
            if(drawCenterX < drawRadius && drawCenterY < drawRadius)                    draw_regular_polygon(drawCenterX + ubX_px, drawCenterY + ubY_px, drawRadius, 32, white);
            if(drawCenterX < drawRadius && drawCenterY > ubY_px - drawRadius)           draw_regular_polygon(drawCenterX + ubX_px, drawCenterY - ubY_px, drawRadius, 32, white);
            if(drawCenterX > ubX_px - drawRadius && true)                               draw_regular_polygon(drawCenterX - ubX_px, drawCenterY         , drawRadius, 32, white);
            if(drawCenterX > ubX_px - drawRadius && drawCenterY < drawRadius)           draw_regular_polygon(drawCenterX - ubX_px, drawCenterY + ubY_px, drawRadius, 32, white);
            if(drawCenterX > ubX_px - drawRadius && drawCenterY > ubY_px - drawRadius)  draw_regular_polygon(drawCenterX - ubX_px, drawCenterY - ubY_px, drawRadius, 32, white);
        }
    }
};





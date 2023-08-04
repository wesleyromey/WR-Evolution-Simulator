#ifndef SECONDARY_INCLUDES_H
#include "../secondary/secondaryIncludes.h"
#define SECONDARY_INCLUDES_H
#endif



//struct DeadCell;

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
    void gen_stats_random(int _cellType, std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
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
        stats["initEnergy"]     = {              1000,   500,  2000, mutChance, mutAmt}; // (0,10000) // TODO: Convert initEnergy to a mutatable stat
        stats["maxAtkCooldown"] = {                10,    10,    10,         0,      0}; // 10
        stats["maxEnergy"]      = { 5000*stats["dia"][0],  1, 90000,         0,      0}; // 5000*stats["dia"]
        stats["maxHealth"]      = { stat_init(1,   1),     1, 10000, mutChance, mutAmt}; // (1,10)
        // TODO: Add stats for the AI and its relevant mutation rate
        stats["mutationRate"]   = { stat_init(0,   0),     0,  1000,         0,      0}; // (0,1000)
        // TODO: change speedIdle, speedWalk, and speedRun to a "maxSpeed" stat and change speed to a continuously varying decision
        stats["speedIdle"]      = {                 0,     0,     0,         0,      0}; // 0
        stats["speedWalk"]      = {                 1,     0,    10, mutChance, mutAmt}; // (0, 1)
        stats["speedRun"]       = {                 2,     0,   100, mutChance, mutAmt}; // (0, 100)
        stats["stickiness"]     = {                 0,     0,     0,         0,      0}; // 0
        stats["visionDist"]     = { stat_init(5,   5),     0,  1000,         0,      0}; // (0, 10)
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
        initialize_cell(pAlivesRegions, pCellsHist);
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
        // TODO: Since I removed the decisions from this function, I may have to edit the unit tests
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
    std::vector<Cell*> find_touching_cells(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        std::set<int> checkedCells = {uniqueCellNum};
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
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
    //  Used to return std::vector<Cell*>
    std::vector<int> get_nearest_cell_ids(int maxNumCellsToReturn,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
    std::vector<Cell*> pCellsHist){
        // visionDist: The distance the cell can see
        int xReg = xyRegion.first, yReg = xyRegion.second;
        
        // A region counts as too far if it its nearest point is out of range
        //      NOTE: Assume that a cell's diameter is smaller than the size of each cell region
        //      NOTE: Also, some regions may be larger than CELL_REGION_SIDE_LEN
        float maxDetectableCellDistance = (float)stats["visionDist"][0] + CELL_REGION_SIDE_LEN; //(float)stats["dia"][2] / 2;
        //float visionDist_NumReg = maxDetectableCellDistance / CELL_REGION_SIDE_LEN;
        int visionDist_NumReg = maxDetectableCellDistance / CELL_REGION_SIDE_LEN;

        // Get the rectangular box in which all relevant regions appear
        std::tuple<int, int, int, int> xyRegionNeighborhoodBounds = get_xyRegion_neighborhoodBounds(visionDist_NumReg);
        int xRegLb = std::get<0>(xyRegionNeighborhoodBounds);
        int xRegUb = std::get<1>(xyRegionNeighborhoodBounds);
        int yRegLb = std::get<2>(xyRegionNeighborhoodBounds);
        int yRegUb = std::get<3>(xyRegionNeighborhoodBounds);
        int numRegionsToCheckX = (xRegUb - xRegLb + visionDist_NumReg) % cellRegionNumUbX;
        int numRegionsToCheckY = (yRegUb - yRegLb + visionDist_NumReg) % cellRegionNumUbY;


        /*
        // Sort every region in a copy of pAlivesRegions based on the distance
        //  from the current region
        std::vector<std::pair<int,int>> nearest_xyReg;
        // Only evaluate the regions nearest to the current cell
        //  NOTE: it would be equally valid to replace pAlivesRegions with pDeadsRegions, since their keys are identical
        for(auto pReg : pAlivesRegions){
            int xRegOther = pReg.first.first, yRegOther = pReg.first.second;
            int distX = xRegOther - xReg, distY = yRegOther - yReg;
            float distXY = sqrt(distX*distX + distY*distY);
            if(distXY <= visionDist_NumReg + 1) nearest_xyReg.push_back(pReg.first);
        }
        */
        // Go through all the regions within visionDist from the current cell
        //  TODO: Implement this for both alive cells and dead cells
        std::map<int, float> nearbyCellDistances;
        //std::vector<Cell*> pNearestCells;
        for(int iX = xRegLb, iY = yRegLb; iY != numRegionsToCheckX; iX++){
            iX %= cellRegionNumUbX;
            if(iX == xRegUb){ iX = 0; iY++; iY %= cellRegionNumUbY; }
            if(iX == xRegUb || iY == yRegUb) break;
            std::pair<int, int> pReg = {iX, iY};
            
            // Search the alive and dead cells in the region and map their cell ids to their distance from the current cell
            for(auto pCell : pAlivesRegions[pReg]){
                int cellId = pCell->uniqueCellNum;
                if(nearbyCellDistances.count(cellId)) continue;
                // Effective distance between 2 cells = distance - (dia of other cell) / 2
                float distXY = calc_distance_between_points(posX, posY, pCell->posX, pCell->posY);
                float effectiveVisionRadius = stats["visionDist"][0] + (float)pCell->stats["dia"][0]/2;
                if(distXY <= effectiveVisionRadius && pCell != pSelf){
                    nearbyCellDistances[cellId] = max_float(distXY - effectiveVisionRadius, 0);
                }
            }
            /*
            for(auto pCell : pDeadsRegions[pReg]){
                int cellId = pCell->uniqueCellNum;
                if(nearbyCellDistances.count(cellId)) continue;
                // Effective distance between 2 cells = distance - (dia of other cell) / 2
                float distXY = calc_distance_between_points(posX, posY, pCell->posX, pCell->posY);
                float effectiveVisionRadius = stats["visionDist"][0] + (float)pCell->dia;
                if(distXY <= effectiveVisionRadius){
                    nearbyCellDistances[cellId] = max_float(distXY - effectiveVisionRadius, 0);
                }
            }
            */
        }
        /*
        for(auto pReg : nearest_xyReg){
            for(auto pCell : pAlivesRegions[pReg]){
                int cellId = pCell->uniqueCellNum;
                if(nearbyCellDistances.count(cellId)) continue;
                // Effective distance between 2 cells = distance - (dia of other cell) / 2
                float distXY = calc_distance_between_points(posX, posY, pCell->posX, pCell->posY);
                float effectiveVisionRadius = stats["visionDist"][0] + (float)pCell->stats["dia"][0]/2;
                if(distXY <= effectiveVisionRadius && pCell != pSelf){
                    nearbyCellDistances[cellId] = max_float(distXY - effectiveVisionRadius, 0);
                }
            }
            for(auto pCell : pDeadsRegions[pReg]){
                int cellId = pCell->pOldSelf->uniqueCellNum;
                if(nearbyCellDistances.count(cellId)) continue;
                // Effective distance between 2 cells = distance - (dia of other cell) / 2
                float distXY = calc_distance_between_points(posX, posY, pCell->posX, pCell->posY);
                float effectiveVisionRadius = stats["visionDist"][0] + (float)pCell->dia;
                if(distXY <= effectiveVisionRadius){
                    nearbyCellDistances[cellId] = max_float(distXY - effectiveVisionRadius, 0);
                }
            }
        }
        */

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
                //int distX = left.first, distY = left.second;
                //float distXY_left = sqrt(distX*distX + distY*distY);
                //distX = right.first; distY = right.second;
                //float distXY_right = sqrt(distX*distX + distY*distY);
                //return distXY_right > distXY_left;
            }
        );

        /*
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
        */

        // Add the first few closest cells' unique ids to the list to be returned
        //  (don't return more than the specified upper limit)
        std::vector<int> nearestCellIds;
        for(int i = 0; i < min_int(nearbyCellDistancesVec.size(), maxNumCellsToReturn); i++){
            int cellId = nearbyCellDistancesVec[i].first;
            nearestCellIds.push_back(cellId);
        }
        return nearestCellIds;

        // Remove cells which occur too late in the list
        //while(nearbyCellDistancesVec.size() > maxNumCellsToReturn) nearbyCellDistancesVec.pop_back();
        //return nearestCellIds;
    }
    // NOTE: This function also determines what the AI inputs are
    std::vector<float> get_ai_inputs(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
    std::vector<Cell*> pCellsHist){
        std::vector<float> aiInputs;
        int _numAiInputs = 0;
        #define add_to_neural_net(aiInputs, property, _numAiInputs) {aiInputs.push_back(property); _numAiInputs++;}

        //cout << "get_ai_inputs()\n";
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
        std::vector<int> nearestCellIds = get_nearest_cell_ids(maxNumCellsSeen, pAlivesRegions, pCellsHist);
        //cout << "get_nearest_cell_ids() success\n";
        for(int i = 0; i < maxNumCellsSeen; i++){
            float ageOther = 0, attackCooldownOther = 0;
            float healthOther = 0, energyOther = 0;
            float idSimilarity = 0;
            float relDist = 0, relDirOther = 0, relSpeedRadial = 0, relSpeedTangential = 0;
            if(i < nearestCellIds.size()){
                int cellId = nearestCellIds[i];
                //cout << "about to run pCellsHist[cellId]\n";
                Cell* pCell = get_pCell(cellId);
                //cout << "pCell is successfully defined\n";
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
        //cout << "got ai inputs\n";
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
        if(stats["stickiness"][0]) energyCostToCloneMap["stickiness"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["stickiness"], {{"x", stats["stickiness"][0]}, {"size", size}});
        else energyCostToCloneMap["stickiness"] = 0;
        if(stats["attack"][0]) energyCostToCloneMap["attack"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["attack"], {{"x", stats["attack"][0]}, {"size", size}});
        else energyCostToCloneMap["attack"] = 0;
        energyCostToCloneMap["size"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["size"], {{"x", size}, {"size", size}});
        for(auto item : energyCostToCloneMap) energyCostToClone += item.second;

        // Surviving (per second)
        energyCostPerFrame = 0;
        energyCostPerSecMap["base"] =         StrExprInt::solve(ENERGY_COST_PER_USE["base"], {{"x", -1}, {"size", size}});
        energyCostPerSecMap["visionDist"] =   StrExprInt::solve(ENERGY_COST_PER_USE["visionDist"], {{"x", stats["visionDist"][0]}, {"size", size}});
        energyCostPerSecMap["stickiness"] =   StrExprInt::solve(ENERGY_COST_PER_USE["stickiness"], {{"x", stats["stickiness"][0]}, {"size", size}});
        energyCostPerSecMap["mutationRate"] = StrExprInt::solve(ENERGY_COST_PER_USE["mutationRate"], {{"x", stats["mutationRate"][0]}, {"size", size}});
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
        //cout << "  energy: " << energy;
        energy -= energyCostPerFrame;
        if(speedMode == IDLE_MODE) energy -= energyCostPerUse["speedIdle"] / TICKS_PER_SEC;
        if(speedMode == WALK_MODE) energy -= energyCostPerUse["speedWalk"] / TICKS_PER_SEC;
        if(speedMode == RUN_MODE)  energy -= energyCostPerUse["speedRun"] / TICKS_PER_SEC;
        //cout << " ----> " << energy << endl;
    }
    void enforce_valid_ai(){
        if(aiNetwork.size() != nodesPerLayer.size() - 1) print_scalar_vals("aiNetwork.size()", aiNetwork.size(), "nodesPerLayer.size()", nodesPerLayer.size());
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
        assert(pSelf != NULL);
        assert(uniqueCellNum >= 0);
        // Cell Inputs
        assert(speedMode == IDLE_MODE || speedMode == WALK_MODE || speedMode == RUN_MODE);
        while(speedDir < 0) speedDir += 360;
        speedDir = speedDir % 360 - speedDir % 15; // Originally: speedDir %= 360;
        while(cloningDirection < 0) cloningDirection += 360;
        cloningDirection %= 360;
        // doAttack, doSelfDestruct, doCloning

        // Cell State
        enforce_bounds(health, 0, stats["maxHealth"][0]);
        enforce_bounds(energy, 0, stats["maxEnergy"][0]);

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
    void initialize_cell(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
    std::vector<Cell*>& pCellsHist){
        assert(pSelf != NULL);
        age = 0;
        attackCooldown = stats["maxAtkCooldown"][0];
        energy = stats["initEnergy"][0];
        health = stats["maxHealth"][0];
        if(pParent == NULL) init_ai(pAlivesRegions, pCellsHist);
        // Sort out initial decisions
        if(aiMode == RNG_BASED_AI_MODE){
            force_decision(1, 0, 0, IDLE_MODE, doAttack, false, doCloning);
            speedDir = rand() % 360;
            cloningDirection = rand() % 360;
            int _rngPct = rand() % 100; 
            if(_rngPct < pctChanceIdle) speedMode = IDLE_MODE;
            else if(_rngPct < pctChanceIdle + pctChanceWalk) speedMode = WALK_MODE;
            else speedMode = RUN_MODE;
            doAttack = enableAutomaticAttack;
            doSelfDestruct = false;
            doCloning = enableAutomaticCloning;
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
    void init_ai(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
    std::vector<Cell*>& pCellsHist){
        //cout << "Initializing AI\n";
        // NOTE: Do NOT use this function until all the inputs are initialized
        std::vector<float> aiInputs = get_ai_inputs(pAlivesRegions, pCellsHist);
        std::tuple<std::vector<int>, std::vector<bool>> aiOutputs = get_ai_outputs();
        //cout << "retrieved AI I/O\n";

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
        //cout << "Initialized AI\n";
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
    void decide_next_frame(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
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
            //set_ai_outputs(_speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
            //cout << "forcedDecisionsQueue[0]: " << x(0) << ", " << x(1) << ", " << x(2) << ", " << x(3) << ", " << x(4) << ", " << x(5) << ", " << x(6) << endl;
            if(x(0) <= 0) forcedDecisionsQueue.erase(forcedDecisionsQueue.begin());
            #undef x
        } else if(aiMode == EVOLUTIONARY_NEURAL_NETWORK_AI_MODE && forcedDecisionsQueue.size() == 0){
            // If the AI is free to decide, then decide what to do
            std::vector<float> layerInputs = get_ai_inputs(pAlivesRegions, pCellsHist);
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
            if(stats["EAM_SUN"][0] == 100){
                _speedDir = 0; _speedMode = IDLE_MODE; _doAttack = false;
            } else if(stats["EAM_GND"][0] == 100){
                preplan_random_cell_activity(5, 5, 200, false, _doCloning, pctChanceIdle, pctChanceWalk);
            } else {
                std::vector<int> nearestCellIds = get_nearest_cell_ids(20, pAlivesRegions, pCellsHist);
                if(nearestCellIds.size() == 0 || !_doAttack){
                    preplan_random_cell_activity(5, 5, 1, _doAttack, _doCloning, pctChanceIdle, pctChanceWalk);
                } else {
                    chase_optimal_cell(pCellsHist, nearestCellIds, _doAttack, false, _doCloning); // TODO
                }
            }
        }
        set_ai_outputs(_speedDir, _cloningDir, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
        enforce_valid_cell(false);
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
        // NOTE: If I push a copy of the cell into a new location, I need to update self
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
    void do_energy_transfer(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        if(!isAlive) return;
        //cout << "  energy: " << energy << " ----> ";
        // Energy from the sun
        //  First, calculate which cells are touching the current cell
        std::vector<Cell*> touchingCells = find_touching_cells(pAlives, pAlivesRegions);
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
    void do_energy_decay(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Alive cells don't decay
        if(isAlive || decayPeriod <= 0) return;
        // Energy to cells which are touching the dead cell
        std::vector<Cell*> touchingCells = find_touching_cells(pAlives, pAlivesRegions);
        for(int i = touchingCells.size()-1; i >= 0; i--){
            // Remove dead cells from this list, as they don't receive any of the energy
            if(touchingCells[i]->isAlive == false) touchingCells.erase(touchingCells.begin() + i);
        }
        int rmEnergy = 0; // Energy to give to other cells
        std::vector<int> energyWeight(touchingCells.size());
        for(int i = 0; i < energyWeight.size(); i++){
            energyWeight[i] = touchingCells[i]->stats["EAM_CELLS"][0] * energy / 1000;
            rmEnergy += energyWeight[i];
        }
        if (rmEnergy > energy) {
            float multiplyBy = (float)energy / (float)rmEnergy;
            for(int i = 0; i < touchingCells.size(); i++){
                Cell* pCell = touchingCells[i];
                pCell->energy += multiplyBy * energyWeight[i] * pCell->stats["EAM_CELLS"][0] / 100;
            }
            energy = 0;
            cout << "  energy of dead cell is fully depleted\n";
            return;
        }
        for(int i = 0; i < touchingCells.size(); i++) {
            Cell* pCell = touchingCells[i];
            int cellEnergyGain = energyWeight[i] * pCell->stats["EAM_CELLS"][0] / 100;
            pCell->energy += cellEnergyGain;
            if(cellEnergyGain >= 100) pCell->force_immediate_decision(1, pCell->speedDir, pCell->cloningDirection, IDLE_MODE, pCell->doAttack, pCell->doSelfDestruct, pCell->doCloning);
        }
        energy -= rmEnergy;
        print_scalar_vals("  dead cell energy consumed by predators", rmEnergy);

        // Energy to ground
        rmEnergy = 0;
        if(timeSinceDead % decayPeriod == 0) rmEnergy = decayRate * energy / 100 + 10;
        energy -= rmEnergy;
        print_scalar_vals("  decayed energy", rmEnergy, "decayRate", decayRate, "decayPeriod", decayPeriod, "timeSinceDead", timeSinceDead, "Remaining energy", energy);

        enforce_valid_cell(true);
    }
    void randomize_pos(int _lbX, int _ubX, int _lbY, int _ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, _lbX, _ubX);
        posY = gen_uniform_int_dist(rng, _lbY, _ubY);
        enforce_valid_xyPos();
    }
    Cell* clone_self(int cellNum, std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
    std::vector<Cell*>& pCellsHist, std::vector<Cell*>& pAlives,
    int targetCloningDir = -1, bool randomizeCloningDir = false, bool doMutation = true){
        // The clone's position will be roughly the cell's diameter plus 1 away from the cell
        //Cell* pClone = new Cell(cellNum, CELL_TYPE_GENERIC, pAlivesRegions, pSelf);
        assert(pSelf != NULL);
        energy -= energyCostToClone; //energy -= pSelf->energyCostToClone;

        // Cloning the cell
        Cell* pClone = new Cell();
        pCellsHist.push_back(pClone);
        pAlives.push_back(pClone);
        *pClone = *pSelf; // Almost all quantities should be copied over perfectly
        pClone->define_self(cellNum, pClone, pSelf);
        pClone->initialize_cell(pAlivesRegions, pCellsHist);

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
    void update_forces(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Dead cells aren't affected by force
        if(!isAlive) return;

        // First, calculate which cells are nearby
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();

        // Check each nearby cell for any forces
        // TODO: apply an inward force for stickiness such that the force applied
        //  exactly matches the amount needed to snap the cells to the right distance from each other.
        // TODO: apply a force due to nonliving objects and walls, if applicable
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
        pAttacked->health -= stats["attack"][0];
        attackCooldown = stats["maxAtkCooldown"][0];
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
            std::set<int> attackedCellNums;
            for(auto reg : neighboringRegions){
                for(auto pCell : pAlivesRegions[reg]){
                    if(uniqueCellNum == pCell->uniqueCellNum) continue;
                    if(attackedCellNums.count(pCell->uniqueCellNum)) continue;
                    if(calc_distance_from_point(pCell->posX, pCell->posY) <= (float)(stats["dia"][0] + pCell->stats["dia"][0] + 0.1) / 2){
                        attack_cell(pCell);
                        //forcedDecisionsQueue.insert(forcedDecisionsQueue.begin(), {1, speedDir, cloningDirection, speedMode, doAttack, doSelfDestruct, doCloning});
                        attackedCellNums.insert(pCell->uniqueCellNum);
                    }
                }
            }
        }
        if(doCloning && energy > 1.2*energyCostToClone && pAlives.size() < cellLimit.val){
            Cell* pCell = clone_self(pCellsHist.size(), pAlivesRegions, pCellsHist, pAlives, cloningDirection);
            
            // This code is already inside of the clone_self(...) function
            //pCellsHist.push_back(pCell);
            //pAlives.push_back(pCell);
        }
        enforce_valid_cell(true);
    }
    // Generate random-ish movement that looks reasonable for a cell to do
    void preplan_random_cell_activity(int pctChanceToChangeDir, int pctChanceToChangeSpeed, int numFrames,
    bool _enableAttack, bool _enableCloning, int _pctChanceIdle = 33, int _pctChanceWalk = 33){
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
                    if(_rngPct < _pctChanceIdle) newSpeedMode = IDLE_MODE;
                    else if(_rngPct < pctChanceIdle + _pctChanceWalk) newSpeedMode = WALK_MODE;
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
        int dx = targetX - posX, dy = targetY - posY;
        float distance = calc_distance_from_point(targetX, targetY);
        if(distance == 0) return 0;
        int ans = (int)arc_cos_deg(abs(dx), distance);
        //print_scalar_vals("  distance", distance, "dx", dx, "dy", dy, "acos(|dx|, distance)", ans);
        if(dx >= 0){
            if(dy >= 0) ans = ans;
            ans = 360 - ans;
            //print_scalar_vals("  (dx >= 0) ans", ans);
        } else {
            if(dy >= 0) ans = 180 - ans;
            ans = 180 + ans;
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
    void chase_optimal_cell(std::vector<Cell*> pCellsHist,
    std::vector<int>& nearestCellIds, bool _doAttack, bool _doSelfDestruct, bool _doCloning){
        clear_forced_decisions();

        #define get_pCell(cellId) pCellsHist[cellId];

        // First, mark each cell with a number and record the largest (best) number
        std::vector<int> cellRanks;
        for(auto cellId : nearestCellIds){
            Cell* pCell = get_pCell(cellId);
            int rank = 0;
            int deadCoef = 16000, plantCoef = 8000, gndCoef = 4000, balancedCoef = 2000, predatorCoef = 1000;
            int speedCoef = 10, distanceCoef = 1, doAttackCoef = 0;

            // Assume the dead cell hasn't moved since dying
            if(pCell->isAlive == false) assert(pCell->speedMode = IDLE_MODE);
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
        int minRank = cellRanks[0];
        for(auto rank : cellRanks) minRank = min_int(minRank, rank);
        for(int i = cellRanks.size()-1; i >= 0; i--){
            if(cellRanks[i] != minRank) nearestCellIds.erase(nearestCellIds.begin() + i);
        }
        // Get the optimal speedDir and speed for the first cell in the list
        int cellIdToChase = nearestCellIds[0];
        Cell* pCellToChase = get_pCell(cellIdToChase);
        int xNext = pCellToChase->posX + pCellToChase->get_speed()*cos_deg(pCellToChase->speedDir);
        int yNext = pCellToChase->posY + pCellToChase->get_speed()*sin_deg(pCellToChase->speedDir);
        int optimalSpeedDir = get_optimal_speedDir_to_point(xNext, yNext);
        // Pursue the relevant cell for 1 more frame
        int _speedDir = optimalSpeedDir;
        int targetDistance = calc_distance_from_point(pCellToChase->posX, pCellToChase->posY);
        int targetSpeed = find_closest_value(targetDistance, {stats["speedIdle"][0], stats["speedWalk"][0], stats["speedRun"][0]});
        int _speedMode;
        if(targetSpeed == stats["speedIdle"][0]) _speedMode = stats["speedIdle"][0];
        if(targetSpeed == stats["speedWalk"][0]) _speedMode = stats["speedWalk"][0];
        if(targetSpeed == stats["speedRun"][0])  _speedMode = stats["speedRun"][0];
        force_decision(1, _speedDir, rand() % 360, _speedMode, _doAttack, _doSelfDestruct, _doCloning);

        #undef get_pCell
    }
    void kill_self(std::vector<Cell*>& pAlives, std::vector<Cell*>& _pDeads, int i_pAlive){
        assert(isAlive);
        decayPeriod = 1;
        decayRate = 2;
        energy += energyCostToClone;
        timeSinceDead = 0;
        isAlive = false;
        speedMode = IDLE_MODE;
        pAlives.erase(pAlives.begin() + i_pAlive);
        _pDeads.push_back(pSelf);
    }
    // TODO: replace the DeadCell* type with Cell*
    void remove_this_dead_cell_if_depleted(std::vector<Cell*>& _pDeads, int iDead){
        if(isAlive || energy > 0) return;
        assert(_pDeads[iDead] == pSelf);
        _pDeads.erase(_pDeads.begin() + iDead);
        delete pSelf;
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
        // TODO: If part of a cell is not fully rendered, render a copy of it on the other side
        draw_texture(pCellSkeleton, drawX, drawY, drawSize, drawSize);
        // Draw the health and energy on top of this
        SDL_Texture* energyTex = findSDLTex(energy * 100 / stats["maxEnergy"][0], P_CELL_ENERGY_TEX);
        draw_texture(energyTex, drawX, drawY, drawSize, drawSize);
        SDL_Texture* healthTex = findSDLTex(100*health/stats["maxHealth"][0], P_CELL_HEALTH_TEX);
        draw_texture(healthTex, drawX, drawY, drawSize, drawSize);
        if(doAttack && stats["attack"][0] > 0)  draw_texture(pDoAttackTex,  drawX, drawY, drawSize, drawSize);
        if(doCloning) draw_texture(pDoCloningTex, drawX, drawY, drawSize, drawSize);
        std::vector<SDL_Texture*> EAM_Tex = findEAMTex();
        for(auto tex : EAM_Tex) draw_texture(tex, drawX, drawY, drawSize, drawSize);
        if(drawVisionRadius && stats["visionDist"][0] > 0){
            int drawCenterX = drawScaleFactor*(posX + 0.5);
            int drawCenterY = drawScaleFactor*(posY + 0.5);
            int drawRadius = stats["visionDist"][0]*drawScaleFactor;
            draw_regular_polygon(drawCenterX, drawCenterY, drawRadius, 32, {0xff, 0xff, 0xff, 0x40});
        }
    }
};











// The following struct vars do NOT exist in (alive) Cells:
//  int timeSinceDead;
//  int dia; // stored in stats["dia"]
//  int decayRate;
//  int decayPeriod;
struct DeadCell {
    // Identity
    DeadCell* pSelf = NULL; // Place a pointer to self here
    //  NOTE: The user MUST define the pointer after placing it into a vector to be permanently kept as data.
    //  Otherwise, it will be the wrong pointer.
    Cell* pOldSelf = NULL; // This cell when it was alive
    int uniqueCellNum = -1;

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
    void kill_cell(Cell* pAlive, DeadCell* pDead, int i_pAlive, std::vector<Cell*>& pAlives, std::vector<DeadCell*>& pDeads) {
        //DeadCell* pDead = new DeadCell;
        pSelf = pDead;
        pOldSelf = pAlive;
        uniqueCellNum = pAlive->uniqueCellNum;
        decayPeriod = 20;
        decayRate = 5;
        dia = pAlive->stats["dia"][0];
        energy = pAlive->energy + pAlive->energyCostToClone;
        timeSinceDead = 1;
        posX = pAlive->posX;
        posY = pAlive->posY;
        pAlive->isAlive = false;
        pAlive->speedMode = IDLE_MODE;

        pAlives.erase(pAlives.begin() + i_pAlive);
        pDeads.push_back(pDead);
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
        decayRate = saturate_int(decayRate, 0, 100);
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
    // Ignore dead cells
    std::vector<Cell*> find_touching_cells(std::vector<Cell*>& pAlives,
    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        std::set<int> checkedCells;
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
                if(checkedCells.count(pCell->uniqueCellNum)) continue;
                if(pCell->calc_distance_from_point(posX, posY) > (float)(dia + pCell->stats["dia"][0] + 0.1) / 2) continue;
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
            energyWeight[i] = touchingCells[i]->stats["EAM_CELLS"][0] * energy / 1000;
            rmEnergy += energyWeight[i];
        }
        if (rmEnergy > energy) {
            float multiplyBy = (float)energy / (float)rmEnergy;
            for(int i = 0; i < touchingCells.size(); i++){
                Cell* pCell = touchingCells[i];
                pCell->energy += multiplyBy * energyWeight[i] * pCell->stats["EAM_CELLS"][0] / 100;
            }
            energy = 0;
            return;
        }
        for(int i = 0; i < touchingCells.size(); i++) {
            Cell* pCell = touchingCells[i];
            int cellEnergyGain = energyWeight[i] * pCell->stats["EAM_CELLS"][0] / 100;
            pCell->energy += cellEnergyGain;
            if(cellEnergyGain >= 100) pCell->force_immediate_decision(1, pCell->speedDir, pCell->cloningDirection, IDLE_MODE, pCell->doAttack, pCell->doSelfDestruct, pCell->doCloning);
        }
        energy -= rmEnergy;

        // Energy to ground
        rmEnergy = 0;
        if(timeSinceDead % decayPeriod == 0) rmEnergy = decayRate * energy / 100 + 10;
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
        int drawX = drawScaleFactor*(posX + 0.5 - (float)dia/2);
        int drawY = drawScaleFactor*(posY + 0.5 - (float)dia/2);
        int drawSize = drawScaleFactor*dia;
        draw_texture(pDeadCellTex, drawX, drawY, drawSize, drawSize);
    }
};




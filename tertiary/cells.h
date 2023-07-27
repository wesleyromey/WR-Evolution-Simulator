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
    Cell* pParent = NULL;
    bool id[ID_LEN]; // An identifier which every creature "knows"
    //  which may change through random mutations, but ultimately
    //  does NOT change for an individual after birth
    int uniqueCellNum = -1;
    //  This is unique because pCellsHist never gets deleted from until the simulation ends

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


    // Stats
    std::map<std::string, std::vector<int>> stats;

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
    void gen_stats_random(int _cellType, std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Random generation from scratch
        assert(pSelf != NULL);

        #define stat_init(lb, ub) gen_uniform_int_dist(rng, lb, ub)
        // Initialize all stats
        // Notation: Pct1k == one thousandth of the entire value
        // {{"stat1", {val, lb, ub, mutationPct1kChance, mutationMaxPct1kChange}}, ...}
        //  Whenever a mutation occurs, the stat must be able to change by at least 1 (unless lb == ub)
        stats["attack"]         = { stat_init(1,   1),     0, 10000, 100, 100}; // (0,3)
        stats["dex"]            = {                 0,     0,     0,   0,   0}; // TODD: Add this as an actual stat
        stats["dia"]            = { stat_init(2,   2),     1,    10, 100, 100}; // (2,10)
        stats["EAM_SUN"]        = { stat_init(0, 100),     0,   100, 100, 100}; // (0,100)
        stats["EAM_GND"]        = { stat_init(0, 100),     0,   100, 100, 100}; // (0,100)
        stats["EAM_CELLS"]      = { stat_init(0, 100),     0,   100, 100, 100}; // (0,100)
        stats["initEnergy"]     = {              1000,  1000,  1000,   0,   0}; // (0,10000) // TODO: Convert initEnergy to a mutatable stat
        stats["maxAtkCooldown"] = {                10,    10,    10,   0,   0}; // 10
        stats["maxEnergy"]      = { 5000*stats["dia"][0],  1, 90000,   0,   0}; // 5000*stats["dia"]
        stats["maxHealth"]      = { stat_init(1,   1),     1, 10000, 100, 100}; // (1,10)
        // TODO: Add stats for the AI and its relevant mutation rate
        stats["mutationRate"]   = { stat_init(0,   0),     0,  1000, 100, 100}; // (0,1000)
        // TODO: change speedIdle, speedWalk, and speedRun to a "maxSpeed" stat and change speed to a continuously varying decision
        stats["speedIdle"]      = {                 0,     0,     0,   0,   0}; // 0
        stats["speedWalk"]      = {                 1,     0,     2, 100, 100}; // (0, 1)
        stats["speedRun"]       = {                 2,     0,   100, 100, 100}; // (0, 100)
        stats["stickiness"]     = {                 0,     0,     0,   0,   0}; // 0
        stats["visionDist"]     = { stat_init(5,   5),     0,  1000, 100, 100}; // (0, 10)
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
        initialize_cell(pAlivesRegions);
        enforce_valid_cell(true);
        #undef stat_init
    }
    /*
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
    */
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
        while(EAM_sum != REQ_EAM_SUM){
            int increment = sign(REQ_EAM_SUM - EAM_sum);
            switch(rand() % NUM_EAM_ELE){
                case 0:
                stats["EAM_SUN"][0] += increment;
                break;
                case 1:
                stats["EAM_GND"][0] += increment;
                break;
                case 2:
                stats["EAM_CELLS"][0] += increment;
                break;
            }
            EAM_sum += increment;
        }
        assert(calc_EAM_sum() == REQ_EAM_SUM);
    }
    void enforce_bounds(int& val, int lb, int ub){
        assert(lb <= ub);
        val = saturate_int(val, lb, ub);
    }
    // Only consider the nearest cells within the cell's field of view
    std::vector<Cell*> get_nearest_cells(int maxNumCellsToReturn,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // visionDist: The distance the cell can see
        int xReg = xyRegion.first, yReg = xyRegion.second;
        int reg_dX = CELL_REGION_SIDE_LEN, reg_dY = CELL_REGION_SIDE_LEN;
        
        // A region counts as too far if it its nearest point is out of range
        int visionDist_NumRegX = stats["visionDist"][0] / reg_dX + 1;
        int visionDist_NumRegY = stats["visionDist"][0] / reg_dY + 1;
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
                if(distXY <= stats["visionDist"][0] && pCell != pSelf){
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
    // NOTE: This function also determines what the AI inputs are
    std::vector<float> get_ai_inputs(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
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
        //  (a) Find out which 10 cells are closest
        //  (b) For now, we just care about their id similarity
        int maxNumCellsSeen = 10;
        std::vector<Cell*> pNearestCells = get_nearest_cells(maxNumCellsSeen, pAlivesRegions);
        for(int i = 0; i < maxNumCellsSeen; i++){
            float ageOther = 0, attackCooldownOther = 0;
            float healthOther = 0, energyOther = 0;
            float idSimilarity = 0;
            float relDist = 0, relDirOther = 0, relSpeedRadial = 0, relSpeedTangential = 0;
            if(i < pNearestCells.size()){
                Cell* pCell = pNearestCells[i];
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
        // Cloning
        energyCostToClone = 0;
        energyCostToCloneMap["base"] = StrExprInt::solve(ENERGY_COST_TO_CLONE["base"], {{"x", 2*stats["initEnergy"][0]}});
        energyCostToCloneMap["visionDist"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["visionDist"], {{"x", stats["visionDist"][0]}, {"size", size}});
        if(stats["stickiness"][0]) energyCostToCloneMap["stickiness"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["stickiness"], {{"x", stats["stickiness"][0]}, {"size", size}});
        else energyCostToCloneMap["stickiness"] = 0;
        if(stats["attack"][0]) energyCostToCloneMap["attack"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["attack"], {{"x", stats["attack"][0]}, {"size", size}});
        else energyCostToCloneMap["attack"] = 0;
        energyCostToCloneMap["size"] =
            StrExprInt::solve(ENERGY_COST_TO_CLONE["size"], {{"x", size}, {"size", size}});
        for(auto item : energyCostToCloneMap) energyCostToClone += item.second;

        // Surviving (per second)
        energyCostPerFrame = 0;
        energyCostPerSecMap["base"] = StrExprInt::solve(ENERGY_COST_PER_USE["base"],
            {{"x", -1}, {"size", size}});
        energyCostPerSecMap["visionDist"] = StrExprInt::solve(ENERGY_COST_PER_USE["visionDist"],
            {{"x", stats["visionDist"][0]}, {"size", size}});
        energyCostPerSecMap["stickiness"] = StrExprInt::solve(ENERGY_COST_PER_USE["stickiness"],
            {{"x", stats["stickiness"][0]}, {"size", size}});
        energyCostPerSecMap["mutationRate"] = StrExprInt::solve(ENERGY_COST_PER_USE["mutationRate"],
            {{"x", stats["mutationRate"][0]}, {"size", size}});
        energyCostPerSecMap["age"] = StrExprInt::solve(ENERGY_COST_PER_USE["age"],
            {{"x", age}, {"size", size}});
        energyCostPerSecMap["maxHealth"] = StrExprInt::solve(ENERGY_COST_PER_USE["maxHealth"],
            {{"x", stats["maxHealth"][0]}, {"size", size}});
        for(auto item : energyCostPerSecMap) energyCostPerFrame += item.second;
        energyCostPerFrame /= TICKS_PER_SEC;

        // Surviving (per frame) and using abilities
        energyCostPerUse["speedRun"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],
            {{"x", stats["speedRun"][0]}, {"size", size}});
        energyCostPerUse["speedWalk"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],
            {{"x", stats["speedWalk"][0]}, {"size", size}});
        energyCostPerUse["speedIdle"] = StrExprInt::solve(ENERGY_COST_PER_USE["speed"],
            {{"x", stats["speedIdle"][0]}, {"size", size}});
        energyCostPerUse["attack"] = StrExprInt::solve(ENERGY_COST_PER_USE["attack"],
            {{"x", stats["attack"][0]}, {"size", size}});
    }
    void consume_energy_per_frame(){
        update_energy_costs();
        energy -= energyCostPerFrame;
        if(speedMode == IDLE_MODE) energy -= energyCostPerUse["speedIdle"] / TICKS_PER_SEC;
        if(speedMode == WALK_MODE) energy -= energyCostPerUse["speedWalk"] / TICKS_PER_SEC;
        if(speedMode == RUN_MODE)  energy -= energyCostPerUse["speedRun"] / TICKS_PER_SEC;
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
        speedDir %= 360;
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
    }
    void initialize_cell(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        assert(pSelf != NULL);
        age = 0;
        attackCooldown = stats["maxAtkCooldown"][0];
        energy = stats["initEnergy"][0];
        health = stats["maxHealth"][0];
        if(pParent == NULL) init_ai(pAlivesRegions);
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
    void decide_next_frame(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
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
            if(stats["EAM_SUN"][0] == 100){
                _speedDir = 0; _speedMode = IDLE_MODE; _doAttack = false;
            } else if(stats["EAM_GND"][0] == 100){
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
        // Energy from the sun
        //  First, calculate which cells are touching the current cell
        std::vector<Cell*> touchingCells = find_touching_cells(pAlives, pAlivesRegions);
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
        //cout << StrExprInt::solve(ENERGY_COST_PER_USE["overcrowding"],
        //    {{"overcrowdingEnergyCoef", overcrowdingEnergyCoef.val},
        //    {"x", sumOfCellSizes-size}, {"size", size}}) << ", ";

        // Enforce energy constraints
        energy = min_int(energy, stats["maxEnergy"][0]);
        enforce_valid_cell(false);
    }
    void randomize_pos(int _lbX, int _ubX, int _lbY, int _ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, _lbX, _ubX);
        posY = gen_uniform_int_dist(rng, _lbY, _ubY);
        enforce_valid_xyPos();
    }
    Cell* clone_self(int cellNum, std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions,
    int targetCloningDir = -1, bool randomizeCloningDir = false, bool doMutation = true){
        // The clone's position will be roughly the cell's diameter plus 1 away from the cell
        //Cell* pClone = new Cell(cellNum, CELL_TYPE_GENERIC, pAlivesRegions, pSelf);
        assert(pSelf != NULL);
        energy -= energyCostToClone; //energy -= pSelf->energyCostToClone;

        // Cloning the cell
        Cell* pClone = new Cell();
        *pClone = *pSelf; // Almost all quantities should be copied over perfectly
        pClone->define_self(cellNum, pClone, pSelf);
        pClone->initialize_cell(pAlivesRegions);

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
        // Apply the forces which should already calculated
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
            for(auto reg : neighboringRegions){
                for(auto pCell : pAlivesRegions[reg]){
                    if(uniqueCellNum == pCell->uniqueCellNum) continue;
                    if(calc_distance_from_point(pCell->posX, pCell->posY) <= (float)(stats["dia"][0] + pCell->stats["dia"][0] + 0.1) / 2){
                        attack_cell(pCell);
                    }
                }
            }
        }
        if(doCloning && energy > 1.2*energyCostToClone && pAlives.size() < cellLimit.val){
            Cell* pCell = clone_self(pCellsHist.size(), pAlivesRegions, cloningDirection);
            pCellsHist.push_back(pCell);
            pAlives.push_back(pCell);
        }
        enforce_valid_cell(true);
    }
    // Generate random-ish movement that looks reasonable for a cell to do
    void preplan_random_cell_activity(int pctChanceToChangeDir, int pctChanceToChangeSpeed, int numFrames,
    bool _enableAttack, bool _enableCloning){
        int newSpeedDir = pSelf->speedDir, newSpeedMode = pSelf->speedMode;
        int numFramesSinceLastDecision = 0;
        bool changedSpeedDir = false;
        while(numFrames > 0){
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
            rank += predatorCoef * (pCell->stats["EAM_CELLS"][0] == 100);
            rank += balancedCoef * (pCell->stats["EAM_SUN"][0] < 100 && pCell->stats["EAM_GND"][0] < 100 && pCell->stats["EAM_CELLS"][0] < 100);
            rank += gndCoef * (pCell->stats["EAM_GND"][0] == 100);
            rank += plantCoef * (pCell->stats["EAM_SUN"][0] == 100);
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
        int targetSpeed = find_closest_value(targetDistance, {stats["speedIdle"][0], stats["speedWalk"][0], stats["speedRun"][0]});
        int _speedMode;
        if(targetSpeed == stats["speedIdle"][0]) _speedMode = stats["speedIdle"][0];
        if(targetSpeed == stats["speedWalk"][0]) _speedMode = stats["speedWalk"][0];
        if(targetSpeed == stats["speedRun"][0])  _speedMode = stats["speedRun"][0];
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
        // TODO: If part of a cell is not fully rendered, render a copy of it on the other side
        int drawX = drawScaleFactor*(posX - stats["dia"][0]/2);
        int drawY = drawScaleFactor*(posY - stats["dia"][0]/2);
        int drawSize = drawScaleFactor*stats["dia"][0];
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
        dia = pAlive->stats["dia"][0];
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
            pCell->energy += energyWeight[i] * pCell->stats["EAM_CELLS"][0] / 100;
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





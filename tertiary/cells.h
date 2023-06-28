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
    std::vector<int> nodesPerLayer = {-1, 10, -1}; // Fill in the hidden layer (middle) values.
    //  Leave the first (input) and last (output) blank

    // Internal timers
    int age = 0; // Relative to birth (limits lifespan)
    int attackCooldown = 0; // When this reaches 0, an attack can be done
    
    // Variables intrinsic to the creature's "genome"
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

    // Bounds
    int maxEnergy = 10000;
    //  TODO: Multiply this by size
    int maxMutationRate = 10000;
    int maxDia = 30;
    int maxAttackCooldown = 10;

    // Struct-specific methods
    std::vector<Cell*> find_touching_cells(std::vector<Cell*>& pAlives,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
                if(pCell->calc_distance_from_point(posX, posY) <= (float)(dia + pCell->dia + 0.1) / 2){
                    ans.push_back(pCell);
                }
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
            while(neighboringRegions[i].first < 0) neighboringRegions[i].first += CELL_REGION_NUM_X;
            while(neighboringRegions[i].second < 0) neighboringRegions[i].second += CELL_REGION_NUM_Y;
            neighboringRegions[i].first %= CELL_REGION_NUM_X;
            neighboringRegions[i].second %= CELL_REGION_NUM_Y;
        }
        return neighboringRegions;
    }
    // NOTE: this function does NOT have access to the entire list of cells,
    //  so a separate function needs to be run to organize the cells into
    //  region-based lists
    void assign_self_to_xyRegion(){
        xyRegion.first = saturate_int(posX / CELL_REGION_SIDE_LEN, 0, CELL_REGION_NUM_X-1);
        xyRegion.second = saturate_int(posY / CELL_REGION_SIDE_LEN, 0, CELL_REGION_NUM_Y-1);
    }
    void teleport_self(int target_x, int target_y){
        posX = target_x;
        posY = target_y;
        enforce_valid_xyPos();
    }
    void enforce_wrap_around_x(){
        while(posX < 0) posX += UB_X;
        posX %= UB_X;
        assign_self_to_xyRegion();
    }
    void enforce_wrap_around_y(){
        while(posY < 0) posY += UB_Y;
        posY %= UB_Y;
        assign_self_to_xyRegion();
    }
    void enforce_valid_xyPos(){
        if(WRAP_AROUND_X) enforce_wrap_around_x();
        else posX = saturate_int(posX, 0, UB_X);
        if(WRAP_AROUND_Y) enforce_wrap_around_y();
        else posY = saturate_int(posY, 0, UB_Y);
        assign_self_to_xyRegion();
    }
    void update_size(){
        size = PI*dia*dia/4 + 0.5; // size is an int
    }
    int calc_EAM_sum(){
        int EAM_sum = 0;
        for(int i = 0; i < NUM_EAM_ELE; i++){
            EAM_sum += EAM[i];
        }
        return EAM_sum;
    }
    bool verify_EAM_constraints(){
        // Constraint: EAM[i] >= 0
        for(int i = 0; i < NUM_EAM_ELE; i++){
            if(EAM[i] < 0) return false;
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
    //  Return the bounds of the regions (lbX, ubX, lbY, ubY)
    std::vector<Cell*> get_nearest_cells(int maxNumCellsToReturn,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // visionDist: The distance the cell can see
        int xReg = xyRegion.first, yReg = xyRegion.second;
        int reg_dX = CELL_REGION_SIDE_LEN, reg_dY = CELL_REGION_SIDE_LEN;
        int lbX = posX - visionDist, ubX = posX + visionDist;
        int lbY = posY - visionDist, ubY = posY + visionDist;
        
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
        std::vector<Cell*> nearestCells = get_nearest_cells(maxNumCellsSeen, pAlivesRegions);
        for(int i = 0; i < maxNumCellsSeen; i++){
            int relDist = 0, relDirOther = 0;
            int relSpeedRadial = 0, relSpeedTangential = 0;
            int healthOther = 0, energyOther = 0, ageOther = 0;
            int idSimilarity = 0;
            if(i < nearestCells.size()){
                Cell* pCell = nearestCells[i];
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
    void set_ai_outputs(int _speedDir, int _cloningDirection, char _speedMode,
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
    std::tuple<std::vector<int>, std::vector<char>, std::vector<bool>> get_ai_outputs(){
        std::vector<int> intVec;
        int _numAiOutputs = 0;
        intVec.push_back(speedDir); _numAiOutputs++;
        intVec.push_back(cloningDirection); _numAiOutputs++;
        std::vector<char> charVec;
        charVec.push_back(speedMode); _numAiOutputs++;
        std::vector<bool> boolVec;
        boolVec.push_back(doAttack); _numAiOutputs++;
        boolVec.push_back(doSelfDestruct); _numAiOutputs++;
        boolVec.push_back(doCloning); _numAiOutputs++;
        if(nodesPerLayer[nodesPerLayer.size()-1] < 0) {
            nodesPerLayer[nodesPerLayer.size()-1] = _numAiOutputs;
        }
        return {intVec, charVec, boolVec};
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
        energyCostPerUse["attack"] = StrExprInt::solve(ENERGY_COST_PER_USE["attack"],
            {{"x", attack}, {"size", size}});
    }
    void consume_energy_per_frame(){
        energy -= energyCostPerFrame;
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
    void enforce_valid_cell(){
        assert(uniqueCellNum >= 0);
        while(speedDir < 0) speedDir += 360*100;
        speedDir %= 360;
        saturate_int(dia, 0, maxDia);
        saturate_int(energy, 0, maxEnergy);
        saturate_int(mutationRate, 0, maxMutationRate);
        while(cloningDirection < 0) cloningDirection += 360*100;
        cloningDirection %= 360;
        assert(speedMode == IDLE_MODE || speedMode == WALK_MODE || speedMode == RUN_MODE);
        enforce_valid_xyPos();
        enforce_EAM_constraints();
        enforce_valid_ai();
        update_size();
        update_energy_costs();
    }
    void set_int_stats(std::map<std::string, int>& varVals){
        // TODO: Include the ability to set the aiNetwork and nodesPerLayer
        // Only contains functionality for the more important stats
        int lenVarVals = 0;
        if(varVals.count("age"))            {lenVarVals++; age = varVals["age"];}
        if(varVals.count("attack"))         {lenVarVals++; attack = varVals["attack"];}
        if(varVals.count("attackCooldown")) {lenVarVals++; attackCooldown = varVals["attackCooldown"];}
        if(varVals.count("cloningDirection")) {lenVarVals++; cloningDirection = varVals["cloningDirection"];}
        if(varVals.count("dia"))            {lenVarVals++; dia = varVals["dia"];}
        if(varVals.count("doAttack"))       {lenVarVals++; doAttack = varVals["doAttack"];}
        if(varVals.count("doSelfDestruct")) {lenVarVals++; doSelfDestruct = varVals["doSelfDestruct"];}
        if(varVals.count("EAM[EAM_CELLS]")) {lenVarVals++; EAM[EAM_CELLS] = varVals["EAM[EAM_CELLS]"];}
        if(varVals.count("EAM[EAM_GND]"))   {lenVarVals++; EAM[EAM_GND] = varVals["EAM[EAM_GND]"];}
        if(varVals.count("EAM[EAM_SUN]"))   {lenVarVals++; EAM[EAM_SUN] = varVals["EAM[EAM_SUN]"];}
        if(varVals.count("energy"))         {lenVarVals++; energy = varVals["energy"];}
        if(varVals.count("health"))         {lenVarVals++; health = varVals["health"];}
        if(varVals.count("maxHealth"))      {lenVarVals++; maxHealth = varVals["maxHealth"];}
        if(varVals.count("mutationRate"))   {lenVarVals++; mutationRate = varVals["mutationRate"];}
        if(varVals.count("posX"))           {lenVarVals++; posX = varVals["posX"];}
        if(varVals.count("posY"))           {lenVarVals++; posY = varVals["posY"];}
        if(varVals.count("speedDir"))       {lenVarVals++; speedDir = varVals["speedDir"];}
        if(varVals.count("speedMode"))      {lenVarVals++; speedMode = varVals["speedMode"];}
        if(varVals.count("speedRun"))       {lenVarVals++; speedRun = varVals["speedRun"];}
        if(varVals.count("speedWalk"))      {lenVarVals++; speedWalk = varVals["speedWalk"];}
        if(varVals.count("stickiness"))     {lenVarVals++; stickiness = varVals["stickiness"];}
        if(varVals.count("visionDist"))     {lenVarVals++; visionDist = varVals["visionDist"];}

        // Ensure that varVals does NOT contain values not accounted for in this function
        assert(lenVarVals == varVals.size());
        
        // Ensure we update all dependent variables
        enforce_valid_cell();
    }
    void init_ai(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // NOTE: Do NOT use this function until all the inputs are initialized
        std::vector<float> aiInputs = get_ai_inputs(pAlivesRegions);
        std::tuple<std::vector<int>, std::vector<char>, std::vector<bool>> aiOutputs = get_ai_outputs();

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
    void decide_next_frame(std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        // Modify the values the creature can directly control based on the ai
        //  i.e. the creature decides what to do based on this function
        //std::cout << aiNetwork.size() << ", " << nodesPerLayer.size() << std::endl;
        enforce_valid_ai();
        std::vector<float> layerInputs = get_ai_inputs(pAlivesRegions);
        for(int layerNum = 1; layerNum < aiNetwork.size(); layerNum++){
            layerInputs = do_forward_prop_1_layer(layerInputs, layerNum);
        }
        update_timers();
        // Format and set the outputs
        int _speedDir = saturate_int((int)layerInputs[0], 0, 359);
        int _cloningDirection = saturate_int((int)layerInputs[2], 0, 359);
        char _speedMode = (char)saturate_int((char)layerInputs[3], IDLE_MODE, RUN_MODE);
        bool _doAttack = (layerInputs[4] >= 0);
        bool _doSelfDestruct = (layerInputs[5] >= 1); // If this condition is too easy to trigger, then cells die too easily
        bool _doCloning = (layerInputs[6] >= 0);
        set_ai_outputs(_speedDir, _cloningDirection, _speedMode, _doAttack, _doSelfDestruct, _doCloning);
    }
    void mutate_ai(){
        float prob = (float)mutationRate / maxMutationRate;
        for(int i = 0; i < aiNetwork.size(); i++){
            for(int j = 0; j < aiNetwork[i].size(); j++){
                aiNetwork[i][j].mutate_node(mutationRate, prob);
            }
        }
    }
    void mutate_stats(){
        // Random mutation based on parent's mutation rate
        float probDefault = (float)parent->mutationRate / (float)parent->maxMutationRate;
        speedWalk = gen_normal_int_dist_special(rng, probDefault, parent->speedWalk, 1, 1, INT_MAX);
        speedRun = gen_normal_int_dist_special(rng, probDefault, parent->speedRun, 1, parent->speedRun + 1, INT_MAX);
        visionDist = gen_normal_int_dist_special(rng, probDefault, parent->visionDist, 1 + mutationRate/50, 0, INT_MAX);
        stickiness = 0;
        mutationRate = gen_normal_int_dist_special(rng, 1, parent->mutationRate, 5 + parent->mutationRate/50, 0, maxMutationRate);
        maxHealth = gen_normal_int_dist_special(rng, probDefault, parent->maxHealth, 1 + parent->maxHealth/20, 1, INT_MAX);
        health = maxHealth;
        attack = gen_normal_int_dist_special(rng, probDefault, parent->attack, 1 + parent->attack/20, 0, INT_MAX);
        dia = gen_normal_int_dist_special(rng, probDefault, parent->dia, 1 + parent->dia/20, 1, maxDia);
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
    }
    void define_self(Cell* pCell){
        // NOTE: If I push a copy of the cell into a new location, I need to update self
        pSelf = pCell;
    }
    void set_initEnergy(int val, bool setEnergy = true){
        initEnergy = val;
        if(setEnergy) energy = initEnergy;
    }
    void gen_stats_random(int cellNum,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions, 
            Cell* pCell = NULL){
        // Random generation from scratch
        pSelf = pCell;
        parent = NULL;
        uniqueCellNum = cellNum;
        speedWalk = gen_uniform_int_dist(rng, 1, 3);
        speedRun = gen_uniform_int_dist(rng, speedWalk + 1, 3*speedWalk);
        visionDist = 0;
        stickiness = 0;
        mutationRate = 1000;
        maxHealth = gen_uniform_int_dist(rng, 1, 3);
        health = maxHealth;
        attack = 1;
        dia = gen_uniform_int_dist(rng, 1, 3);
        set_initEnergy(gen_uniform_int_dist(rng, 500, 2000), true);
        update_size();
        for(int i = 0; i < NUM_EAM_ELE; i++){
            EAM[i] = gen_uniform_int_dist(rng, 0, REQ_EAM_SUM);
        }
        enforce_EAM_constraints();
        for(int i = 0; i < ID_LEN; i++){
            id[i] = std_uniform_dist(rng) < 0.5;
        }
        init_ai(pAlivesRegions);
        update_energy_costs();
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
        if(simGndEnergy[posX][posY] < EAM[EAM_GND]){
            energy += simGndEnergy[posX][posY];
            simGndEnergy[posX][posY] = 0;
        } else {
            energy += EAM[EAM_GND];
            simGndEnergy[posX][posY] -= EAM[EAM_GND];
        }
        
        // Energy from cells which just died -> Add a pointer to the cell to the list
        //  associated with the cell that just died last frame AND is within range
        //  NOTE: This is dealt with in the DeadCell struct

        // Energy loss from overcrowding directly
        // TODO: Create a dex stat to resist this overcrowding
        energy -= overcrowdingEnergyCoef * sumOfCellSizes / size;

        // Enforce energy constraints
        if(energy > maxEnergy) energy = maxEnergy;
    }
    void randomize_pos(int lbX, int ubX, int lbY, int ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, lbX, ubX);
        posY = gen_uniform_int_dist(rng, lbY, ubY);
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
        pClone->energy = 1000; // TODO: change this into a mutatable value
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
            return 0;
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
                    int dX2 = (UB_X - abs(dX)) * -sign(dX); // Result has opposite sign vs dX
                    dX = (abs(dX) < abs(dX2) ? dX : dX2);
                }
                if (WRAP_AROUND_Y) {
                    int dY2 = (UB_Y - abs(dY)) * -sign(dY); // Result has opposite sign vs dX
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
        return;
    }
    void apply_forces(){
        // Apply the forces which should already calculated
        increment_pos(forceX / forceDampingFactor, forceY / forceDampingFactor);
        forceX = 0;
        forceY = 0;
    }
    bool calc_if_cell_is_dead(){
        // Check if cell should be killed
        //  NOTE: Due to how the program is structured, I have to actually kill each dead cell outside of this struct
        return (health <= 0 || energy <= 0 || doSelfDestruct);
    }
    float calc_distance_from_point(int xCoord, int yCoord){
        // Assume -UB_X < posX and -UB_Y < posY
        int xDist = xCoord - posX;
        if(WRAP_AROUND_X && abs(xDist) > abs(xCoord + UB_X - posX)){
            xDist = xCoord + UB_X - posX;
        }
        int yDist = yCoord - posY;
        if(WRAP_AROUND_Y && abs(yDist) > abs(yCoord + UB_Y - posY)){
            yDist = yCoord + UB_Y - posY;
        }
        float ans = std::sqrt(xDist * xDist + yDist * yDist);
        return ans;
    }
    void attack_cell(Cell* pAttacked){
        pAttacked->health -= attack;
        attackCooldown = maxAttackCooldown;
        energy -= energyCostPerUse["attack"];
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
        if(doCloning && energy > energyCostToClone && pAlives.size() < cellLimit){
            Cell* pCell = clone_self(pCellsHist.size(), cloningDirection); // A perfect clone of pSelf
            pCell->mutate_stats();
            pCellsHist.push_back(pCell);
            pAlives.push_back(pCell);
        }
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
    std::vector<SDL_Texture*> findEAMTex(std::vector<int> EAM_weights){
        std::vector<SDL_Texture*> ans;
        // First, check if everything is balanced
        int minEAM = EAM[EAM_SUN], maxEAM = EAM[EAM_SUN];
        for(int i = 0; i < NUM_EAM_ELE; i++){
            if(EAM[i] < minEAM) minEAM = EAM[i];
            if(EAM[i] > maxEAM) maxEAM = EAM[i];
        }
        if(maxEAM <= 1.7*minEAM){
            ans.push_back(P_EAM_TEX["balanced"]);
            return ans;
        }
        // Now that we know it is not perfectly "balanced"
        //  NOTE: there are 4 pixels to color in
        if(EAM_weights[EAM_GND]) ans.push_back(P_EAM_TEX["g4"]);
        if(EAM_weights[EAM_SUN]){
            std::string nextFile = "s" + std::to_string(EAM_weights[EAM_SUN]);
            ans.push_back(P_EAM_TEX[nextFile]);
        }
        if(EAM_weights[EAM_CELLS]){
            std::string nextFile = "c" + std::to_string(EAM_weights[EAM_SUN]);
            ans.push_back(P_EAM_TEX[nextFile]);
        }
        return ans;
    }
    void draw_cell(){
        int drawX = DRAW_SCALE_FACTOR*posX;
        int drawY = DRAW_SCALE_FACTOR*posY;
        int drawSize = DRAW_SCALE_FACTOR*dia;
        draw_texture(pCellSkeleton, drawX, drawY, drawSize, drawSize);
        // Draw the health and energy on top of this
        SDL_Texture* energyTex = findSDLTex(energy, P_CELL_ENERGY_TEX);
        draw_texture(energyTex, drawX, drawY, drawSize, drawSize);
        SDL_Texture* healthTex = findSDLTex(100*health/maxHealth, P_CELL_HEALTH_TEX);
        draw_texture(healthTex, drawX, drawY, drawSize, drawSize);
        if(doAttack)  draw_texture(pDoAttackTex,  drawX, drawY, drawSize, drawSize);
        if(doCloning) draw_texture(pDoCloningTex, drawX, drawY, drawSize, drawSize);
        // EAM
        std::vector<int> EAM_weights = findWeighting(4, EAM, NUM_EAM_ELE);
        std::vector<SDL_Texture*> EAM_Tex = findEAMTex(EAM_weights);
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
    // NOTE: this function does NOT have access to the entire list of cells,
    //  so a separate function needs to be run to organize the cells into
    //  region-based lists
    void assign_self_to_xyRegion(){
        xyRegion.first = saturate_int(posX / CELL_REGION_SIDE_LEN, 0, CELL_REGION_NUM_X-1);
        xyRegion.second = saturate_int(posY / CELL_REGION_SIDE_LEN, 0, CELL_REGION_NUM_Y-1);
    }
    std::vector<std::pair<int, int>> get_neighboring_xyRegions(){
        int xReg = xyRegion.first, yReg = xyRegion.second;
        std::vector<std::pair<int, int>> neighboringRegions = {
            xyRegion,
            {xReg-1, yReg}, {xReg+1, yReg}, {xReg, yReg-1}, {xReg, yReg+1},
            {xReg-1, yReg-1}, {xReg-1, yReg+1}, {xReg+1, yReg-1}, {xReg+1, yReg+1}
        };
        for(int i = 0; i < neighboringRegions.size(); i++){
            while(neighboringRegions[i].first < 0) neighboringRegions[i].first += CELL_REGION_NUM_X;
            while(neighboringRegions[i].second < 0) neighboringRegions[i].second += CELL_REGION_NUM_Y;
            neighboringRegions[i].first %= CELL_REGION_NUM_X;
            neighboringRegions[i].second %= CELL_REGION_NUM_Y;
        }
        return neighboringRegions;
    }
    void enforce_wrap_around_x(){
        while(posX < 0) posX += UB_X;
        posX %= UB_X;
        assign_self_to_xyRegion();
    }
    void enforce_wrap_around_y(){
        while(posY < 0) posY += UB_Y;
        posY %= UB_Y;
        assign_self_to_xyRegion();
    }
    void enforce_valid_xyPos(){
        if(WRAP_AROUND_X) enforce_wrap_around_x();
        else posX = saturate_int(posX, 0, UB_X);
        if(WRAP_AROUND_Y) enforce_wrap_around_y();
        else saturate_int(posY, 0, UB_Y);
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
        pDead->decayPeriod = 10;
        pDead->decayRate = 10;
        pDead->dia = 1;
        pDead->energy = 1000;
        pDead->timeSinceDead = 1;
        pDead->randomize_pos(0, UB_X, 0, UB_Y);
    }
    std::vector<Cell*> find_touching_cells(std::vector<Cell*>& pAlives,
            std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions){
        std::vector<Cell*> ans;
        std::vector<std::pair<int, int>> neighboringRegions = get_neighboring_xyRegions();
        for(auto reg : neighboringRegions){
            for(auto pCell : pAlivesRegions[reg]){
                if(pCell->calc_distance_from_point(posX, posY) <= (float)(dia + pCell->dia + 0.1) / 2){
                    ans.push_back(pCell);
                }
            }
        }
        /*
        for(auto pAlive : pAlives){
            if(pAlive->calc_distance_from_point(posX, posY) <= (float)(dia + pAlive->dia + 0.1) / 2){
                ans.push_back(pAlive);
            }
        }
        */
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
    }
    void randomize_pos(int lbX, int ubX, int lbY, int ubY){
        // lb means lower bound, ub means upper bound,
        // X means x-coordinate, Y means y-coordinate
        posX = gen_uniform_int_dist(rng, lbX, ubX);
        posY = gen_uniform_int_dist(rng, lbY, ubY);
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
        int drawX = DRAW_SCALE_FACTOR*posX;
        int drawY = DRAW_SCALE_FACTOR*posY;
        int drawSize = DRAW_SCALE_FACTOR*dia;
        draw_texture(pDeadCellTex, drawX, drawY, drawSize, drawSize);
    }
};





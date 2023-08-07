// Contains global variables and constants

//using namespace std;
using std::cout, std::endl, std::string;


// Universal Constants
static const double PI = 3.141592653589793238462643;
static const std::set<char> DIGITS = {'0','1','2','3','4','5','6','7','8','9'};
static const std::set<char> LETTERS = {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
};
static const std::set<char> OPERATORS = {'+','-','*','/'};
static const int MAX_INT_DEFAULT = 2147483647; // The max value an int can have in C++


// For functions involving these variables
//#define stdFcnCellInputs { \
//    std::map<std::pair<int,int>, std::vector<Cell*>>& pAlivesRegions, \
//    std::vector<Cell*>& pCellsHist \
//}


// A structure for integer simulation parameters to be held in,
//  where each simulation parameter contains a list of possible values
void update_global_params();
int cellRegionNumUbX = -1, cellRegionNumUbY = -1, drawScaleFactor = -1, ubX_px = -1, ubY_px = -1;
struct SimParamInt{
    int val;
    int valIndex;
    std::vector<int> possibleVals; // Recommended to sort these in ascending order
    std::map<int, int> nextPossibleVal;
    int lastIncrement = 0; // The last input in the increment_val(...) function (regularly resets to 0)

    SimParamInt(int initVal, std::vector<int> possibleValues): val(initVal), possibleVals(possibleValues){
        for(valIndex = 0; valIndex < possibleVals.size(); valIndex++){
            if(val <= possibleVals[valIndex]) break;
        }
        correct_val_and_valIndex();
        //cout << "valIndex = " << valIndex << ", val: " << val << endl;
    }

    SimParamInt(int initVal, int smallestPossibleVal, int largestPossibleVal): val(initVal){
        init_nextPossibleVal_map();
        init_standard_possibleVals(smallestPossibleVal, largestPossibleVal);
        for(valIndex = 0; valIndex < possibleVals.size(); valIndex++){
            if(val <= possibleVals[valIndex]) break;
        }
        correct_val_and_valIndex();
        //cout << "valIndex = " << valIndex << ", val: " << val << endl;
    }

    void init_nextPossibleVal_map(){
        for(int val = 0; val < 10; val++) nextPossibleVal[val] = val+1;
        int pow_int(int root, int exponent);
        #define p(magnitude) pow_int(10,magnitude)

        //MAX_INT_DEFAULT == 2147483647
        //lb = -2*p(9), ub = 2*p(9);
        std::vector<int> valsSoFar;
        nextPossibleVal[0] = 1; valsSoFar.push_back(0);
        for(int mag = 1; mag < 9; mag++){
            nextPossibleVal[1*p(mag)] = 12*p(mag-1);    valsSoFar.push_back(1*p(mag));
            nextPossibleVal[12*p(mag-1)] = 15*p(mag-1); valsSoFar.push_back(12*p(mag-1));
            nextPossibleVal[15*p(mag-1)] = 2*p(mag);    valsSoFar.push_back(15*p(mag-1));
            nextPossibleVal[2*p(mag)] = 3*p(mag);       valsSoFar.push_back(2*p(mag));
            nextPossibleVal[3*p(mag)] = 4*p(mag);       valsSoFar.push_back(3*p(mag));
            nextPossibleVal[4*p(mag)] = 5*p(mag);       valsSoFar.push_back(4*p(mag));
            nextPossibleVal[5*p(mag)] = 6*p(mag);       valsSoFar.push_back(5*p(mag));
            nextPossibleVal[6*p(mag)] = 7*p(mag);       valsSoFar.push_back(6*p(mag));
            nextPossibleVal[7*p(mag)] = 8*p(mag);       valsSoFar.push_back(7*p(mag));
            nextPossibleVal[8*p(mag)] = 9*p(mag);       valsSoFar.push_back(8*p(mag));
            nextPossibleVal[9*p(mag)] = 10*p(mag);      valsSoFar.push_back(9*p(mag));
        }
        nextPossibleVal[1*p(9)] = 12*p(8);  valsSoFar.push_back(1*p(9));
        nextPossibleVal[12*p(8)] = 15*p(8); valsSoFar.push_back(12*p(8));
        nextPossibleVal[15*p(8)] = 2*p(9);  valsSoFar.push_back(15*p(8));
        nextPossibleVal[2*p(9)] = INT_MAX;   valsSoFar.push_back(INT_MAX);
        // Deal with the negative values
        for(int i = 1; i < valsSoFar.size(); i++){
            int val = -valsSoFar[i], nextVal = -valsSoFar[i-1];
            nextPossibleVal[val] = nextVal;
        }
        #undef p
    }

    void correct_val_and_valIndex(){
        while(valIndex < 0) valIndex += possibleVals.size();
        valIndex %= possibleVals.size();
        val = possibleVals[valIndex];
        update_global_params();
    }

    void correct_valIndex(){
        while(valIndex < 0) valIndex += possibleVals.size();
        valIndex %= possibleVals.size();
    }

    void init_standard_possibleVals(int lb, int ub){
        assert(lb <= ub);
        assert(nextPossibleVal.count(lb));
        for(int val = lb; val <= ub; val = nextPossibleVal[val]){
            possibleVals.push_back(val);
        }
    }

    void increment_val(bool doIncrease){
        if(doIncrease) valIndex++;
        else valIndex--;
        correct_val_and_valIndex();
        lastIncrement = (doIncrease ? 1 : -1);
        update_global_params();
    }

    void set_val(int newVal){
        // Disclaimer: This function does NOT ensure the value is valid
        val = newVal;
        correct_valIndex();
        update_global_params();
    }
    
};


// Global Simulation Parameters
SimParamInt initNumCells(0, 0, 10000);
SimParamInt ubX(30, 1, 400); // 120
SimParamInt ubY(20, 1, 400); // 80
// NOTE: The program might not work properly if this is disabled
static const bool WRAP_AROUND_X = true; // Enforce the constraint 0 <= x < ubX.val
// NOTE: The program might not work properly if this is disabled
static const bool WRAP_AROUND_Y = true; // Enforce the constraint 0 <= y < ubY.val
static const int TICKS_PER_SEC = 10;    // Each tick, the new positions are calculated - This may be obsolete or confusing
SimParamInt cellLimit(400, 0, 1000);
// Each cell in the simulator must spend this amount of energy per cell it touches.
//  Increasing this value increases the amount of energy spent due to overcrowding. 
SimParamInt overcrowdingEnergyCoef(0, 0, 1000); // 1
// Energy accumulation for all ground spaces
SimParamInt maxGndEnergy(500, 1, 1000000);
std::vector<std::vector<int>> simGndEnergy;
static const int FRAMES_BETWEEN_GND_ENERGY_ACCUMULATION = 10;
SimParamInt gndEnergyPerIncrease(10, 0, 10000);
SimParamInt defaultMutationChance(100, 0, 1000);
SimParamInt defaultMutationAmt(100, 0, 1000);

// Day-Night Cycle
int energyFromSunPerSec = 0; // This value is automatically updated each frame
int dayNightCycleTime = 0; // Wraps between 0 and dayLenSec - 1
// Max energy which can be accumulated from the sun 
SimParamInt maxSunEnergyPerSec(50, 0, 1000);
// Number of seconds per day (Not sure if this is actually day length in frames
//  or day length in seconds)
SimParamInt dayLenSec(200, 1, 1000000);
static const int DAY_NIGHT_ALWAYS_DAY_MODE = 0;
static const int DAY_NIGHT_BINARY_MODE = 1;
static const int DAY_NIGHT_DEFAULT_MODE = 2;
SimParamInt dayNightMode(DAY_NIGHT_ALWAYS_DAY_MODE, {DAY_NIGHT_ALWAYS_DAY_MODE, DAY_NIGHT_BINARY_MODE, DAY_NIGHT_DEFAULT_MODE});
#define seqOf10(lb, dx) lb, lb+dx, lb+2*dx, lb+3*dx, lb+4*dx, lb+5*dx, lb+6*dx, lb+7*dx, lb+8*dx, lb+9*dx
#define seqOf50(lb, dx) seqOf10(lb,dx), seqOf10(lb+10*dx,dx), seqOf10(lb+20*dx,dx), seqOf10(lb+30*dx,dx), seqOf10(lb+40*dx,dx)
#define seqOf100(lb, dx) seqOf50(lb,dx), seqOf50(lb+50*dx,dx)
// dayNightExponentPct: Applies to day-night cycles where the sun's path is periodic
// Smaller values (closer to 0) mean the sun remains lower in the sky.
// Larger values (to +inf) mean the sun is near its max height for longer
// A value of 175 approximates a sine wave
SimParamInt dayNightExponentPct(100, {seqOf100(0,10), 1000}); // 0 <= EXPONENT < infinity (default = 200)
// The day lasts between dayNightLb and dayNightUb
// The night lasts from dayNightUb to 100 and 0 to dayNightLb
SimParamInt dayNightLbPct(0,{seqOf100(0,1)});
SimParamInt dayNightUbPct(50,{seqOf100(0,1)});
#undef seqOf100
#undef seqOf50
#undef seqOf10

// Initialize SDL
static const char* WINDOW_TITLE = "Evolution Simulator";
//static const int TARGET_SCREEN_WIDTH = 770; // Try 1540 for full screen and 770 for half screen
//static const int TARGET_SCREEN_HEIGHT = 400; // Try 700 for full screeen, or 400 for a quarter screen
static const int WINDOW_WIDTH  = 600;       // Try 1540 for full screen and 770 for half screen
static const int WINDOW_HEIGHT = 450;       // Try 700 for full screeen, or 450 for a quarter screen
// I split cells into square regions so I only have to compare the positions
//  of nearby cells to calculate forces, crowding, interactions, vision, etc.
// If a region near the bottom or the right side is smaller, then it is absorbed
//  into the neighboring region(s)
static const int CELL_REGION_SIDE_LEN = 10;

int mousePosX = 0, mousePosY = 0;
static const Uint32 FRAME_DELAY = 10; // ms; frame rate is (1000/FRAME_DELAY) fps; Default 125
Uint32 frameStart = 0; // The time in ms since the start of the simulation
Uint32 frameTime = 0; // The amount of time the frame lasted for
int frameNum = 0; // The frame number of the simulation

// Manually control cell decisions, frame ticks, etc.
static const int EVOLUTIONARY_NEURAL_NETWORK_AI_MODE = 0, RNG_BASED_AI_MODE = 1;
int aiMode = RNG_BASED_AI_MODE;
int pctChanceIdle = 33, pctChanceWalk = 33, pctChanceRun = 100 - pctChanceIdle - pctChanceWalk; // for RNG_BASED_AI_MODE
bool doCellAi = true;
bool automateEnergy = true;
bool enableAutomaticAttack = true;
bool enableAutomaticSelfDestruct = false;
bool enableAutomaticCloning = true;

// Simulation States: These control the GUI, simulation mode, etc.
static const int SIM_STATE_UNDEF = -1;
static const int SIM_STATE_MAIN_MENU = 0;
static const int SIM_STATE_SKIP_FRAMES = 1;
static const unsigned int AUTO_ADVANCE_DEFAULT = 1000;
static const int SIM_STATE_STEP_FRAMES = 2;
static const int SIM_STATE_QUIT = 3;
static const int SIM_STATE_OPTIONS = 4;
static const int SIM_STATE_INIT = 5;
static const int SIM_STATE_RESTART = 6;
int simState = SIM_STATE_UNDEF;

// The x and y coordinates defining the GUI
static const std::vector<int> X_VEC_GUI = {0, WINDOW_WIDTH / 3, 2 * WINDOW_WIDTH / 3, 51 * WINDOW_WIDTH / 60, WINDOW_WIDTH};
static const std::vector<int> Y_VEC_GUI = {0, 9 * WINDOW_HEIGHT / 10, 19 * WINDOW_HEIGHT / 20, WINDOW_HEIGHT};


// Values used for all Cell type variables
std::random_device rd{};
std::mt19937 rng{rd()};
static const int NUM_EAM_ELE = 3;
static const int REQ_EAM_SUM = 100; // Required sum of all elements in EAM
static const int ID_LEN = 40;
static const int IDLE_MODE = 0, WALK_MODE = 1, RUN_MODE = 2;
static const int EAM_SUN = 0, EAM_GND = 1, EAM_CELLS = 2;
    //  Indices for each element in EAM.
    //  EAM_SUN means energy from sun (or radiation)
    //  EAM_GND means energy from ground
    //  EAM_CELLS means energy from other cells
static const int CELL_TYPE_PLANT = 0, CELL_TYPE_WORM = 1, CELL_TYPE_PREDATOR = 2, CELL_TYPE_MUTANT = 3;
static const int CELL_TYPE_GENERIC = 4, CELL_TYPE_PLANT_WORM_PREDATOR_OR_MUTANT = 5;
std::discrete_distribution<int> availableCellTypes = {CELL_TYPE_PLANT, CELL_TYPE_WORM, CELL_TYPE_PREDATOR, CELL_TYPE_MUTANT};
SimParamInt forceDampingFactor(1000000, 0, 1000000);
    // Default: 100
    // Applies to the repulsive force that keeps cells apart
    // Smaller values increase the strength of repulsive force
    // Tiny cells will not affect each others' movement using forces due to their tiny size,
    //  but larger cells will typically push each other away gently
    // Larger cells have a larger diameter, so getting to the center of the circle
    //  will yield the maximum force, which grows quadratically with diameter.
    //  Since launch distance is proportional to force, absurdly large launching distances
    //  can be achieved if a cell is inside a particularly large cell
    // Another program feature is that both cells get launched the same distance away, so a large
    //  number of tiny cells can propel larger cells to "teleport" them wherever
std::map<std::string, std::string> ENERGY_COST_TO_CLONE = {
    {"base", "x"}, {"visionDist", "100*x"},
    {"attack", "400"}, {"size", "10*size"},
};
std::map<std::string, std::string> ENERGY_COST_PER_USE = {
    {"base", "10*size"}, {"speed", "(x*x+20)*x"}, {"visionDist", "x*x"},
    {"age", "x*x/2500/size"}, {"maxHealth", "5*x/size/size"},
    {"attack", "200*x/size/size"},
    {"overcrowding", "overcrowdingEnergyCoef*x/size"}
};


// When the global parameters are changed, dependent global parameters also change
int min_int(int num1, int num2);
int max_int(int num1, int num2);
void update_global_params(){
    // If a region near the bottom or the right side is smaller, then it is absorbed
    //  into the neighboring region(s)
    if(ubX.val <= 0 || ubY.val <= 0) return;
    cellRegionNumUbX = max_int(ubX.val / CELL_REGION_SIDE_LEN, 1);
    cellRegionNumUbY = max_int(ubY.val / CELL_REGION_SIDE_LEN, 1);
    drawScaleFactor = min_int(WINDOW_WIDTH / ubX.val, 9 * WINDOW_HEIGHT / 10 / ubY.val);
    ubX_px = ubX.val * drawScaleFactor;
    ubY_px = ubY.val * drawScaleFactor;
    return;
}
// The values of certain global parameters are restricted by others
//  NOTE: This is updated so that the SimIntParam variable automatically does this
void enforce_global_param_constraints(){
    switch(dayNightLbPct.lastIncrement){
        case -1:
        while(dayNightLbPct.val >= dayNightUbPct.val) dayNightLbPct.increment_val(false);
        break;
        case 1:
        while(dayNightLbPct.val >= dayNightUbPct.val) dayNightLbPct.increment_val(true);
        break;
    }
    dayNightLbPct.lastIncrement = 0;
    switch(dayNightUbPct.lastIncrement){
        case -1:
        while(dayNightLbPct.val >= dayNightUbPct.val) dayNightUbPct.increment_val(false);
        break;
        case 1:
        while(dayNightLbPct.val >= dayNightUbPct.val) dayNightUbPct.increment_val(true);
        break;
    }
    dayNightUbPct.lastIncrement = 0;
}













// Parse string expressions
//  DISCLAIMER: You may have to surround your numbers
//  with more brackets or else the formatting requirements
//  might not be easy to satisfy
namespace StrExprInt {
    int do_operation_int(int num1, int num2, char op){
        switch(op){
            case '+':
            return num1 + num2;
            case '-':
            return num1 - num2;
            case '*':
            return num1 * num2;
            case '/':
            return num1 / num2;
            case '\0':
            assert(num1 == 0);
            return num2;
            default:
            assert(false);
        }
        return -1;
    }
    // This function assumes that the variable ends at the occurrence of the last alphabetical character
    //  and ensures that this variable is actually in varDict
    int get_next_int_var(std::string& strExpr, int& iCur, std::map<std::string, int>& varDict){
        int i0 = iCur;
        while(LETTERS.count(strExpr[iCur])) iCur++;
        return varDict[strExpr.substr(i0, iCur-i0)];
    }
    int get_next_int(std::string& strExpr, int& iCur){
        int i0 = iCur;
        while(DIGITS.count(strExpr[iCur])) iCur++;
        return std::stoi(strExpr.substr(i0, iCur-i0));
    }

    // Evaluate strExpr and enforce its format is correct
    //  Aside from brackets, operations go from left to right without exception.
    //  If the result of this algorithm contradicts the conventional order of operations,
    //  throw an error
    //  NOTE: This function only works if all variables in varDict are known
    int solve_strExpr_int(std::string& strExpr, int& iCur, std::map<std::string, int>& varDict){
        // Each '*' and '/' term MUST be surrounded by brackets, i.e. '(' and ')'
        //  if they come after a '+' or '-' (unary '-' exempt)
        //  This way, operations can be evaluated from left to right
        // All variables MUST be made exclusively of letters
        // If any of these rules are violated, throw an error
        char prevOp = '\0'; // '+', '-', '*', '/'
        int ans = 0;
        int num = 0;
        bool lastDidOp = false; // Ensure an operation from {-,+,*,/} is not followed by another operation from {-,+,*,/}
        bool lastDidNum = false; // Ensure a number or a variable is not followed by another number or variable
        while(iCur < strExpr.size()){
            if (LETTERS.count(strExpr[iCur])) {
                assert(!lastDidNum);
                lastDidNum = true; lastDidOp = false;
                num = get_next_int_var(strExpr, iCur, varDict);
                //  iCur is automatically modified here
                ans = do_operation_int(ans, num, prevOp);
                continue;
            }
            if (DIGITS.count(strExpr[iCur])) {
                assert(!lastDidNum);
                lastDidNum = true; lastDidOp = false;
                num = get_next_int(strExpr, iCur);
                //  iCur is automatically modified here
                ans = do_operation_int(ans, num, prevOp);
                continue;
            }
            switch(strExpr[iCur]){
                case '-':
                case '+':
                prevOp = strExpr[iCur];
                assert(!lastDidOp);
                lastDidNum = false; lastDidOp = true;
                break;
                case '*':
                case '/':
                assert(!lastDidOp);
                lastDidNum = false; lastDidOp = true;
                assert(prevOp != '-' && prevOp != '+');
                prevOp = strExpr[iCur];
                break;
                case '(':
                num = solve_strExpr_int(strExpr, ++iCur, varDict);
                ans = do_operation_int(ans, num, prevOp);
                lastDidNum = true; lastDidOp = false;
                break;
                case ')':
                return ans;
                case ' ':
                break;
                default:
                assert(false);
            }
            iCur++;
        }
        return ans;
    }
    int solve(std::string strExpr, std::map<std::string, int> varDict = {}){
        assert(strExpr.size());
        int iCur = 0;
        int ans = solve_strExpr_int(strExpr, iCur, varDict);
        return ans;
    }

    // Test the main function in this namespace with different string expressions
    void test_input(std::string strExpr, std::map<std::string, int> varDict = {}){
        int iCur = 0;
        int ans = solve_strExpr_int(strExpr, iCur, varDict);
        std::cout << strExpr << " = " << ans << std::endl;
    }
    void test_inputs(){
        std::string strExpr;
        std::map<std::string, int> varDict;
        test_input("4", {});
        test_input("-5", {});
        //test_input("   -5 / 2", {});
        //  This one's supposed to throw an error because the format is wrong
        //  This expression is equivalent to "0-5/2", which would contradict with
        //  the conventional order of operations if the first number != 0
        test_input("    (-5) / 2", {});                 // -2
        test_input("-(7*2) + 1", {});                   // -13
        //test_input("-(7*2 + 1 - 32)*(41/10)", {});
        //  This one's supposed to throw an error
        test_input("(-1)*(7*2 + 1 - 32)*(41/10)", {});  // 68
        varDict["x"] = 12;
        test_input("x", varDict);                       // 12
        test_input("(2*x) - 3", varDict);               // 21
        varDict["y"] = 4;
        varDict["size"] = 3;
        test_input("size", varDict);                    // 3
        test_input(" x /x - size + (x*y/ size  )   ", varDict);  // 14

    }


};


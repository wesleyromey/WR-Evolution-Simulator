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

// Global Simulation Parameters
int initNumCells = 400;
// TODO: Ensure the actual window size is constant, but UB_X and UB_Y are free to be changed by the user
static const int UB_X = 60, UB_Y = 40;  // 120, 80
// I split cells into square regions so I only have to compare the positions
// of nearby cells to calculate forces, crowding, interactions, vision, etc.
static const int CELL_REGION_SIDE_LEN = (int)(sqrt(sqrt(UB_X*UB_X + UB_Y*UB_Y)));
static const int CELL_REGION_NUM_X = UB_X / CELL_REGION_SIDE_LEN;
// If a region near the bottom or the right side is smaller, then it is absorbed
// into the neighboring region(s)
static const int CELL_REGION_NUM_Y = UB_Y / CELL_REGION_SIDE_LEN;
// NOTE: The program might not work properly if this is disabled
static const bool WRAP_AROUND_X = true; // Enforce the constraint 0 <= x < UB_X
// NOTE: The program might not work properly if this is disabled
static const bool WRAP_AROUND_Y = true; // Enforce the constraint 0 <= y < UB_Y
static const int TICKS_PER_SEC = 10;    // Each tick, the new positions are calculated 
int cellLimit = 1000;
// Each cell in the simulator must spend this amount of energy per cell it touches.
//  Increasing this value increases the amount of energy spent due to overcrowding. 
int overcrowdingEnergyCoef = 100;
// Energy accumulation for all ground spaces
int maxGndEnergy = 2000;
//int initGndEnergy = maxGndEnergy / 2;
int simGndEnergy[UB_X][UB_Y];
static const int FRAMES_BETWEEN_GND_ENERGY_ACCUMULATION = 10;
int gndEnergyPerIncrease = 10;

// Day-Night Cycle
int energyFromSunPerSec = 0; // This value is automatically updated each frame
int dayNightCycleTime = 0; // Wraps between 0 and dayLenSec - 1
// Max energy which can be accumulated from the sun 
int maxSunEnergyPerSec = 50;
// Number of seconds per day (Not sure if this is actually day length in frames
//  or day length in seconds)
int dayLenSec = 200;
static const int DAY_NIGHT_ALWAYS_DAY_MODE = 0;
static const int DAY_NIGHT_BINARY_MODE = 1;
static const int DAY_NIGHT_DEFAULT_MODE = 2;
static const int DAY_NIGHT_MODE = DAY_NIGHT_DEFAULT_MODE;
// Smaller values (closer to 0) mean the sun remains lower in the sky.
// Larger values (to +inf) mean the sun is near its max height for longer
// A value of 1.75 approximates a sine wave
float dayNightExponent = 2.0; // 0 <= EXPONENT < infinity
// The day lasts between dayNightLb and dayNightUb
// The night lasts from dayNightUb to 1.0 and 0.0 to dayNightLb
float dayNightLb = 0.0, dayNightUb = 0.5;

// Initialize SDL
static const char* WINDOW_TITLE = "Evolution Simulator";
// Calculate DRAW_SCALE_FACTOR (rounded down to nearest int)
static const int TARGET_SCREEN_WIDTH = 770; // Try 1540 for full screen and 770 for half screen
static const int TARGET_SCREEN_HEIGHT = 800; // Try 800, or 400 for a quarter screen
int tmpDrawScaleX = TARGET_SCREEN_WIDTH / UB_X;
int tmpDrawScaleY = TARGET_SCREEN_HEIGHT / UB_Y;
int tmpDrawScale = tmpDrawScaleX < tmpDrawScaleY ? tmpDrawScaleX : tmpDrawScaleY;
static const int DRAW_SCALE_FACTOR = tmpDrawScale;
static const int WINDOW_WIDTH  = DRAW_SCALE_FACTOR*UB_X;
static const int WINDOW_HEIGHT = 10 * DRAW_SCALE_FACTOR*UB_Y / 9;
static const int UB_X_PX = UB_X * DRAW_SCALE_FACTOR;
static const int UB_Y_PX = UB_Y * DRAW_SCALE_FACTOR;
static const unsigned char RGB_MIN = 0, RGB_MAX = 255;

//bool mouseButtonDownPrevFrame = false;
int mousePosX = 0, mousePosY = 0;
static const Uint32 FRAME_DELAY = 20; // ms; frame rate is (1000/FRAME_DELAY) fps
Uint32 frameStart = 0; // The time in ms since the start of the simulation
Uint32 frameTime = 0; // The amount of time the frame lasted for

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
int forceDampingFactor = 100;
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
    {"base", "x"}, {"visionDist", "100*x"}, {"stickiness", "x*x"},
    {"attack", "400"}, {"size", "10*size"},
};
std::map<std::string, std::string> ENERGY_COST_PER_USE = {
    {"base", "10*size"}, {"speed", "(x*x+20)*x"}, {"visionDist", "x*x"},
    {"stickiness", "2*x"}, {"mutationRate", "x/100"},
    {"age", "x*x/2500/size"}, {"maxHealth", "5*x/size/size"},
    {"attack", "20*x/size/size"},
    {"overcrowding", "overcrowdingEnergyCoef*x/size"}
};

// When the global parameters are changed, dependent global parameters also change
int saturate_int(int num, int lb, int ub);
int pow_int(int root, int exponent);
void update_global_params(){
    // UB_X and UB_Y
    //CELL_REGION_SIDE_LEN = (int)(sqrt(sqrt(UB_X*UB_X + UB_Y*UB_Y)));
    //CELL_REGION_NUM_X = UB_X / CELL_REGION_SIDE_LEN;
    //CELL_REGION_NUM_Y = UB_Y / CELL_REGION_SIDE_LEN;

    // Draw scale (related to UB_X and UB_Y)
    //tmpDrawScaleX = TARGET_SCREEN_WIDTH / UB_X;
    //tmpDrawScaleY = TARGET_SCREEN_HEIGHT / UB_Y;
    //tmpDrawScale = tmpDrawScaleX < tmpDrawScaleY ? tmpDrawScaleX : tmpDrawScaleY;
    //static const int DRAW_SCALE_FACTOR = tmpDrawScale;
    //static const int WINDOW_WIDTH  = DRAW_SCALE_FACTOR*UB_X;
    //static const int WINDOW_HEIGHT = 10 * DRAW_SCALE_FACTOR*UB_Y / 9;
    //static const int UB_X_PX = UB_X * DRAW_SCALE_FACTOR;
    //static const int UB_Y_PX = UB_Y * DRAW_SCALE_FACTOR;
    return;
}
// Each global parameters has different lower and upper bounds
void enforce_global_param_bounds(){
    int defaultMax = pow_int(10,6);
    initNumCells = saturate_int(initNumCells, 0, 2000);
    //UB_X = saturate_int(UB_X, 1, defaultMax);
    //UB_Y = saturate_int(UB_X, 1, defaultMax);
    cellLimit = saturate_int(cellLimit, 0, 2000);
    dayLenSec = saturate_int(dayLenSec, 1, defaultMax);
    maxSunEnergyPerSec = saturate_int(maxSunEnergyPerSec, 0, 1000);
    maxGndEnergy = saturate_int(maxGndEnergy, 1, defaultMax);
    gndEnergyPerIncrease = saturate_int(gndEnergyPerIncrease, 0, defaultMax);
    forceDampingFactor = saturate_int(forceDampingFactor, 1, defaultMax);
    overcrowdingEnergyCoef = saturate_int(overcrowdingEnergyCoef, 0, defaultMax);

    update_global_params();
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


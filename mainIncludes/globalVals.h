// Contains global variables and constants

using namespace std;


// Universal Constants
static const double PI = 3.141592653589793238462643;
static const std::set<char> DIGITS = {'0','1','2','3','4','5','6','7','8','9'};
static const std::set<char> LETTERS = {
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'
};
static const std::set<char> OPERATORS = {'+','-','*','/'};

// Global Simulation Parameters
static const int UB_X = 60, UB_Y = 40;
static const bool WRAP_AROUND_X = true; // Enforce the constraint 0 <= x < UB_X
//  NOTE: The program might not work properly if this is disabled
static const bool WRAP_AROUND_Y = true; // Enforce the constraint 0 <= y < UB_Y
//  NOTE: The program might not work properly if this is disabled
static const int TICKS_PER_SEC = 10;    // Each tick, the new positions are calculated 
static const int MAX_SUN_ENERGY_PER_SEC = 40;  // This is the maximum amount of energy which can be accumulated from the sun 
static const int DAY_LEN_SEC = 100;   // Number of seconds per "day"
static const int CELL_LIMIT = 400;
static const float OVERCROWDING_ENERGY_COEF = 10.;
//  Each cell in the simulator must spend this amount of energy (rounded down) per cell it touches. 
int energyFromSunPerSec = 0; // This value is automatically updated each frame
int dayNightCycleTime = 0; // Wraps between 0 and DAY_LEN_SEC - 1

// Energy accumulation for all ground spaces
static const int MAX_GND_ENERGY = 2000;
static const int INIT_GND_ENERGY = MAX_GND_ENERGY / 2;
int simGndEnergy[UB_X][UB_Y];
static const int FRAMES_BETWEEN_GND_ENERGY_ACCUMULATION = 10;
static const int GND_ENERGY_PER_INCREASE = 10;

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
static const int WINDOW_HEIGHT = DRAW_SCALE_FACTOR*(UB_Y+4);
static const unsigned char RGB_MIN = 0, RGB_MAX = 255;

bool mouseButtonDownPrevFrame = false;
bool simIsRunning = true; // If false, exit the program

// Simulation States: These control the GUI, simulation mode, etc.
//  TODO: Actually implement these into the game
static const int SIM_STATE_MAIN_MENU = 0;
static const int SIM_STATE_SKIP_FRAMES = 1;
static const int SIM_STATE_STEP_FRAMES = 2;
static const int SIM_STATE_QUIT = 3;
static const unsigned int AUTO_ADVANCE_DEFAULT = 5000;
int simState = SIM_STATE_MAIN_MENU;




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
static const int FORCE_DAMPING_FACTOR = 100;
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
    {"base", "200"}, {"visionDist", "100*x"}, {"stickiness", "x*x"},
    {"attack", "400"}, {"size", "100*size"},
};
std::map<std::string, std::string> ENERGY_COST_PER_USE = {
    {"base", "10*size"}, {"speed", "(x*x+20)*x"}, {"visionDist", "x*x"},
    {"stickiness", "2*x"}, {"mutationRate", "x/100"},
    {"age", "x*x/2500/size"}, {"maxHealth", "5*x/size"}, {"attack", "20*x/size"},
    {"overcrowding", "OVERCROWDING_ENERGY_COEF*x"}
};


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


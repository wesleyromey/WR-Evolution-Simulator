// This file contains most of the SDL functions
//  required to display the User Interface (UI)
//  including graphics, buttons, menus, etc.

#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif

// NOTE: Some of the constants occur at the
//  end of this file
static const char* WINDOW_TITLE = "Evolution Simulator";
// Calculate DRAW_SCALE_FACTOR (rounded down to nearest int)
static const int TARGET_SCREEN_WIDTH = 770; // Try 1540 for full screen and 770 for half screen
static const int TARGET_SCREEN_HEIGHT = 800; // Try 800, or 400 for a quarter screen
int tmpDrawScaleX = TARGET_SCREEN_WIDTH / UB_X;
int tmpDrawScaleY = TARGET_SCREEN_HEIGHT / UB_Y;
int tmpDrawScale = tmpDrawScaleX < tmpDrawScaleY ? tmpDrawScaleX : tmpDrawScaleY;
static const int DRAW_SCALE_FACTOR = tmpDrawScale;
static const int WINDOW_WIDTH  = DRAW_SCALE_FACTOR*UB_X;
static const int WINDOW_HEIGHT = DRAW_SCALE_FACTOR*UB_Y;

bool mouseButtonDownPrevFrame = false;
bool simIsRunning = true; // If false, exit the program


// SDL Frame Rendering, Textures, etc.
static const int RGB_MIN = 0, RGB_MAX = 255;
SDL_Window* init_SDL_window();
SDL_Window* P_WINDOW = init_SDL_window();
SDL_Renderer* init_SDL_renderer();
SDL_Renderer* P_RENDERER = init_SDL_renderer();
SDL_Texture* load_texture(const char* filePath);
void draw_texture(SDL_Texture* pTexture, int xPos, int yPos, int height, int width);
SDL_Texture* findSDLTex(int num,
    const std::vector<std::pair<int, SDL_Texture*>>& sdlMap
);
SDL_Texture* P_CELL_TEX = load_texture("res/cell_skeleton.png");
static const std::vector<std::pair<int, SDL_Texture*>> P_CELL_ENERGY_TEX = {
    {0,     load_texture("res/energy/0.png")},
    {100,   load_texture("res/energy/100.png")},
    {200,   load_texture("res/energy/200.png")},
    {300,   load_texture("res/energy/300.png")},
    {400,   load_texture("res/energy/400.png")},
    {500,   load_texture("res/energy/500.png")},
    {600,   load_texture("res/energy/600.png")},
    {700,   load_texture("res/energy/700.png")},
    {800,   load_texture("res/energy/800.png")},
    {900,   load_texture("res/energy/900.png")},
    {1000,  load_texture("res/energy/1000.png")},
    {2000,  load_texture("res/energy/2000.png")},
    {3000,  load_texture("res/energy/3000.png")},
    {4000,  load_texture("res/energy/4000.png")},
    {5000,  load_texture("res/energy/5000.png")},
    {6000,  load_texture("res/energy/6000.png")},
    {7000,  load_texture("res/energy/7000.png")},
    {8000,  load_texture("res/energy/8000.png")},
    {9000,  load_texture("res/energy/9000.png")},
    {10000, load_texture("res/energy/10000.png")},
};
static const std::vector<std::pair<int, SDL_Texture*>> P_CELL_HEALTH_TEX = {
    {0,   load_texture("res/health/0pct.png")},
    {10,  load_texture("res/health/10pct.png")},
    {20,  load_texture("res/health/20pct.png")},
    {30,  load_texture("res/health/30pct.png")},
    {40,  load_texture("res/health/40pct.png")},
    {50,  load_texture("res/health/50pct.png")},
    {60,  load_texture("res/health/60pct.png")},
    {70,  load_texture("res/health/70pct.png")},
    {80,  load_texture("res/health/80pct.png")},
    {90,  load_texture("res/health/90pct.png")},
    {100, load_texture("res/health/100pct.png")}
};
SDL_Texture* P_DO_ATTACK_TEX = load_texture("res/stat/doAttack.png");
SDL_Texture* P_DO_CLONING_TEX  = load_texture("res/stat/doClone.png");
std::map<std::string, SDL_Texture*> P_EAM_TEX = {
    {"s4", load_texture("res/EAM/s4.png")},
    {"s3", load_texture("res/EAM/s3.png")},
    {"s2", load_texture("res/EAM/s2.png")},
    {"s1", load_texture("res/EAM/s1.png")},
    {"g4", load_texture("res/EAM/g4.png")},
    {"c4", load_texture("res/EAM/c4.png")},
    {"c3", load_texture("res/EAM/c3.png")},
    {"c2", load_texture("res/EAM/c2.png")},
    {"c1", load_texture("res/EAM/c1.png")},
    {"s2g2", load_texture("res/EAM/s2g2.png")},
    {"s2c2", load_texture("res/EAM/s2c2.png")},
    {"g2c2", load_texture("res/EAM/g2c2.png")},
    {"balanced", load_texture("res/EAM/balanced.png")},
};
SDL_Texture* P_DEAD_CELL_TEX = load_texture("res/deadCell.png");
SDL_Texture* P_BKGND_TEX = load_texture("res/bkgnd_default.png");
// The background is drawn using 1 color.
//  This function was made to ensure color transitions are smooth
// efs = energyFromSunlight
// TODO
void draw_bkgnd(int energyFromSunPerSec){
    int efs = energyFromSunPerSec;
    // (Approximately)
    // efs  | Red   | Green | Blue
    // 0    | 0     | 0     | 0
    // 1    | 40    | 0     | 0
    // 10   | 100   | 0     | 0
    // 50   | 160   | 160   | 0
    // 250  | 200   | 200   | 200
    // 1000 | 0     | 255   | 255  // Remains here forever
    
    // Every color logarithmically increases from a starting point.
    //  and may in some cases linearly go back down to a different end point.
    //  If the starting point is the value at the start of the function:
    //      color = C0 + C1*ln(efs), or alternatively:
    //  If the starting point is at an initial efs:
    //      color = C1 * (ln(efs) - ln(startEfs))
    static const int LN_RED_C0 = 40, LN_RED_C1 = 30;
    #define fRedLn(efs) saturate_int(LN_RED_C0 + LN_RED_C1*ln(efs), RGB_MIN, RGB_MAX)
    static const int MID_EFS_RED = 250, END_EFS_RED = 1000;
    static const int RED_START = 40, RED_MID = fRedLn(MID_EFS_RED), RED_END = 0;
    static const int RED_SLOPE = (RED_END - RED_MID) / (END_EFS_RED - MID_EFS_RED);
    #define fRedLinear(efs) saturate_int(RED_MID + RED_SLOPE * (efs - MID_EFS_RED), RGB_MIN, RGB_MAX)

    static const int START_EFS_GRN = 10;
    static const int LN_GRN_C1 = 55; // Increase to make the increase for Green last shorter
    #define fGrnLn(efs) saturate_int(LN_GRN_C1*(ln(efs) - ln(START_EFS_GRN)), RGB_MIN, RGB_MAX)

    static const int START_EFS_BLUE = 50;
    static const int LN_BLUE_C1 = 85;
    #define fBlueLn(efs) saturate_int(LN_BLUE_C1*(ln(efs) - ln(START_EFS_BLUE)), RGB_MIN, RGB_MAX)

    // First, set the RGB values for the background based on sunlight
    int red = 0, grn = 0, blue = 0; // Set for efs == 0
    if(efs != 0){
        red = (efs <= 250) ? fRedLn(efs) : fRedLinear(efs);
        grn = fGrnLn(efs);
        blue = fBlueLn(efs);
    }

    // Second, create the texture using the calculated RGB values
    //cout << "efs: " << efs << "; RGB: " << red << ", " << grn << ", " << blue << endl;
    SDL_SetRenderDrawColor(P_RENDERER, red, grn, blue, SDL_ALPHA_OPAQUE);
    SDL_RenderPresent(P_RENDERER);
}
std::vector<std::pair<int, SDL_Texture*>> P_GND_TEX = {
    // The amount of energy per second from sunlight as a percent of
    //  the max ground energy should be described by these file names
    {0,   load_texture("res/gndEnergy/0pct.png")},
    {10,  load_texture("res/gndEnergy/10pct.png")},
    {20,  load_texture("res/gndEnergy/20pct.png")},
    {30,  load_texture("res/gndEnergy/30pct.png")},
    {40,  load_texture("res/gndEnergy/40pct.png")},
    {50,  load_texture("res/gndEnergy/50pct.png")},
    {60,  load_texture("res/gndEnergy/60pct.png")},
    {70,  load_texture("res/gndEnergy/70pct.png")},
    {80,  load_texture("res/gndEnergy/80pct.png")},
    {90,  load_texture("res/gndEnergy/90pct.png")},
    {100, load_texture("res/gndEnergy/100pct.png")},
};
void draw_gnd(){
    int numRows = UB_X, numCols = UB_Y;
    for(int row = 0; row < numRows; row++){
        for(int col = 0; col < numCols; col++){
            int drawX = DRAW_SCALE_FACTOR*row;
            int drawY = DRAW_SCALE_FACTOR*col;
            int drawSize = DRAW_SCALE_FACTOR;
            assert(drawX >= 0 && drawY >= 0 && simGndEnergy[row][col] >= 0);
            draw_texture(
                findSDLTex(100 * simGndEnergy[row][col] / MAX_GND_ENERGY, P_GND_TEX),
                drawX, drawY, drawSize, drawSize
            );
        }
    }
}



SDL_Texture* load_texture(const char* filePath){
    SDL_Texture* ans = IMG_LoadTexture(P_RENDERER, filePath);
    if(ans == NULL) cout << filePath << endl;
    assert(ans != NULL);
    return ans;
}

// Draw a texture at a particular x and y position
void draw_texture(SDL_Texture* pTexture, int xPos, int yPos, int height, int width){
    // xPos = 0, yPos = 0 refers to the top left corner of the window
    // Not sure what pSrc refers to.
    //  pSrc may represent the original object, but I'm not sure
    //  If pSrc == NULL, then a new texture is rendered
    //  Otherwise, the texture may be replaced (???)
    // pDst represents the new object
    //  If dst == NULL, then the texture fills the entire window
    SDL_Rect* pDst = new SDL_Rect;
    pDst->x = xPos; pDst->y = yPos;
    pDst->h = height; pDst->w = width;
    //SDL_RenderCopy(pRenderer, pTexture, pSrc, pDst);
    SDL_RenderCopy(P_RENDERER, pTexture, NULL, pDst);
    //  This renders the opject according to pDst
    //  I could replace NULL with pSrc, but I don't know what pSrc refers to
}

// TODO: Move this function to ui.h
// sdlMap contains various thresholds which, when exceeded,
//  Returns the corresponding SDL_Texture*
SDL_Texture* findSDLTex(int num,
        const std::vector<std::pair<int, SDL_Texture*>>& sdlMap){
    // Use the binary search algorithm
    int iLb = 0, iUb = sdlMap.size()-1;
    while(iLb < iUb){
        int iMid = (iLb+iUb+1)/2;
        if(num < sdlMap[iMid].first) iUb = --iMid;
        else iLb = iMid;
    }
    assert(iLb == iUb);
    return sdlMap[iLb].second;
}

SDL_Window* init_SDL_window(){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* pWindow = SDL_CreateWindow(
        WINDOW_TITLE, 0, 20, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI
    );
    assert(pWindow != NULL);
    return pWindow;
}

SDL_Renderer* init_SDL_renderer(){
    SDL_Renderer* pRenderer = SDL_CreateRenderer(
        P_WINDOW, -1, SDL_RENDERER_ACCELERATED
    );
    assert(pRenderer != NULL);
    return pRenderer;
}

void wait_for_user_to_exit_SDL(){
    SDL_Event windowEvent;
    if(!simIsRunning) return;
    while(true){
        if(SDL_PollEvent(&windowEvent)){
            if(windowEvent.type == SDL_QUIT){
                simIsRunning = false;
                return;
            }
        }
    }
}

void exit_SDL(){
    SDL_DestroyWindow(P_WINDOW);
    SDL_Quit();
    std::cout << "SDL is quitting!\n";
}


// Handle events between frames using SDL
//  e.g. Allow the user to decide when to advance to the next frame
//bool KEYS[322] = {0}; // 322 is the number of SDLK_DOWN events
//SDL_EnableKeyRepeat(0,0); // ???
unsigned int autoAdvanceSim = 0;
static const unsigned int AUTO_ADVANCE_DEFAULT = 10000;
void SDL_event_handler(){
    bool pauseSim = simIsRunning && !autoAdvanceSim; // default: true
    if(autoAdvanceSim) autoAdvanceSim--;
    SDL_Event windowEvent;
    while(pauseSim){
        while(SDL_WaitEvent(&windowEvent)){
            switch(windowEvent.type){
                case SDL_KEYDOWN:
                switch(windowEvent.key.keysym.sym){
                    case SDLK_ESCAPE:
                    simIsRunning = false;
                    case SDLK_n:
                    case SDLK_SPACE:
                    pauseSim = false;
                    autoAdvanceSim = 0;
                    break;
                    case SDLK_a:
                    autoAdvanceSim = AUTO_ADVANCE_DEFAULT;
                    cout << "a is pressed! Simulation is speeding up for " << autoAdvanceSim << " frames" << endl;
                    break;
                }
                break;
                case SDL_QUIT:
                simIsRunning = false;
                pauseSim = false;
                break;
            }
            if(!pauseSim) break;
            if(autoAdvanceSim) {
                pauseSim = false;
                break;
            }
        }
    }
    if(!simIsRunning) std::cout << "Simulation will exit..." << std::endl;
}



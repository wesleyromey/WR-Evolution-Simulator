// This file contains most of the SDL functions
//  required to display the User Interface (UI)
//  including graphics, buttons, menus, etc.

#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif

void wait_for_user_to_exit_SDL(){
    if(simState == SIM_STATE_QUIT) return;
    SDL_Event windowEvent;
    while(true){
        if(SDL_PollEvent(&windowEvent)){
            if(windowEvent.type == SDL_QUIT){
                simState = SIM_STATE_QUIT;
                return;
            }
        }
    }
}

void run_step_frames_press_n(bool& pauseSim, unsigned int& autoAdvanceSim){
    pauseSim = false;
    autoAdvanceSim = 0;
}

void run_skip_frames(int& simState, unsigned int& autoAdvanceSim, int numFramesToSkip = AUTO_ADVANCE_DEFAULT){
    simState = SIM_STATE_SKIP_FRAMES;
    autoAdvanceSim = numFramesToSkip;
    std::cout << "a is pressed! Simulation is speeding up for " << autoAdvanceSim << " frames" << endl;
}

void run_sim_state_step_frames(SDL_Event& windowEvent, bool& pauseSim, unsigned int& autoAdvanceSim, int& simState){
    SDL_WaitEvent(&windowEvent);
    Uint32 mouseClickType = 0;
    switch(windowEvent.type){
        case SDL_KEYDOWN:
        switch(windowEvent.key.keysym.sym){
            case SDLK_ESCAPE:
            simState = SIM_STATE_QUIT;
            case SDLK_n:
            case SDLK_SPACE:
            run_step_frames_press_n(pauseSim, autoAdvanceSim);
            break;
            case SDLK_a:
            run_skip_frames(simState, autoAdvanceSim, AUTO_ADVANCE_DEFAULT);
            break;
            case SDLK_s:
            run_skip_frames(simState, autoAdvanceSim, 10*AUTO_ADVANCE_DEFAULT);
        }
        break;
        case SDL_MOUSEBUTTONDOWN:
        mouseClickType = SDL_GetMouseState(&mousePosX, &mousePosY);
        //cout << "mouseClickType: " << mouseClickType << ", mousePosX: " << mousePosX << ", mousePosY: " << mousePosY << endl;
        if(mouseClickType == 1){
            // Left Click
            if(mousePosY < ubY_px || mousePosX >= X_VEC_GUI[3]) break;
            else{
                if(mousePosX < X_VEC_GUI[1]){
                    run_step_frames_press_n(pauseSim, autoAdvanceSim);
                } else if(mousePosX < X_VEC_GUI[2]){
                    run_skip_frames(simState, autoAdvanceSim, AUTO_ADVANCE_DEFAULT);
                } else if(mousePosX < X_VEC_GUI[3]){
                    simState = SIM_STATE_OPTIONS;
                }
            }
        }
        break;
        case SDL_QUIT:
        simState = SIM_STATE_QUIT;
        pauseSim = false;
        break;
    }
}


// Return true if the box was clicked (and the sim state changed)
bool check_if_mouse_clicked_on_box(Uint32 mousePosX, Uint32 mousePosY, int xLb, int yLb, int xUb, int yUb){
    if(mousePosX < xLb || mousePosX >= xUb) return false;
    if(mousePosY < yLb || mousePosY >= yUb) return false;
    return true;
}
bool change_simState_on_box_click(Uint32 mousePosX, Uint32 mousePosY, int xLb, int yLb, int xUb, int yUb, int newSimStateIfClicked){
    if(!check_if_mouse_clicked_on_box(mousePosX, mousePosY, xLb, yLb, xUb, yUb)) return false;
    simState = newSimStateIfClicked;
    return true;
}
// Return true if the box was clicked
/*
//bool increment_var_on_box_click(Uint32 mousePosX, Uint32 mousePosY, int xLb, int yLb, int xUb, int yUb, int* pVarToChange, bool doIncrease){
void increment_var(int* pVarToChange, bool doIncrease){
    // lb = lower bound, ub = upper bound, x and y are coordinates
    //cout << "xLb: " << xLb << ", xUb: " << xUb << ", yLb: " << yLb << ", yUb: " << yUb << endl;
    //if(!check_if_mouse_clicked_on_box(mousePosX, mousePosY, xLb, yLb, xUb, yUb)) return false;
    if(doIncrease && *pVarToChange < pow_int(10,9)){
        int incrementExponent = (int)(log10(max_int(1, *pVarToChange / 2)));
        int varIncrement = pow_int(10, incrementExponent);
        *pVarToChange += varIncrement;
    }
    if(!doIncrease && *pVarToChange > 0){
        int incrementExponent = (int)(log10(max_int(1, *pVarToChange / 2.01)));
        int varIncrement = pow_int(10, incrementExponent);
        *pVarToChange -= varIncrement;
    }
    //enforce_global_param_bounds();
    //return true;
}
*/
// Return true if the box was clicked
//  This function only works for vars whose possible values include only the values between 0 and endVal
bool increment_var_on_box_click(Uint32 mousePosX, Uint32 mousePosY, int xLb, int yLb, int xUb, int yUb,
        SimParamInt* pVarToIncrement, bool doIncrease){
    if(!check_if_mouse_clicked_on_box(mousePosX, mousePosY, xLb, yLb, xUb, yUb)) return false;
    pVarToIncrement->increment_val(doIncrease);
    enforce_global_param_constraints();
    return true;
}
void draw_options_menu(int x0, int dx, int dy, std::vector<std::pair<int, string>>& optionText);
void draw_main_menu(std::vector<int>& xVec, std::vector<int>& yVec, std::vector<std::pair<string, SimParamInt*>>& simParamsText, int xLbStart, int yLbStart, int dyStart);
void SDL_draw_frame();
void run_sim_state_options_menu(SDL_Event& windowEvent, bool& pauseSim, int& simState){
    std::vector<std::pair<int, string>> optionText;
    #define wfx(fraction) (WINDOW_WIDTH*fraction)
    #define wfy(fraction) (WINDOW_HEIGHT*fraction)
    optionText.push_back({wfy(0.4), "Continue"});
    optionText.push_back({wfy(0.6), "Restart" });
    optionText.push_back({wfy(0.8), "Quit"    });
    int x0 = wfx(0.25), y0 = 0;
    int x1 = WINDOW_WIDTH - x0;
    int dx = x1 - x0, dy = 0.1*WINDOW_HEIGHT;
    //std::cout << "x0: " << x0 << ", dx: " << dx << ", dy: " << dy << endl;
    draw_options_menu(x0, dx, dy, optionText);
    #undef wfx
    #undef wfy
    #define deal_with_box_click(y0, newState) change_simState_on_box_click(mousePosX, mousePosY, x0, y0, x1, y0 + dy, newState)
    while(simState == SIM_STATE_OPTIONS){
        SDL_WaitEvent(&windowEvent);
        Uint32 mouseClickType = 0;
        switch(windowEvent.type){
            case SDL_MOUSEBUTTONDOWN:
            mouseClickType = SDL_GetMouseState(&mousePosX, &mousePosY);
            if(mouseClickType != 1) break; // MUST be a left click, else break
            if(deal_with_box_click(optionText[0].first, SIM_STATE_STEP_FRAMES)) break;
            if(deal_with_box_click(optionText[1].first, SIM_STATE_RESTART)) break;
            if(deal_with_box_click(optionText[2].first, SIM_STATE_QUIT)) break;
        }
    }
    #undef deal_with_box_click
}
std::vector<std::pair<string, SimParamInt*>> decide_sim_settings_options_text(){
    std::vector<std::pair<string, SimParamInt*>> simParamsText;
    simParamsText.push_back({"Initial # of Cells", &initNumCells});
    simParamsText.push_back({"Max # of Cells", &cellLimit});
    simParamsText.push_back({"Day-Night Cycle Mode", &dayNightMode});
    switch(dayNightMode.val){
        case DAY_NIGHT_ALWAYS_DAY_MODE:
        simParamsText.push_back({"Sun Energy Regeneration", &maxSunEnergyPerSec});
        break;
        case DAY_NIGHT_BINARY_MODE:
        simParamsText.push_back({"Day Length", &dayLenSec});
        simParamsText.push_back({"Max Sun Energy Regeneration", &maxSunEnergyPerSec});
        //simParamsText.push_back({"Day start (pct)", &dayNightLbPct});
        simParamsText.push_back({"Pct of day with sunlight", &dayNightUbPct}); // Original title: "Day end (pct)"
        break;
        case DAY_NIGHT_DEFAULT_MODE:
        simParamsText.push_back({"Day Length", &dayLenSec});
        simParamsText.push_back({"Max Sun Energy Regeneration", &maxSunEnergyPerSec});
        //simParamsText.push_back({"Sun Path coef (Default 200)", &dayNightExponentPct});
        //simParamsText.push_back({"Day start (pct)", &dayNightLbPct});
        simParamsText.push_back({"Pct of day with sunlight", &dayNightUbPct}); // Original title: "Day end (pct)"
        break;
    }
    simParamsText.push_back({"Max Ground Energy per cell", &maxGndEnergy});
    simParamsText.push_back({"Ground Energy Regeneration", &gndEnergyPerIncrease});
    //simParamsText.push_back({"Force Damping Factor", &forceDampingFactor});
    //simParamsText.push_back({"Overcrowding energy coefficient", &overcrowdingEnergyCoef});
    simParamsText.push_back({"Map height", &ubY});
    simParamsText.push_back({"Map width", &ubX});
    simParamsText.push_back({"Mutation Chance", &defaultMutationChance});
    simParamsText.push_back({"Mutation Amount", &defaultMutationAmt});
    return simParamsText;
}
std::vector<int> decide_sim_settings_options_x_coords(std::vector<std::pair<string, SimParamInt*>>& simParamsText){
    #define wfx(fraction) (int)(fraction*WINDOW_WIDTH)
    std::vector<int> xVec = {wfx(0.05), wfx(0.6), wfx(0.8), wfx(0.84), wfx(0.88)};
    #undef wfx
    return xVec;
}
std::vector<int> decide_sim_settings_options_y_coords(std::vector<std::pair<string, SimParamInt*>>& simParamsText){
    #define wfy(fraction) (int)(fraction*WINDOW_HEIGHT)
    //max_int()
    int y0 = wfy(0), y1 = wfy(0.20), dy = 0.08*WINDOW_HEIGHT; // 0.075*WINDOW_HEIGHT can fit 10 options
    if(simParamsText.size() > 10) dy *= 10.0 / simParamsText.size();
    std::vector<int> yVec = {y0};
    for(int i = 0; i < simParamsText.size(); i++) yVec.push_back(y1 + i*dy);
    //cout << "yVec: { "; for(auto num : yVec) cout << num << " "; cout << "}\n";
    #undef wfy
    return yVec;
}
// Allow the user to customize various simulation parameters before each simulation
void run_sim_state_main_menu(SDL_Event& windowEvent, bool& pauseSim, int& simState){
    //frameStart = SDL_GetTicks();
    #define deal_with_box_click(xLb, xUb, yLb, yUb, pVarToIncrement, doIncrease) increment_var_on_box_click(mousePosX, mousePosY, xLb, yLb, xUb, yUb, pVarToIncrement, doIncrease)
    while(simState == SIM_STATE_MAIN_MENU){
        std::vector<std::pair<string, SimParamInt*>> simParamsText = decide_sim_settings_options_text();
        // Specify the x and y coordinates to draw the text and allow it to respond when clicking on it
        std::vector<int> xVec = decide_sim_settings_options_x_coords(simParamsText);
        std::vector<int> yVec = decide_sim_settings_options_y_coords(simParamsText);
        int xLbStart = xVec[2], yLbStart = yVec[0], dyStart = 0.75*(yVec[1]-yVec[0]);
        int dy = yVec[2]-yVec[1];

        draw_main_menu(xVec, yVec, simParamsText, xLbStart, yLbStart, dyStart);
        SDL_WaitEvent(&windowEvent);
        Uint32 mouseClickType = 0;
        if(windowEvent.type == SDL_MOUSEBUTTONDOWN){
            mouseClickType = SDL_GetMouseState(&mousePosX, &mousePosY);
            if(mouseClickType != 1) continue; // MUST be a left click, else continue
            //cout << "mousePosX: " << mousePosX << ", mousePosY: " << mousePosY << endl;
            int yIndex = 1;
            for(int i = 0; i < simParamsText.size(); i++){
                int yLb = yVec[yIndex++];
                string paramText = simParamsText[i].first;
                SimParamInt* pParameter = simParamsText[i].second;
                deal_with_box_click(xVec[2], xVec[3], yLb, yLb + dy, pParameter, false);
                deal_with_box_click(xVec[3], xVec[4], yLb, yLb + dy, pParameter, true);
                //cout << paramText << ": " << *pParameter << " ";
            }
            //cout << "\n\n";
            // Check if the start button was pressed
            change_simState_on_box_click(mousePosX, mousePosY, xLbStart, yLbStart, WINDOW_WIDTH, yLbStart + dyStart, SIM_STATE_INIT);
        } else if(windowEvent.type == SDL_QUIT){
            simState = SIM_STATE_QUIT;
        }
    }
    #undef deal_with_box_click
}


// Handle events between frames using SDL
//  e.g. Allow the user to decide when to advance to the next frame
//bool KEYS[322] = {0}; // 322 is the number of SDLK_DOWN events
//SDL_EnableKeyRepeat(0,0); // ???
unsigned int autoAdvanceSim = 0;
void SDL_event_handler(){
    bool pauseSim = true;
    SDL_Event windowEvent;
    while(pauseSim){
        switch(simState){
            case SIM_STATE_MAIN_MENU:
            run_sim_state_main_menu(windowEvent, pauseSim, simState);
            break;
            case SIM_STATE_STEP_FRAMES:
            run_sim_state_step_frames(windowEvent, pauseSim, autoAdvanceSim, simState);
            break;
            case SIM_STATE_SKIP_FRAMES:
            if(autoAdvanceSim){
                autoAdvanceSim--;
                pauseSim = false;
            } else simState = SIM_STATE_STEP_FRAMES;
            break;
            case SIM_STATE_OPTIONS:
            run_sim_state_options_menu(windowEvent, pauseSim, simState);
            SDL_draw_frame();
            break;
            case SIM_STATE_QUIT:
            case SIM_STATE_INIT:
            case SIM_STATE_RESTART:
            pauseSim = false;
            break;
            default:
            // If a state is NOT accounted for, then throw an error
            cout << "Caution! This simulation state is NOT accounted for: " << simState << endl;
            assert(false);
            break;
        }
    }
}



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

void run_step_frames_press_a(int& simState, unsigned int& autoAdvanceSim){
    simState = SIM_STATE_SKIP_FRAMES;
    autoAdvanceSim = AUTO_ADVANCE_DEFAULT;
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
            run_step_frames_press_a(simState, autoAdvanceSim);
            break;
        }
        break;
        case SDL_MOUSEBUTTONDOWN:
        mouseClickType = SDL_GetMouseState(&mousePosX, &mousePosY);
        //cout << "mouseClickType: " << mouseClickType << ", mousePosX: " << mousePosX << ", mousePosY: " << mousePosY << endl;
        if(mouseClickType == 1){
            // Left Click
            if(mousePosY < UB_Y_PX) break;
            else{
                if(mousePosX < WINDOW_WIDTH / 3){
                    run_step_frames_press_n(pauseSim, autoAdvanceSim);
                } else if(mousePosX < 2 * WINDOW_WIDTH / 3){
                    run_step_frames_press_a(simState, autoAdvanceSim);
                } else {
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
bool change_simState_on_box_click(Uint32 mousePosX, Uint32 mousePosY, int x0, int y0, int x1, int y1, int newSimStateIfClicked){
    if(mousePosX < x0 || mousePosX >= x1) return false;
    if(mousePosY < y0 || mousePosY >= y1) return false;
    simState = newSimStateIfClicked;
    return true;
}
// Return true if the box was clicked
bool increment_var_on_box_click(Uint32 mousePosX, Uint32 mousePosY, int xLb, int yLb, int xUb, int yUb, int* pVarToChange, bool doIncrease){
    // lb = lower bound, ub = upper bound, x and y are coordinates
    //cout << "xLb: " << xLb << ", xUb: " << xUb << ", yLb: " << yLb << ", yUb: " << yUb << endl;
    if(mousePosX < xLb || mousePosX >= xUb) return false;
    if(mousePosY < yLb || mousePosY >= yUb) return false;
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
    enforce_global_param_bounds();
    return true;
}
void draw_options_menu(int x0, int dx, int dy, std::vector<std::pair<int, string>>& optionText);
void draw_main_menu(std::vector<int>& xVec, std::vector<int>& yVec, std::vector<std::pair<string, int*>>& simParamsText, int xLbStart, int yLbStart, int dyStart);
void SDL_draw_frame();
void run_sim_state_options_menu(SDL_Event& windowEvent, bool& pauseSim, int& simState){
    std::vector<std::pair<int, string>> optionText;
    #define wfx(fraction) (0 + (WINDOW_WIDTH-0)*fraction)
    #define wfy(fraction) (0 + (WINDOW_HEIGHT-0)*fraction)
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
    SDL_draw_frame();
}
// Allow the user to customize various simulation parameters before each simulation
void run_sim_state_main_menu(SDL_Event& windowEvent, bool& pauseSim, int& simState){
    frameStart = SDL_GetTicks();
    std::vector<std::pair<string, int*>> simParamsText;
    simParamsText.push_back({"Initial # of Cells", &initNumCells});
    simParamsText.push_back({"Max # of Cells", &cellLimit});
    simParamsText.push_back({"Day Length", &dayLenSec});
    simParamsText.push_back({"Max Sun Energy Regeneration", &maxSunEnergyPerSec});
    simParamsText.push_back({"Max Ground Energy per cell", &maxGndEnergy});
    simParamsText.push_back({"Ground Energy Regeneration", &gndEnergyPerIncrease});
    simParamsText.push_back({"Force Damping Factor", &forceDampingFactor});
    simParamsText.push_back({"Overcrowding energy coefficient", &overcrowdingEnergyCoef});

    // Specify the x and y coordinates to draw the text and allow it to respond when clicking on it
    #define wfx(fraction) (int)(fraction*WINDOW_WIDTH)
    #define wfy(fraction) (int)(fraction*WINDOW_HEIGHT)
    std::vector<int> xVec = {wfx(0.05), wfx(0.6), wfx(0.8), wfx(0.84), wfx(0.88)};
    int y0 = wfy(0), y1 = wfy(0.2), dy = 0.075*WINDOW_HEIGHT;
    std::vector<int> yVec = {y0};
    for(int i = 0; i < simParamsText.size(); i++) yVec.push_back(y1 + i*dy);
    //cout << "yVec: { "; for(auto num : yVec) cout << num << " "; cout << "}\n";
    int xLbStart = xVec[2], yLbStart = y0, dyStart = 0.75*(y1-y0);
    #undef wfx
    #undef wfy

    #define deal_with_box_click(xLb, xUb, yLb, yUb, pVarToIncrement, doIncrease) increment_var_on_box_click(mousePosX, mousePosY, xLb, yLb, xUb, yUb, pVarToIncrement, doIncrease)
    while(simState == SIM_STATE_MAIN_MENU){
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
                int* pParameter = simParamsText[i].second;
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
    SDL_draw_frame();
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



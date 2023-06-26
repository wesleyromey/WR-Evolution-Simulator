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


// Return true if sim state changed
bool change_simState_on_box_click(Uint32 mousePosX, Uint32 mousePosY, int x0, int y0, int x1, int y1, int newSimStateIfClicked){
    if(mousePosX < x0 || mousePosX >= x1) return false;
    if(mousePosY < y0 || mousePosY >= y1) return false;
    simState = newSimStateIfClicked;
    return true;
}
void draw_options_menu(int x0, int dx, int dy, vector<pair<int, string>> optionText);
void SDL_draw_frame();
void run_sim_state_options_menu(SDL_Event& windowEvent, bool& pauseSim, int& simState){
    vector<pair<int, string>> optionText;
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
            frameStart = SDL_GetTicks();
            SDL_WaitEvent(&windowEvent);
            simState = SIM_STATE_STEP_FRAMES;
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
    if(simState == SIM_STATE_RESTART) cout << "Restarting Sim" << endl;
}



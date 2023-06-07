// This file contains most of the SDL functions
//  required to display the User Interface (UI)
//  including graphics, buttons, menus, etc.

#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif


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


void run_step_frames_press_n(bool& pauseSim, unsigned int& autoAdvanceSim){
    pauseSim = false;
    autoAdvanceSim = 0;
}
void run_step_frames_press_a(int& simState, unsigned int& autoAdvanceSim){
    simState = SIM_STATE_SKIP_FRAMES;
    autoAdvanceSim = AUTO_ADVANCE_DEFAULT;
    cout << "a is pressed! Simulation is speeding up for " << autoAdvanceSim << " frames" << endl;
}

void run_sim_state_step_frames(SDL_Event& windowEvent, bool& pauseSim, bool& simIsRunning, unsigned int& autoAdvanceSim, int& simState){
    SDL_WaitEvent(&windowEvent);
    Uint32 mouseClickType = 0;
    switch(windowEvent.type){
        case SDL_KEYDOWN:
        switch(windowEvent.key.keysym.sym){
            case SDLK_ESCAPE:
            simIsRunning = false;
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
                    // Options
                    cout << "NOTE: Options are NOT available yet!\n";
                }
            }
        }
        break;
        case SDL_QUIT:
        simState = SIM_STATE_QUIT;
        simIsRunning = false;
        pauseSim = false;
        break;
    }
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
            SDL_WaitEvent(&windowEvent);
            simState = SIM_STATE_STEP_FRAMES;
            break;
            case SIM_STATE_STEP_FRAMES:
            run_sim_state_step_frames(windowEvent, pauseSim, simIsRunning, autoAdvanceSim, simState);
            break;
            case SIM_STATE_SKIP_FRAMES:
            if(autoAdvanceSim){
                autoAdvanceSim--;
                pauseSim = false;
            } else simState = SIM_STATE_STEP_FRAMES;
            break;
            case SIM_STATE_QUIT:
            pauseSim = false;
            simIsRunning = false;
            break;
        }
    }
    if(!simIsRunning) std::cout << "Simulation will exit..." << std::endl;
}



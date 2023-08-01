#ifndef INCLUDE_4_H
#include "include4/include4.h"
#define INCLUDE_4_H
#endif


// TODO: Display this as a text message within the simulator itself
void dispIntroMsg(){
    std::cout << "\nWelcome to the WR Evolution Simulator! ";
    std::cout << "See https://github.com/wesleyromey/WR-Evolution-Simulator for ";
    std::cout << "the source code and license info.\n";
}

int main(int argc, char* argv[]){
#ifdef DO_VIDEO
    automateEnergy = false; enableAutomaticAttack = false; enableAutomaticSelfDestruct = false; enableAutomaticCloning = false;
#endif
    dispIntroMsg();
    simState = SIM_STATE_MAIN_MENU;
#ifdef DEBUG
    //testForce();
    //testAi();
    //testStats();
    //testFrames();
    //testGlobalEnergy();
    //test_SDL();
    //test_event_handler();
    //test_new_tex();
    //test_cur_tex();
    redraw_existing_tex();
#else
    while(simState != SIM_STATE_QUIT){
        do_sim_iteration();
    }
    //int drawCenterX = 100, drawCenterY = 100, radius = 100, numVertices = 32;
    //SDL_Color _color = {0xff, 0xff, 0xff, 0x40};
    //draw_regular_polygon(drawCenterX, drawCenterY, radius, numVertices, _color);
    
    //SDL_RenderPresent(P_RENDERER);
    //SDL_Event windowEvent;
    //bool flag = false;
    //while(!flag){
    //    SDL_WaitEvent(&windowEvent);
    //    if(windowEvent.type == SDL_QUIT) flag = true;
    //}
#endif
    if(simState == SIM_STATE_QUIT) exit_sim();
    return 0;
}



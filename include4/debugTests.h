#ifndef TERTIARY_INCLUDES_H
#include "../tertiary/tertiaryIncludes.h"
#define TERTIARY_INCLUDES_H
#endif

#ifndef SIM_H
#include "sim.h"
#define SIM_H
#endif


// This file is meant to store all the unit tests and other debug mode only
// test functions that don't belong in the main program


// Show people what every portion of the cell refers to

// Test the global energy-related parameters, such as day-night cycles
void testGlobalEnergy(){
    gen_cell();
    std::map<std::string, int> hardcodedVals = {
        {"attack", 1}, {"dia", 1}, {"EAM[EAM_CELLS]", 0}, {"EAM[EAM_GND]", 0}, {"EAM[EAM_SUN]", 100},
        {"health", 1}, {"maxHealth", 1}, {"mutationRate", 1000}, {"speedRun", 3}, {"speedWalk", 1},
        {"visionDist", 0}
    };
    pAlives[0]->set_int_stats(hardcodedVals);
    init_sim_global_vals();

    // Test the day-night cycle, sun energy, and ground energy (Test complete!)
    std::vector<int> solarEnergyPerUnitTime;
    for(int i = 0; i < dayLenSec.val; i++){
        do_frame(i);
        solarEnergyPerUnitTime.push_back(energyFromSunPerSec);
    }
    // Display the results
    std::cout << "solarEnergyPerUnitTime: { ";
    for(auto item : solarEnergyPerUnitTime) std::cout << item << " ";
    std::cout << "}\n";

    // Test the energy from dead cells (TODO)
    gen_cell();
    hardcodedVals = {
        {"attack", 1}, {"dia", 1}, {"EAM[EAM_CELLS]", 40}, {"EAM[EAM_GND]", 30}, {"EAM[EAM_SUN]", 30},
        {"health", 1}, {"maxHealth", 1}, {"mutationRate", 1000}, {"speedRun", 3}, {"speedWalk", 1},
        {"visionDist", 0}
    };
    pAlives[0]->set_int_stats(hardcodedVals);
    do_frame(0);
    gen_dead_cell();
    gen_dead_cell(pAlives[0], 0);
    for(int i = 1; pDeads.size() > 0; i++){
        int initSize = pDeads.size();
        do_frame(i);
        if(pDeads.size() != initSize){
            std::cout << initSize - pDeads.size() << " dead cells were removed at frame " << i << std::endl;
        }
    }
    
    // Delete cells
    exit_sim();
}

// A function devoted to testing the progression of frames within the simulation
//  (e.g. time)
void testFrames(){
    // We will first test a single cell's ability to absorb energy
    gen_cell();
    std::map<std::string, int> hardcodedVals = {
        {"attack", 1}, {"dia", 1}, {"EAM[EAM_CELLS]", 0}, {"EAM[EAM_GND]", 0}, {"EAM[EAM_SUN]", 100},
        {"health", 1}, {"maxHealth", 1}, {"mutationRate", 1000}, {"speedRun", 3}, {"speedWalk", 1},
        {"visionDist", 0}
    };
    pAlives[0]->set_int_stats(hardcodedVals);
    do_frame(0);
    exit_sim();
}

// A function devoted to testing the energy usage of the cell for cloning,
//  surviving, and using its abilities
void testStats(){
    gen_cell();
    std::map<std::string, int> varVals = {
        {"age", 11}, {"aggression", 11}, {"attack", 11}, {"attackCooldown", 11},
        {"cloningDirection", 11}, 
        {"dia", 11}, {"doAttack", true}, {"doSelfDestruct", true},
        {"EAM[EAM_CELLS]", 11}, {"EAM[EAM_GND]", 11}, {"EAM[EAM_SUN]", 78}, {"energy", 111},
        {"health", 111}, {"maxHealth", 1111}, {"mutationRate", 111}, {"posX", 11}, {"posY", 11},
        {"speedDir", 11}, {"speedMode", RUN_MODE}, {"speedRun", 111}, {"speedWalk", 11},
        {"stickiness", 11}, {"visionDist", 11},
    };
    pAlives[0]->set_int_stats(varVals);
    exit_sim();
}

// A function devoted to testing the cell AI
void testAi(){
    std::cout << "Test the ai and its cloning / mutation properties:" << std::endl;
    gen_cell();
    gen_cell();
    gen_cell(pAlives[1]);
    gen_cell(pAlives[2], false, 0);
    std::cout << "mutationRate of 1st cell: " << pAlives[1]->mutationRate << std::endl;
    std::cout << "The 2nd cell is a perfect clone of the 1st cell\n";
    std::cout << "The 3rd cell is a mutated clone of the 2nd cell\n";
    std::cout << "The 0th cell is completely unrelated from the other cells\n";
    for(auto pCell : pAlives) pCell->print_id();
    exit_sim();
}

// A function devoted to testing the application of forces to a cell
void testForce(){
    std::cout << "Need UB_X = 100 and UB_Y = 100 ";
    assert(UB_X == 100 && UB_Y == 100);
    std::cout << "(done!)\n";
    std::cout << "forceDampingFactor.val: " << forceDampingFactor.val << "\n\n";

    int numCells = 2;
    randomly_place_new_cells(numCells);
    for (int i = 0; i < numCells; i++) {
        pAlives[i]->speedMode = IDLE_MODE;
        pAlives[i]->stickiness = 0;
        pAlives[i]->dia = 1;
        pAlives[i]->update_size();
    }

    const bool DO_DIA_1_TESTS = true;
    const bool DO_DIA_10_TESTS = true; 
    if(DO_DIA_1_TESTS){
        // Ensure all cells have a speed of 0 and a diameter of 1
        std::cout << "Testing cells with diameter 1";
        // Place the cells in various locations to test the different options
        pAlives[0]->update_pos(30, 30); pAlives[1]->update_pos(30, 30);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
        std::cout << "\n";
    }
    if(DO_DIA_10_TESTS){
        std::cout << "Testing a diameter of 10\n";
        for (int i = 0; i < numCells; i++) {
            pAlives[i]->dia = 10;
            pAlives[i]->update_size();
        }
        pAlives[0]->update_pos(30, 30); pAlives[1]->update_pos(30, 30);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
        pAlives[0]->update_pos(50, 50); pAlives[1]->update_pos(51, 50);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
        pAlives[0]->update_pos(50, 50); pAlives[1]->update_pos(50, 51);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
        pAlives[0]->update_pos(50, 50); pAlives[1]->update_pos(51, 51);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
        pAlives[0]->update_pos(0, 0); pAlives[1]->update_pos(99, 99);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
        pAlives[0]->update_pos(20, 20); pAlives[1]->update_pos(22, 24);
        print_cell_coords(pAlives); do_frame(false); print_cell_coords(pAlives);
    }

    exit_sim();
}

void test_SDL(){
    for(int frameNum = 0; frameNum < 100; frameNum++){
        draw_bkgnd(500);
        draw_texture(pCellSkeleton, 0, frameNum, 100, 100);
        draw_texture(pEnergyGnd0pctTex, 100-frameNum, 100+frameNum, 100, 100);
        draw_texture(pHealth70pctTex, 300, 100+2*frameNum,
            150-frameNum/2, 150-frameNum/2);
        SDL_RenderPresent(P_RENDERER);
        //  Displays the current frame (of textures) to the user
    }
}

void test_event_handler(){
    randomly_place_new_cells(10);
    int frameNum = 0;
    while(simState != SIM_STATE_QUIT){
        do_frame(frameNum++);
    }
}

void test_new_tex(){
    int imgWidth = 10;
    int imgHeight = 10;
    int numChannels = 3;
    unsigned char RGB[3] = {0xc3, 0xc3, 0xc3};
    unsigned char filled[] = {
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],

        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
        RGB[0], RGB[1], RGB[2], RGB[0], RGB[1], RGB[2],
    };

    imgWidth = 3;
    imgHeight = 3;
    numChannels = 3;
    #define RGB "abcd"
    #undef RGB
    #define RGB 0x00, 0x00, 0x00
    //  NOTE: It is possible to use #define to redefine a constant
    //  (best to use #undef first)
    unsigned char black3channels[] = {
        RGB, RGB, RGB,
        RGB, RGB, RGB, 
        RGB, RGB, RGB,
    };
    SDL_Surface* pBlackSurface3 = SDL_CreateRGBSurfaceFrom((void*)black3channels,
        imgWidth, imgHeight, numChannels*8, imgWidth*numChannels,
        0x0000ff, 0x00ff00, 0xff0000, 0
    );
    SDL_FreeSurface(pBlackSurface3);
    std::cout << pBlackSurface3 << endl;
    SDL_Texture* pBlackTex3 = SDL_CreateTextureFromSurface(P_RENDERER, pBlackSurface3);


    numChannels = 4;
    #define RGBA 0xFF, 0xFF, 0xFF, 0xFF
    #define ___0 0x00, 0x00, 0x00, 0x00
    unsigned char white4channels[] = {
        RGBA, ___0, ___0,
        ___0, RGBA, ___0,
        ___0, ___0, RGBA,
    };
    SDL_Texture* pWhiteTex4 = convArrToSDLTex(white4channels, 3, 3);
    /*
    SDL_Surface* pWhiteSurface4 = SDL_CreateRGBSurfaceFrom((void*)white4channels,
        imgWidth, imgHeight, numChannels*8, imgWidth*numChannels,
        0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
    );
    cout << pWhiteSurface4 << endl;
    SDL_Texture* pWhiteTex4 = SDL_CreateTextureFromSurface(P_RENDERER, pWhiteSurface4);
    */
    #undef RGBA
    #undef ___0

    int row = 0, col = 0;
    simState = SIM_STATE_STEP_FRAMES;
    while(simState != SIM_STATE_QUIT){
        ++row; ++col;
        row %= UB_X; col %= UB_Y;
        int drawX = drawScaleFactor*row;
        int drawY = drawScaleFactor*col;
        int drawSize = drawScaleFactor*10;
        SDL_RenderClear(P_RENDERER);
        draw_bkgnd(1000);
        draw_texture(pBlackTex3, drawX, drawY, drawSize, drawSize);
        draw_texture(pWhiteTex4, drawX, drawY, drawSize, drawSize);
        SDL_RenderPresent(P_RENDERER);
        SDL_event_handler();
    }
}

void test_cur_tex(){
    int imgWidth = 3, imgHeight = 3, numChannels = 4;

    #define _BLK 0x00, 0x00, 0xff, 0xff
    unsigned char _tmpImg[] = {
        _BLK, _BLK, _BLK,
        _BLK, _BLK, _BLK,
        _BLK, _BLK, _BLK,
    };
    SDL_Texture* pTex = convArrToSDLTex(_tmpImg, imgWidth, imgHeight);
    #undef _BLK

    int row = 0, col = 0;
    simState = SIM_STATE_STEP_FRAMES;
    randomly_place_new_cells(10);
    while(simState != SIM_STATE_QUIT){
        ++row; ++col;
        row %= UB_X; col %= UB_Y;
        int drawX = drawScaleFactor*row;
        int drawY = drawScaleFactor*col;
        int drawSize = drawScaleFactor*2;
        energyFromSunPerSec = 1000;
        SDL_draw_frame();
        SDL_event_handler();
    }
}

// Change an existing texture and draw it at its (possibly) new x and y position
void redraw_existing_tex(){
    // xPos = 0, yPos = 0 refers to the top left corner of the window
    // Not sure what pSrc refers to.
    //  pSrc may represent the original object, but I'm not sure
    //  If pSrc == NULL, then a new texture is rendered
    // pDst represents the new object
    //  If dst == NULL, then the texture fills the entire window

    // Inputs
    SDL_Texture* pTexture = p_V_Symbol;
    int xPos = 12*drawScaleFactor, yPos = 10*drawScaleFactor;
    int width = 5*drawScaleFactor, height = 10*drawScaleFactor;

    // Original Function
    SDL_Rect* pDst = new SDL_Rect;
    pDst->x = xPos; pDst->y = yPos;
    pDst->h = height; pDst->w = width;
    //SDL_RenderCopy(pRenderer, pTexture, pSrc, pDst);
    SDL_RenderCopy(P_RENDERER, pTexture, NULL, pDst);
    //  This renders the opject according to pDst
    //  I could replace NULL with pSrc, but I don't know what pSrc refers to

    // Post-function necessities
    SDL_RenderPresent(P_RENDERER);
    SDL_Event windowEvent;
    int count = 0;
    bool exitSim = false;
    Uint32 mouseClickType = 0;
    while(count < 100 && !exitSim){
        Uint32 frameStart = SDL_GetTicks();
        SDL_WaitEvent(&windowEvent);
        switch(windowEvent.type){
            case SDL_MOUSEMOTION:
            mouseClickType = SDL_GetMouseState(&mousePosX, &mousePosY);
            pDst->x = mousePosX; pDst->y = mousePosY;
            SDL_RenderCopy(P_RENDERER, pTexture, NULL, pDst);
            SDL_RenderPresent(P_RENDERER);
            enforce_frame_rate(frameStart, FRAME_DELAY);
            count++;
            break;
            case SDL_QUIT:
            exitSim = true;
            break;
        }
    }
    std::cout << "sim will end now." << endl;
    delete pDst;
}


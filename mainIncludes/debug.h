// Uncomment this if you want you want to run the program in debug mode
//  Comment this code out if you want to run the actual program

//#define DEBUG
//#define DEBUG_FRAMES
//  Like #define DEBUG, except specifically for frames (MUST draw the textures in the function(s)
//  devoted to drawing frames)


// Determine what types of organisms initially spawn
//#define PLANT_INIT


// Draw video frames, text, etc.
#define DO_VIDEO_FRAMES
#define DO_VIDEO_TEXT
//#define DO_WHITE_TEXT

#if defined(DO_VIDEO_FRAMES) || defined(DO_VIDEO_TEXT)
#define DO_VIDEO
#endif

// TODO: Ensure the code works fully without the DeadCell struct 
#define ENABLE_NEW_P_DEADS


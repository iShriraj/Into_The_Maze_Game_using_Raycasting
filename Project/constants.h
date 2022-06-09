#include <math.h>

#define FALSE 0
#define TRUE 1

#define PI 3.14159265
#define TWO_PI 6.28318530

//Defining Constants

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20
#define NUM_TEXTURES 8

//Size of mini-map
#define MINIMAP_SCALE_FACTOR 0.2

//Window width and Window Height
#define WINDOW_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define WINDOW_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64

#define FOV_ANGLE (60 * (PI / 180))

#define NUM_RAYS WINDOW_WIDTH

#define DIST_PROJ_PLANE (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2)

//Frames per second
#define FPS 30
#define FRAME_TIME_LENGTH (1000 / FPS)

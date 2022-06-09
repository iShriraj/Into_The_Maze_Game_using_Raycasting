#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <SDL.h>
#include "constants.h"
#include "textures.h"

//Defining the 2D map
const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6},
    {6, 0, 6, 6, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 6, 6, 0, 6},
    {6, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 6, 6, 0, 0, 0, 0, 6},
    {6, 0, 6, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 6},
    {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 6, 0, 6},
    {6, 0, 0, 6, 0, 6, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 6},
    {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 6, 6, 6, 6, 0, 6},
    {6, 0, 0, 6, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6},
    {6, 0, 0, 6, 0, 6, 0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 6, 6, 0, 6},
    {6, 0, 0, 6, 6, 6, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 6},
    {6, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
};

//Player attributes
struct Player {
    
    //starting points of the player
    float x;
    float y;
    float width;
    float height;
    int turnDirection; // -1 for left, +1 for right
    int walkDirection; // -1 for back, +1 for front
    float rotationAngle;
    float walkSpeed;
    float turnSpeed;
} player;

//Ray attribues
struct Ray {
    float rayAngle;
    float wallHitX;
    float wallHitY;
    float distance;
    int wasHitVertical;
    int isRayFacingUp;
    int isRayFacingDown;
    int isRayFacingLeft;
    int isRayFacingRight;
    int wallHitContent;
} rays[NUM_RAYS];

//SDL variables initialization
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = FALSE;
int ticksLastFrame;

unsigned int* colorBuffer = NULL;
SDL_Texture* colorBufferTexture;
unsigned int* textures[NUM_TEXTURES];

//Initialize the game window
int initializeWindow() {
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf(stderr, "Error initializing SDL.\n");
        return FALSE;
    }
    //Create window
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_BORDERLESS
    );

    if (!window) {
        printf(stderr, "Error creating SDL window.\n");
        return FALSE;
    }
    //Create Renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        printf(stderr, "Error creating SDL renderer.\n");
        return FALSE;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return TRUE;
}

//Function for terminating the program
void destroyWindow() {
    free(colorBuffer);
    SDL_DestroyTexture(colorBufferTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

//Setup game attributes
void setup() {
    player.x = WINDOW_WIDTH / 2;
    player.y = WINDOW_HEIGHT / 2;
    player.width = 1;
    player.height = 1;
    player.turnDirection = 0;
    player.walkDirection = 0;
    player.rotationAngle = PI / 2;
    player.walkSpeed = 150;
    player.turnSpeed = 45 * (PI / 180);

    // allocate the total amount of bytes in memory to hold colorbuffer
    colorBuffer = (unsigned int*)malloc(sizeof(unsigned int) * WINDOW_WIDTH * WINDOW_HEIGHT);

    // create an SDL_Texture to display the colorbuffer
    colorBufferTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    //textures from the textures.h
    textures[0] = (unsigned int*)REDBRICK_TEXTURE;
    textures[1] = (unsigned int*)PURPLESTONE_TEXTURE;
    textures[2] = (unsigned int*)MOSSYSTONE_TEXTURE;
    textures[3] = (unsigned int*)GRAYSTONE_TEXTURE;
    textures[4] = (unsigned int*)COLORSTONE_TEXTURE;
    textures[5] = (unsigned int*)BLUESTONE_TEXTURE;
}

//Check if map has wall at given location
int mapHasWallAt(float x, float y) {
    if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
        return TRUE;
    }
    int mapGridIndexX = floor(x / TILE_SIZE);
    int mapGridIndexY = floor(y / TILE_SIZE);
    return map[mapGridIndexY][mapGridIndexX] != 0;
}

//Player movement
void movePlayer(float deltaTime) {
    player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
   
    //player move step
    float moveStep = player.walkDirection * player.walkSpeed * deltaTime;

    //new position of player
    float newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
    float newPlayerY = player.y + sin(player.rotationAngle) * moveStep;

    //player can move untill there is no wall
    if (!mapHasWallAt(newPlayerX, newPlayerY)) {
        player.x = newPlayerX;
        player.y = newPlayerY;
    }
}

//Render player in Map
void renderPlayer() {
   
    //Show player
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect playerRect = {
        player.x * MINIMAP_SCALE_FACTOR,
        player.y * MINIMAP_SCALE_FACTOR,
        player.width * MINIMAP_SCALE_FACTOR,
        player.height * MINIMAP_SCALE_FACTOR
    };
    SDL_RenderFillRect(renderer, &playerRect);

    //Show player direction
    SDL_RenderDrawLine(
        renderer,
        MINIMAP_SCALE_FACTOR * player.x,
        MINIMAP_SCALE_FACTOR * player.y,
        MINIMAP_SCALE_FACTOR * player.x + cos(player.rotationAngle) * 40,
        MINIMAP_SCALE_FACTOR * player.y + sin(player.rotationAngle) * 40
    );
}

//Normalize function defination
float normalizeAngle(float angle) {
 
     //not beyond 360 degree
    angle = remainder(angle, TWO_PI);
    if (angle < 0) {
        angle = TWO_PI + angle;
    }
    return angle;
}

//Distance between two points
float distanceBetweenPoints(float x1, float y1, float x2, float y2) {
    
    //calculate distance using Pythagoras Theorem
    
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

//Casting a Ray
void castRay(float rayAngle, int stripId) {
   
    //Initialize ray angle
    rayAngle = normalizeAngle(rayAngle);

    int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
    int isRayFacingUp = !isRayFacingDown;

    int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
    int isRayFacingLeft = !isRayFacingRight;

    float xintercept, yintercept;
    float xstep, ystep;

    
    // HORIZONTAL RAY-GRID INTERSECTION CODE
    
    int foundHorzWallHit = FALSE;
    float horzWallHitX = 0;
    float horzWallHitY = 0;
    int horzWallContent = 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isRayFacingDown ? TILE_SIZE : 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

    // Calculate the increment xstep and ystep
    ystep = TILE_SIZE;
    ystep *= isRayFacingUp ? -1 : 1;

    xstep = TILE_SIZE / tan(rayAngle);

    //ray direction for incrementing/decrementing xstep
    xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
    xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

    float nextHorzTouchX = xintercept;
    float nextHorzTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextHorzTouchX;
        float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundHorzWallHit = TRUE;
            break;
        }
        else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    
    // VERTICAL RAY-GRID INTERSECTION CODE
    
    int foundVertWallHit = FALSE;
    float vertWallHitX = 0;
    float vertWallHitY = 0;
    int vertWallContent = 0;

    // Find the x-coordinate of the closest horizontal grid intersection
    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isRayFacingRight ? TILE_SIZE : 0;

    // Find the y-coordinate of the closest horizontal grid intersection
    yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

    // Calculate the increment xstep and ystep
    xstep = TILE_SIZE;
    xstep *= isRayFacingLeft ? -1 : 1;

    ystep = TILE_SIZE * tan(rayAngle);

    //increment or decrement ystep according to the direction
    ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

    float nextVertTouchX = xintercept;
    float nextVertTouchY = yintercept;

    // Increment xstep and ystep until we find a wall
    while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= WINDOW_HEIGHT) {
        float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
        float yToCheck = nextVertTouchY;

        if (mapHasWallAt(xToCheck, yToCheck)) {
            // found a wall hit
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
            foundVertWallHit = TRUE;
            break;
        }
        else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    // Calculate both horizontal and vertical hit distances and choose the smallest one
    float horzHitDistance = foundHorzWallHit
        ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
        : INT_MAX;
    float vertHitDistance = foundVertWallHit
        ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
        : INT_MAX;

    // only store the smallest of the distances
    if (vertHitDistance < horzHitDistance) {
        rays[stripId].distance = vertHitDistance;
        rays[stripId].wallHitX = vertWallHitX;
        rays[stripId].wallHitY = vertWallHitY;
        rays[stripId].wallHitContent = vertWallContent;
        rays[stripId].wasHitVertical = TRUE;
    }
    else {
        rays[stripId].distance = horzHitDistance;
        rays[stripId].wallHitX = horzWallHitX;
        rays[stripId].wallHitY = horzWallHitY;
        rays[stripId].wallHitContent = horzWallContent;
        rays[stripId].wasHitVertical = FALSE;
    }
    rays[stripId].rayAngle = rayAngle;
    rays[stripId].isRayFacingDown = isRayFacingDown;
    rays[stripId].isRayFacingUp = isRayFacingUp;
    rays[stripId].isRayFacingLeft = isRayFacingLeft;
    rays[stripId].isRayFacingRight = isRayFacingRight;
}

//Casting all Rays
void castAllRays() {
   
    // start first ray subtracting half of the FOV
    float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);

    // loop all columns casting the rays
    for (int stripId = 0; stripId < NUM_RAYS; stripId++) {
        castRay(rayAngle, stripId);
        rayAngle += FOV_ANGLE / NUM_RAYS;
    }
}

//Render mini-map
void renderMap() {
    for (int i = 0; i < MAP_NUM_ROWS; i++) {
        for (int j = 0; j < MAP_NUM_COLS; j++) {
            int tileX = j * TILE_SIZE;
            int tileY = i * TILE_SIZE;
            int tileColor = map[i][j] != 0 ? 255 : 0;

            SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
            SDL_Rect mapTileRect = {
                tileX * MINIMAP_SCALE_FACTOR,
                tileY * MINIMAP_SCALE_FACTOR,
                TILE_SIZE * MINIMAP_SCALE_FACTOR,
                TILE_SIZE * MINIMAP_SCALE_FACTOR
            };
            SDL_RenderFillRect(renderer, &mapTileRect);
        }
    }
}

//Render Rays
void renderRays() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < NUM_RAYS; i++) {
        SDL_RenderDrawLine(
            renderer,
            MINIMAP_SCALE_FACTOR * player.x,
            MINIMAP_SCALE_FACTOR * player.y,
            MINIMAP_SCALE_FACTOR * rays[i].wallHitX,
            MINIMAP_SCALE_FACTOR * rays[i].wallHitY
        );
    }
}

//Control / operate game
void processInput() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
    case SDL_QUIT: {
        isGameRunning = FALSE;
        break;
    }
    case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_ESCAPE)
            isGameRunning = FALSE;
        if (event.key.keysym.sym == SDLK_UP)
            player.walkDirection = +1;
        if (event.key.keysym.sym == SDLK_DOWN)
            player.walkDirection = -1;
        if (event.key.keysym.sym == SDLK_RIGHT)
            player.turnDirection = +1;
        if (event.key.keysym.sym == SDLK_LEFT)
            player.turnDirection = -1;
        break;
    }
    case SDL_KEYUP: {
        if (event.key.keysym.sym == SDLK_UP)
            player.walkDirection = 0;
        if (event.key.keysym.sym == SDLK_DOWN)
            player.walkDirection = 0;
        if (event.key.keysym.sym == SDLK_RIGHT)
            player.turnDirection = 0;
        if (event.key.keysym.sym == SDLK_LEFT)
            player.turnDirection = 0;
        break;
    }
    }
}

//Update function for updating player position , frames
void update() {

    // waste some time until we reach the target frame time length
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksLastFrame + FRAME_TIME_LENGTH));

    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

    ticksLastFrame = SDL_GetTicks();

    movePlayer(deltaTime);
    castAllRays();
}

//Generate 3D projection of walls
void generate3DProjection() {

    // loop every ray in the array of rays
    for (int i = 0; i < NUM_RAYS; i++) {

        // get the perpendicular distance to the wall to fix fishbowl distortion
        float perpDistance = rays[i].distance * cos(rays[i].rayAngle - player.rotationAngle);

        // calculate the distance to the projection plane
        float distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);

        // projected wall height
        float projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        int textureOffsetX;
        if (rays[i].wasHitVertical)
            textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
        else
            textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;

        // get the correct texture id number from the map content
        int texNum = rays[i].wallHitContent - 1;

        // render the wall from wallTopPixel to wallBottomPixel
        for (int y = wallTopPixel; y < wallBottomPixel; y++) {
            int distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
            int textureOffsetY = distanceFromTop * ((float)TEXTURE_HEIGHT / wallStripHeight);

            // set the color of the wall based on the color from the texture
            unsigned int texelColor = textures[texNum][(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
            colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
        }

        // set the color of the ceiling
        for (int y = 0; y < wallTopPixel; y++)
            colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF000000;

        // set the color of the floor
        for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
            colorBuffer[(WINDOW_WIDTH * y) + i] = 0xFF000000;
    }
}

//clear the colour buffer for memory utilization
void clearColorBuffer(unsigned int color) {
    for (int x = 0; x < WINDOW_WIDTH; x++)
        for (int y = 0; y < WINDOW_HEIGHT; y++)
            colorBuffer[(WINDOW_WIDTH * y) + x] = color;
}

//render colour buffer
void renderColorBuffer() {
    SDL_UpdateTexture(
        colorBufferTexture,
        NULL,
        colorBuffer,
        (int)(WINDOW_WIDTH * sizeof(unsigned int))
    );
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

//render function defination
void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    generate3DProjection();

    //Projecting 3D walls
    renderColorBuffer();
    clearColorBuffer(0xFF000000);

    renderMap();
    renderRays();
    renderPlayer();

    SDL_RenderPresent(renderer);
}

//main function
int main() {
    isGameRunning = initializeWindow();

    setup();

    while (isGameRunning) {
        processInput();
        update();
        render();
    }

    destroyWindow();

    return 0;
}

/* *********************************************************************** */
/*                                                                         */
/* OBJECTIVE : 3D                          #####      ###    ###    #      */
/* AUTHOR :  VICTOR DALET                  #         #      #       #      */
/* CREATED : 15 10 2022                    ####      #      #  ##   #      */
/* UPDATE  : 15 10 2022                    #         #      #   #   #      */
/*                                         ####    ###      #####   #.fr   */
/* *********************************************************************** */




#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "include/SDL.h"
#include "constante.h"
#include "upng.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1},
        {1, _, _, _, 1, _, 1, _, 1, _, 1, _, 1, _, 1, _, 1, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, 1, 1, 1, 1, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1},
        {1, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

struct Player {
    double x;
    double y;
    double width;
    double height;
    int turnDirection; // -1 left, 1 right
    int walkDirection; // -1 back, 1 front
    double rotationAngle;
    double walkSpeed;
    double turnSpeed;
} player;

struct Ray {
    double angle;
    double wallHitX;
    double wallHitY;
    double distance;
    int wasVerticalHit;
    int isFacingUp;
    int isFacingDown;
    int isFacingRight;
    int isFacingLeft;
    int wallHitContent;
} rays[NUM_RAYS];

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int isGameRunning = FALSE;
double ticksLastFrame;

Uint32* colorBuffer = NULL;
SDL_Texture* colorBufferTexture = NULL;
Uint32* wallTexture = NULL;
upng_t* pngTexture;

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
                    TILE_SIZE * MINIMAP_SCALE_FACTOR};
            SDL_RenderFillRect(renderer, &mapTileRect);
        }
    }
}

int hasWallAt(double x, double y) {
    if(x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT)
        return TRUE;
    int gridX = floor(x / TILE_SIZE);
    int gridY = floor(y / TILE_SIZE);
    int tile = map[gridY][gridX];
    return tile != 0;
}

void updatePlayer(double deltatime) {
    player.rotationAngle += player.turnDirection * player.turnSpeed * deltatime;

    double moveStep = player.walkDirection * player.walkSpeed * deltatime;

    double newX = player.x + cos(player.rotationAngle) * moveStep;
    double newY = player.y + sin(player.rotationAngle) * moveStep;

    if(!hasWallAt(newX, newY)){
        player.x = newX;
        player.y = newY;
    }
}


int isInsideScreen(double x, double y) {
    if(x >=0 && x <= WINDOW_WIDTH && y >=0 && y <= WINDOW_HEIGHT)
        return TRUE;
    return FALSE;
}

double normalizeAngle(double angle) {
    angle = remainder(angle, TWO_PI);
    if(angle < 0)
        angle = TWO_PI + angle;
    return angle;
}

double distanceBetweenPoints(double x1, double y1, double x2, double y2) {
    return sqrt((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1));
}


void castRay(double angle, int id) {
    angle = normalizeAngle(angle);

    int isFacingDown = angle > 0 && angle < PI;
    int isFacingUp = !isFacingDown;

    int isFacingRight = angle < 0.5 * PI || angle > 1.5 * PI;
    int isFacingLeft = !isFacingRight;

    double xintercept, yintercept;
    double xstep, ystep;

    //***********************
    //Horizontal intersection
    //***********************

    int foundHorzWallHit = FALSE;
    double horzWallHitX = 0;
    double horzWallHitY = 0;
    int horzWallContent = 0;

    yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
    yintercept += isFacingDown ? TILE_SIZE : 0;

    xintercept = player.x + (yintercept - player.y) / tan(angle);

    ystep = TILE_SIZE;
    ystep *= isFacingUp ? -1 : 1;

    xstep = TILE_SIZE / tan(angle);
    xstep *= (isFacingLeft && xstep > 0) ? -1 : 1;
    xstep *= (isFacingRight && xstep < 0) ? -1 : 1;

    double nextHorzTouchX = xintercept;
    double nextHorzTouchY = yintercept;

    while(isInsideScreen(nextHorzTouchX, nextHorzTouchY)) {
        double xToCheck = nextHorzTouchX;
        double yToCheck = nextHorzTouchY + (isFacingUp ? -1 : 0);

        if(hasWallAt(xToCheck, yToCheck)) {
            foundHorzWallHit = TRUE;
            horzWallHitX = nextHorzTouchX;
            horzWallHitY = nextHorzTouchY;
            int row = (int) floor(yToCheck / TILE_SIZE);
            int col = (int) floor(xToCheck / TILE_SIZE);
            horzWallContent = map[row][col];
            break;
        }
        else {
            nextHorzTouchX += xstep;
            nextHorzTouchY += ystep;
        }
    }

    //***********************
    //Vertical intersection
    //***********************

    int foundVertWallHit = FALSE;
    double vertWallHitX = 0;
    double vertWallHitY = 0;
    int vertWallContent = 0;

    xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
    xintercept += isFacingRight ? TILE_SIZE : 0;

    yintercept = player.y + (xintercept - player.x) * tan(angle);

    xstep = TILE_SIZE;
    xstep *= isFacingLeft ? -1 : 1;

    ystep = TILE_SIZE * tan(angle);
    ystep *= (isFacingUp && ystep > 0) ? -1 : 1;
    ystep *= (isFacingDown && ystep < 0) ? -1 : 1;

    double nextVertTouchX = xintercept;
    double nextVertTouchY = yintercept;

    while(isInsideScreen(nextVertTouchX, nextVertTouchY)) {
        double xToCheck = nextVertTouchX + (isFacingLeft ? -1 : 0);
        double yToCheck = nextVertTouchY;

        if(hasWallAt(xToCheck, yToCheck)) {
            foundVertWallHit = TRUE;
            vertWallHitX = nextVertTouchX;
            vertWallHitY = nextVertTouchY;
            int row = (int) floor(yToCheck / TILE_SIZE);
            int col = (int) floor(xToCheck / TILE_SIZE);
            vertWallContent = map[row][col];
            break;
        }
        else {
            nextVertTouchX += xstep;
            nextVertTouchY += ystep;
        }
    }

    double horzHitDistance = foundHorzWallHit ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY) : INT_MAX;
    double vertHitDistance = foundVertWallHit ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY) : INT_MAX;

    if(vertHitDistance < horzHitDistance) {
        rays[id].distance = vertHitDistance;
        rays[id].wallHitX = vertWallHitX;
        rays[id].wallHitY = vertWallHitY;
        rays[id].wallHitContent = vertWallContent;
        rays[id].wasVerticalHit = TRUE;
    }
    else {
        rays[id].distance = horzHitDistance;
        rays[id].wallHitX = horzWallHitX;
        rays[id].wallHitY = horzWallHitY;
        rays[id].wallHitContent = horzWallContent;
        rays[id].wasVerticalHit = FALSE;
    }

    rays[id].angle = angle;
    rays[id].isFacingDown = isFacingDown;
    rays[id].isFacingUp = isFacingUp;
    rays[id].isFacingLeft = isFacingLeft;
    rays[id].isFacingRight = isFacingRight;

}

void castRays() {
    double angle = player.rotationAngle - (FOV_ANGLE / 2);
    for(int id = 0; id < NUM_RAYS; id++) {
        castRay(angle, id);
        angle += FOV_ANGLE / NUM_RAYS;
    }
}

void renderPlayer() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_Rect playerRect = {
            player.x * MINIMAP_SCALE_FACTOR,
            player.y * MINIMAP_SCALE_FACTOR,
            player.width * MINIMAP_SCALE_FACTOR,
            player.height * MINIMAP_SCALE_FACTOR};
    SDL_RenderFillRect(renderer, &playerRect);

    SDL_RenderDrawLine(
            renderer,
            player.x * MINIMAP_SCALE_FACTOR,
            player.y * MINIMAP_SCALE_FACTOR,
            MINIMAP_SCALE_FACTOR * player.x + cos(player.rotationAngle) * 20,
            MINIMAP_SCALE_FACTOR * player.y + sin(player.rotationAngle) * 20);
}

void renderRays() {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for(int i=0; i < NUM_RAYS; i++) {
        SDL_RenderDrawLine(
                renderer,
                player.x * MINIMAP_SCALE_FACTOR,
                player.y * MINIMAP_SCALE_FACTOR,
                rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
                rays[i].wallHitY * MINIMAP_SCALE_FACTOR
        );
    }
}

int init() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr, "Error initializing SDL.\n");
        return FALSE;
    }
    window = SDL_CreateWindow("Wolf3D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);
    if (!window) {
        fprintf(stderr, "Error creating SDL window. \n");
        return FALSE;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return FALSE;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return TRUE;
}


void setup() {
    player.x = WINDOW_WIDTH / 2;
    player.y = WINDOW_HEIGHT / 2;
    player.width = 5;
    player.height = 5;
    player.turnDirection = 0;
    player.walkDirection = 0;
    player.rotationAngle = PI / 2;
    player.walkSpeed = 100;
    player.turnSpeed = 45 * (PI / 100);

    colorBuffer = (Uint32*) malloc(sizeof(Uint32) * (Uint32)WINDOW_WIDTH * (Uint32)WINDOW_HEIGHT);
    colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

    wallTexture = (Uint32*)malloc(sizeof(Uint32) * (Uint32)TEXTURE_WIDTH * (Uint32)TEXTURE_HEIGHT);

    pngTexture = upng_new_from_file(GRAYSTONE_TEXTURE_FILEPATH);
    if(pngTexture != NULL) {
        upng_decode(pngTexture);
        if(upng_get_error(pngTexture) == UPNG_EOK)
            wallTexture = (Uint32*) upng_get_buffer(pngTexture);
    }
}

void handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isGameRunning = FALSE;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        isGameRunning = FALSE;
                        break;
                    case SDLK_UP:
                        player.walkDirection = 1;
                        break;
                    case SDLK_DOWN:
                        player.walkDirection = -1;
                        break;
                    case SDLK_RIGHT:
                        player.turnDirection = 1;
                        break;
                    case SDLK_LEFT:
                        player.turnDirection = -1;
                        break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        player.walkDirection = 0;
                        break;
                    case SDLK_DOWN:
                        player.walkDirection = 0;
                        break;
                    case SDLK_RIGHT:
                        player.turnDirection = 0;
                        break;
                    case SDLK_LEFT:
                        player.turnDirection = 0;
                        break;
                }
                break;
            default:
                break;

        }
    }
}

void update() {
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticksLastFrame + FRAME_TIME_LENGTH));
    double deltatime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
    ticksLastFrame = SDL_GetTicks();

    updatePlayer(deltatime);
    castRays();
}

void clearColorBuffer(Uint32 color) {
    for(int x = 0; x < WINDOW_WIDTH; x++) {
        for(int y = 0; y < WINDOW_HEIGHT; y++)
            colorBuffer[(WINDOW_WIDTH * y) + x] = color;
    }
}

void renderColorBuffer() {
    SDL_UpdateTexture(colorBufferTexture, NULL, colorBuffer, (int)((Uint32)WINDOW_WIDTH * sizeof(Uint32)));
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void generate3DProjection() {
    for(int i = 0; i < NUM_RAYS; i++) {
        double perpDistance = rays[i].distance * cos(rays[i].angle - player.rotationAngle);
        double distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
        double projectedWallHeight = (TILE_SIZE / perpDistance) * distanceProjPlane;

        int wallStripHeight = (int)projectedWallHeight;

        int wallTopPixel = (WINDOW_HEIGHT / 2) - (wallStripHeight / 2);
        wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

        int wallBottomPixel = (WINDOW_HEIGHT / 2) + (wallStripHeight / 2);
        wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

        for(int y = 0; y < wallTopPixel; y++)
            colorBuffer[(WINDOW_WIDTH * y) + i] = 0xebaa81; //ABGR

        int textureOffsetX;
        if(rays[i].wasVerticalHit)
            textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
        else
            textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;

        for(int y = wallTopPixel; y < wallBottomPixel; y++) {
            int distanceFromTop = y + (wallStripHeight / 2) - (WINDOW_HEIGHT / 2);
            int textureOffsetY = distanceFromTop * ((double)TEXTURE_HEIGHT / wallStripHeight);
            Uint32 texelColor = wallTexture[(TEXTURE_WIDTH * textureOffsetY) + textureOffsetX];
            colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
        }

        for(int y = wallBottomPixel; y < WINDOW_HEIGHT; y++)
            colorBuffer[(WINDOW_WIDTH * y) + i] = 0x737373; //ABGR
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    generate3DProjection();

    renderColorBuffer();
    clearColorBuffer(0xFF000000);

    renderMap();
    renderRays();
    renderPlayer();

    SDL_RenderPresent(renderer);
}

void quit() {
    free(wallTexture);
    free(colorBuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}



int main(int argc, char *args[]) {
    isGameRunning = init();

    setup();

    while (isGameRunning) {
        handleInput();
        update();
        render();
    }
    quit();
    return EXIT_SUCCESS;
}
/*
 * OBJECTIVE  : 3D RAYTRACING
 * AUTHOR     : VICTOR DALET
 * FIRST-DATE : 11 10 2022
 * LAST-DATE  : 11 10 2022
 */


#include <stdio.h>
#include <stdlib.h>
#include "include/SDL.h"


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS_LIMIT 16

void SDL_ExitWithError(const char *message) {
    SDL_Log("ERREUR : %s > %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE); }


void SDL_LimitFps(unsigned int limit) {
    unsigned int ticks = SDL_GetTicks();
    if (limit < ticks)
        return;
    else if (limit > ticks + FPS_LIMIT)
        SDL_Delay(FPS_LIMIT);
    else
        SDL_Delay(limit-ticks);
}

int main(int argc, char** argv) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) != 0)
        SDL_ExitWithError("Initialisation SDL");

    window = SDL_CreateWindow("Première fenêtre SDL 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

    if(window == NULL)
        SDL_ExitWithError("Creation fenetre echouee");

    /*<----------------------------------------->*/


    renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);

    SDL_bool program_launch = SDL_TRUE;
    unsigned int frame_limit = 0;



    while(program_launch) {

        frame_limit = SDL_GetTicks() + FPS_LIMIT;
        SDL_LimitFps(frame_limit);
        frame_limit = SDL_GetTicks() + FPS_LIMIT;


        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    program_launch = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_b:
                            printf("Vous avez appuyer sur B.\n");
                        default:
                            continue;
                    }
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_b:
                            printf("Vous avez relacher la touche B.\n");
                        default:
                            continue;
                    }

                case SDL_MOUSEMOTION:
                    printf("%d , %d \n ",event.motion.x,event.motion.y);

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.clicks >= 2)
                        printf("double cliques \n");
                    if (event.button.button == SDL_BUTTON_RIGHT)
                        printf("clique droit \n");

                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_LEAVE)
                        printf("La souris est sortie de la fenetre \n");

                default:
                    break;
            }
        }
    }

    if (renderer == NULL)
        SDL_ExitWithError("Creation rendu enchouee");

    if (SDL_SetRenderDrawColor(renderer,112,168,237,SDL_ALPHA_OPAQUE)!=0)
        SDL_ExitWithError("Impossible de changer la couleur du rendu");

    if (SDL_RenderDrawPoint(renderer,100,400)!=0)
        SDL_ExitWithError("Impossible de dessiner un point");

    if (SDL_RenderDrawLine(renderer,50,50,500,500)!=0)
        SDL_ExitWithError("Impossible de dessinner une ligne");

    SDL_Rect rectangle;
    rectangle.x  = 300;
    rectangle.y = 300;
    rectangle.w = 200;
    rectangle.h = 120;

    if (SDL_RenderDrawRect(renderer,&rectangle)!=0)
        SDL_ExitWithError("Impossible de dessiner un rectangle");

    if (SDL_RenderFillRect(renderer,&rectangle)!=0)
        SDL_ExitWithError("Impossible de changer la couleur du rendu");

    SDL_Surface *image = NULL;
    SDL_Texture  *texture = NULL;

    image = SDL_LoadBMP("../assets/img/planette.bmp");

    if (image == NULL) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible de charger l'image"); }

    texture = SDL_CreateTextureFromSurface(renderer,image);
    SDL_FreeSurface(image);

    if (texture == NULL) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible de creer la texture"); }

    if (SDL_QueryTexture(texture,NULL,NULL,&rectangle.w,&rectangle.h)!=0) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible de creer la texture"); }

    rectangle.x = (WINDOW_WIDTH - rectangle.w) /2;
    rectangle.y = (WINDOW_HEIGHT - rectangle.h) /2;

    if (SDL_RenderCopy(renderer,texture,NULL,&rectangle) != 0) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_ExitWithError("Impossible d'afficher la texture");
    }


    SDL_RenderPresent(renderer);
    /*<----------------------------------------->*/
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS; }




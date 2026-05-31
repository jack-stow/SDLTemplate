
#pragma once

typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Window SDL_Window;

typedef struct {
    SDL_Renderer *renderer;
    SDL_Window *window;
    int up;
    int down;
    int left;
    int right;
} App;
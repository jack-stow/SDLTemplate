#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "types.h"

typedef struct SDL_Texture SDL_Texture;

typedef struct {
    uint8_t r, g, b, a;
} Color;

void prepareScene(void);
void presentScene(void);
void blit(SDL_Texture* texture, real x, real y, real angle, real scale);
SDL_Texture* loadTexture(char* filename);
void draw_circle(real x, real y, real radius, Color color, bool filled);
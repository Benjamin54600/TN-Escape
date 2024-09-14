#ifndef FONT_H
#define FONT_H

#include "globals.h"
#include <stdbool.h>

void loadImages(void);

Map loadMap(const char *filename, int nbsMaxIndices);

void drawMap(const Map map);

void initFonts(void);

TTF_Font *loadFont(const char *file, int ptsize);

void renderText(const char* message, TTF_Font* font, SDL_Color color, int x, int y);

void cleanupFonts(void);

void clearMap(Map map);

#endif

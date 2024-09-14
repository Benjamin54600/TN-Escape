#ifndef MONSTER_H
#define MONSTER_H
#include "globals.h"
#include "font.h"
#include <SDL2/SDL.h>

void initMonsters(Character* monsters[], const char* imagePath, const char* mapFilePath);

Character* loadMonster(const char* imagePath, int x, int y);

void clearMonster(int i);

void clearAllMonsters(void);

void drawMonsters(Map* monster_map);

void deplaceMonster(Map *map, Map *monster_map, int delaiChgmtDir);

bool blob_is_touching_chara(Character monster);

#endif

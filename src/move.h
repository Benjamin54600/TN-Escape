#ifndef MOVE_H
#define MOVE_H

#include "globals.h"
#include "monster.h"

void deplace(Map *map, Mix_Chunk* cri);

//Fonction pour charger un personnage
Character loadCharacter(const char* imagePath, Map* map);

void init_player(Map *map, Character *character);

void clearCharacter(void);

bool is_touching_obstacle(int newPosX, int newPosY, Map *map);

#endif

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "globals.h"
#include <stdbool.h>
#define PROJECTILE_SPEED DISTANCE_DEPLACEMENT
#define PROJECTILE_CADENCE 500 // Nombre de ms entre chaque tir (voir fonction tirRandom)

typedef struct Projectile {
    SDL_Rect position;
    float xvitesse; 
    float yvitesse; 
    bool friend; // 1 si le projectile est tiré par le joueur, 0 si c'est par un monstre
    bool active; // Projectile désactivé s'il rencontre un mur
    struct Projectile *next; // Pointeur vers le prochain projectile
} Projectile;

Projectile* createProjectile(int x, int y, float xvitesse, float yvitesse, bool friend);

void moveProjectiles(Projectile **projectileList, Map *map, Map *monster_map, Mix_Chunk *mobDeathMusic);

void renderProjectiles(Projectile *projectileList,bool friend);

void addProjectile(Projectile **projectileList, Mix_Chunk *fireMusic, Character monster);

void tirPerso(Projectile** projectileList, Mix_Chunk* fireChunk);

void clearProjectiles(Projectile **projectileList);

bool isMur(int Posx, int Posy, Map *map);

bool tirRandom(void);

bool fire_is_touching_chara(Projectile projectile);

bool fire_is_touching_monster(Projectile projectile, Character monster);

#endif

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "projectile.h"
#include "globals.h"
#include "monster.h"
#include "font.h"

Projectile* createProjectile(int x, int y, float xvitesse, float yvitesse, bool friend) {
    Projectile *projectile = (Projectile *)malloc(sizeof(Projectile));
    projectile->position.x = x;
    projectile->position.y = y;
    projectile->position.w = 20; // Largeur du projectile
    projectile->position.h = 20; // Hauteur du projectile
    projectile->xvitesse = xvitesse;
    projectile->yvitesse = yvitesse;
    projectile->friend = friend;
    projectile->active = 1;
    projectile->next = NULL;
    return projectile;
}

void addProjectile(Projectile** projectileList, Mix_Chunk* fireChunk, Character tireur) { //Avec calcul de la direction vers le joueur
    
    int x = tireur.pos_map->x + tireur.pos_map->w / 2;
    int y = tireur.pos_map->y + tireur.pos_map->h / 2;
    int target_x = character.pos_map->x + character.pos_map->w / 2;
    int target_y = character.pos_map->y + character.pos_map->h / 2;

    float dx = target_x - x;
    float dy = target_y - y;
    float distance = sqrt(dx * dx + dy * dy);
    float xvitesse = (dx / distance) * PROJECTILE_SPEED;
    float yvitesse = (dy / distance) * PROJECTILE_SPEED;
    Projectile *newProjectile = createProjectile(x, y, xvitesse, yvitesse, 0);
    newProjectile->next = *projectileList;
    *projectileList = newProjectile;
    Mix_PlayChannel(-1, fireChunk, 0);
}

void tirPerso(Projectile** projectileList, Mix_Chunk* fireChunk) {
    
    int x = character.pos_map->x + character.pos_map->w / 2;
    int y = character.pos_map->y + character.pos_map->h / 2;
    float xvitesse=0;
    float yvitesse=0;
    if (character.orientation == 0) {
        xvitesse = 3;
        yvitesse = 0;
    } else if (character.orientation == 1) {
        xvitesse = -3;
        yvitesse = 0;
    } else if (character.orientation == 2) {
        xvitesse = 0;
        yvitesse = 3;
    } else if (character.orientation == 3) {
        xvitesse = 0;
        yvitesse = -3;
    }

    Projectile *newProjectile = createProjectile(x, y, xvitesse, yvitesse, 1);
    newProjectile->next = *projectileList;
    *projectileList = newProjectile;

    Mix_PlayChannel(-1, fireChunk, 0);
}

void moveProjectiles(Projectile **projectileList, Map *map, Map *monster_map, Mix_Chunk *mobDeathMusic) {
    
    Projectile *current = *projectileList;
    Projectile *previous = NULL;

    while (current != NULL) {
        if (!current->active) {
            // Si le projectile est inactif, le retirer de la liste
            if (previous == NULL) {
                *projectileList = current->next;
                free(current);
                current = *projectileList;
            } else {
                previous->next = current->next;
                free(current);
                current = previous->next;
            }
        } else {
            // Actualisation de la position du projectile
            current->position.x += current->xvitesse;
            current->position.y += current->yvitesse;

            // Collision avec un mur
            if (isMur(current->position.x, current->position.y, map)) {
                current->active = 0; // Désactiver le projectile en vue de sa suppression
            }

            // Collision avec joueur
            if (fire_is_touching_chara(*current)){
                death = 1;
            }

            // Collision avec monstres
            for(int i = 0 ; i < monster_map->numMonster; i++){      // Jsp pk mais on est obligé d'avoir monster_map passée en pointeur pour accéder à la bonne valeur de numMonster
            
                if (monsters[i] != NULL){

                    if (fire_is_touching_monster(*current, *monsters[i])){
                        Mix_PlayChannel(-1, mobDeathMusic, 0);
                        clearMonster(i);
                        current->active = 0;   // On désactive la balle quand on tue le blob

                        // Eventuellement rajouter le passage du sprite au rouge
                    }
                }
            }

            previous = current;
            current = current->next;
        }
    }
}

void renderProjectiles(Projectile *projectileList,bool friend) {
    Projectile *current = projectileList;
    while (current != NULL) {
        if (current->active) {
            if (friend==0){
            SDL_Texture *projectileTexture = IMG_LoadTexture(renderer, "res/images/feu.png"); // On va chercher le sprite
            if (projectileTexture == NULL) {
                fprintf(stderr, "Erreur de chargement de la texture du projectile : %s\n", SDL_GetError());
                exit(EXIT_FAILURE);
            }
            SDL_RenderCopy(renderer, projectileTexture, NULL, &current->position);
            SDL_DestroyTexture(projectileTexture);
            }
            if (friend){
            SDL_Texture *projectileTexture = IMG_LoadTexture(renderer, "res/images/Munition.png"); // On va chercher le sprite
            if (projectileTexture == NULL) {
                fprintf(stderr, "Erreur de chargement de la texture du projectile : %s\n", SDL_GetError());
                exit(EXIT_FAILURE);
            }
            SDL_RenderCopy(renderer, projectileTexture, NULL, &current->position);
            SDL_DestroyTexture(projectileTexture);
            }
            
            
        }
        current = current->next;
    }
}

void clearProjectiles(Projectile **projectileList) {
    Projectile *current = *projectileList;
    while (current != NULL) {
        Projectile *next = current->next;
        free(current);
        current = next;
    }
    *projectileList = NULL; // Réinitialiser la liste des projectiles (utile en cas de nouvelle partie ou nouvelle map)
}

bool isMur(int Posx, int Posy, Map *map) { // Inspirée de is_touching_obstacle
    if (Posx < 0 || Posx >= map->width * TILE_SIZE || Posy < 0 || Posy >= map->height * TILE_SIZE) {
        return true;
    }
    int tileX = Posx / TILE_SIZE;
    int tileY = Posy / TILE_SIZE;
    return (map->tiles[tileY][tileX] == 2);
}

bool tirRandom(void) {  // Fonction pour la génération aléatoire de tirs

    int randomValue = rand() % PROJECTILE_CADENCE;
    return randomValue == 0;
}

bool fire_is_touching_chara(Projectile projectile){

    if ( !projectile.friend){
        
    
        int charaLeft = character.pos_map->x+15;
        int charaRight = character.pos_map->x + character.pos_map->w-15;
        int charaTop = character.pos_map->y+15;
        int charaBottom = character.pos_map->y + character.pos_map->h-15;

        int projLeft = projectile.position.x;
        int projRight = projectile.position.x + projectile.position.w;
        int projTop = projectile.position.y;
        int projBottom = projectile.position.y + projectile.position.h;

        bool chevauchementX = (projLeft < charaRight) && (projRight > charaLeft);
        bool chevauchementY = (projTop < charaBottom) && (projBottom > charaTop);

        return chevauchementX && chevauchementY;
    }
    else {
        return false;
    }
    
}

bool fire_is_touching_monster(Projectile projectile, Character monster){

    if (projectile.friend){
        
    
        int blobLeft = monster.pos_map->x+15;
        int blobRight = monster.pos_map->x + monster.pos_map->w-15;
        int blobTop = monster.pos_map->y+15;
        int blobBottom = monster.pos_map->y + monster.pos_map->h-15;

        int projLeft = projectile.position.x;
        int projRight = projectile.position.x + projectile.position.w;
        int projTop = projectile.position.y;
        int projBottom = projectile.position.y + projectile.position.h;

        bool chevauchementX = (projLeft < blobRight) && (projRight > blobLeft);
        bool chevauchementY = (projTop < blobBottom) && (projBottom > blobTop);

        return chevauchementX && chevauchementY;
    }
    else {
        return false;
    }
    
}

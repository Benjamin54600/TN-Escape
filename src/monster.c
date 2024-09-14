#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>
#include <time.h>
#include "move.h"
#include "globals.h"
#include "monster.h"
#include "move.h"

extern int mapNum;

void initMonsters(Character* monsters[], const char* imagePath, const char* mapFilePath) {
    FILE* file = fopen(mapFilePath, "r");
    if (!file) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier de carte : %s\n", mapFilePath);
        exit(1);
    }

    int monsterCount = 0;
    char line[2*MAP_WIDTH + 1]; // *2 pour les expaces et newline, +1 pour null terminator
    for (int y = 0; fgets(line, sizeof(line), file) != NULL; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (line[2*x] != '0' && y!=0) {   // Condition sur y pour ne pas tenir compte de la première ligne (dimensions de la map) et 2*x pour tenir compte des espaces
                monsters[monsterCount] = loadMonster(imagePath, x * TILE_SIZE, (y-1) * TILE_SIZE);    // y-1 pour négliger la première ligne des coordonnées
                monsterCount++;
            }
        }
    }

    fclose(file);
}

Character* loadMonster(const char* imagePath, int x, int y) {

    Character* monster = (Character*)malloc(sizeof(Character));
    assert(monster != NULL);

    monster->sprite = IMG_LoadTexture(renderer, imagePath);
    if (!monster->sprite) {
        fprintf(stderr, "Erreur lors du chargement de l'image : %s\n", IMG_GetError());
        exit(1);
    }

    monster->allure = malloc(sizeof(SDL_Rect));
    monster->allure->x = 0;
    monster->allure->y = 0;
    monster->allure->w = TILE_SIZE;
    monster->allure->h = TILE_SIZE;

    monster->pos_map = malloc(sizeof(SDL_Rect));
    monster->pos_map->x = x;
    monster->pos_map->y = y;
    monster->pos_map->w = TILE_SIZE;
    monster->pos_map->h = TILE_SIZE;

    monster->orientation = rand() % 4;   // Orientation initiale des monstres aléatoire
    monster->offset = 0;

    return monster;
}

void clearMonster(int i){
        

        if (monsters[i] != NULL){
            
            SDL_DestroyTexture(monsters[i]->sprite);
            monsters[i]->sprite = NULL;

            free(monsters[i]->allure);
            monsters[i]->allure = NULL;

            free(monsters[i]->pos_map);
            monsters[i]->pos_map = NULL;

            free(monsters[i]);
            monsters[i] = NULL;

        }
}

void clearAllMonsters(void){
    
    for (int i = 0 ; i < MAX_MONSTERS ; i++)
    {
        clearMonster(i);
    }
}

void drawMonsters(Map* monster_map) {
    
    // Convertir la pos_map en pixels du personnage en coordonnées de tuiles
    int char_tile_x = character.pos_map->x / TILE_SIZE;
    int char_tile_y = character.pos_map->y / TILE_SIZE;

    for (int i = 0; i < monster_map->numMonster ; i++) {
        
        if (monsters[i] != NULL){

            Character* monster = monsters[i];

            // Convertir la pos_map en pixels du monstre en coordonnées de tuiles
            int monster_tile_x = monster->pos_map->x / TILE_SIZE;
            int monster_tile_y = monster->pos_map->y / TILE_SIZE;

            // Calculer la distance entre le personnage et le monstre
            int dx = monster_tile_x - char_tile_x;
            int dy = monster_tile_y - char_tile_y;
            float distance_squared = dx * dx + dy * dy;

            // Si le monstre est dans le rayon d'éclairage, le dessiner
            if (distance_squared <= (LIGHT_RADIUS+mapNum)*(LIGHT_RADIUS+mapNum)) {
                SDL_RenderCopy(renderer, monster->sprite, monster->allure, monster->pos_map);
            }
        }
    }
}

void deplaceMonster(Map *map, Map *monster_map, int delaiChgmtDir){

    for (int i = 0; i < monster_map->numMonster ;i++) {

        if (monsters[i] != NULL){
            Character* monster = monsters[i];
        
            int newPosX = monster->pos_map->x;
            int newPosY = monster->pos_map->y;
            int newOrientation = monster->orientation;
            int newOffset = monster->offset;

            if (delaiChgmtDir % 5 == 0){
                newOffset = (newOffset + 1) % 2;
            }

            int tileX = newPosX / TILE_SIZE;
            int tileY = newPosY / TILE_SIZE;
            int newTileX;
            int newTileY;

            if (delaiChgmtDir == 30) {
                newOrientation = (rand() % 4); // Nombre aléatoire entre 0 et 3
            }
            
            do {
                if (is_touching_obstacle(newPosX, newPosY, map)) {
                    newOrientation = (rand() % 4); // Nombre aléatoire entre 0 et 3
                }
                
                switch (newOrientation)
                {
                case 0: // Droite
                    newPosX += DISTANCE_DEPLACEMENT;
                    break;
                case 1: // Gauche
                    newPosX -= DISTANCE_DEPLACEMENT;
                    newOrientation = 1;
                    break;
                case 2: // Bas
                    newPosY += DISTANCE_DEPLACEMENT;
                    break;
                case 3: // Haut
                    newPosY -= DISTANCE_DEPLACEMENT;
                    break;
                default:
                printf("Erreur lors de l'initialisation de newOriention\n");
                    break;
                }

            } while(is_touching_obstacle(newPosX, newPosY, map));

            // Vérification des limites de la carte parce que ça m'a bien pris la tête
            if (newPosX < 0 || newPosX >= SCREEN_WIDTH - monster->pos_map->w || newPosY < 0 || newPosY >= SCREEN_HEIGHT - monster->pos_map->h) {
                printf("Tentative de déplacement hors des limites : newPosX = %d, newPosY = %d\n", newPosX,newPosY);
                return;
            }

            newTileX = newPosX / TILE_SIZE;
            newTileY = newPosY / TILE_SIZE;

            if (!is_touching_obstacle(newPosX, newPosY, map)) {

                monster->pos_map->x = newPosX;
                monster->pos_map->y = newPosY;
                monster->orientation = newOrientation;

                if (newOffset != monster->offset) {

                    monster->offset += 1;
                    monster->offset %= 2;

                    if (newOrientation != 3){
                        newOrientation = (newOrientation + 1) % 3 ;     // Changement de l'orientation pour être adapté à l'ordre des directions du sprite des blobs (pas le même que pour Rambo)
                    }

                    monster->allure->x = monster->offset * monster->allure->h;
                    monster->allure->y = newOrientation * monster->allure->w;
                }
            }

            monster_map->tiles[tileY][tileX] = 0;
            monster_map->tiles[newTileY][newTileX] = 1;
            
            if(blob_is_touching_chara(*monster)){
                death = 1;
            }
        }
    }
}

bool blob_is_touching_chara(Character monster){

    int charaLeft = character.pos_map->x+25;
    int charaRight = character.pos_map->x + character.pos_map->w-25;
    int charaTop = character.pos_map->y+25;
    int charaBottom = character.pos_map->y + character.pos_map->h-25;

    int blobLeft = monster.pos_map->x;
    int blobRight = monster.pos_map->x + monster.pos_map->w;
    int blobTop = monster.pos_map->y;
    int blobBottom = monster.pos_map->y + monster.pos_map->h;

    bool chevauchementX = (blobLeft < charaRight) && (blobRight > charaLeft);
    bool chevauchementY = (blobTop < charaBottom) && (blobBottom > charaTop);

    return chevauchementX && chevauchementY;
}

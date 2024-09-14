#include "globals.h"
#include "move.h"
#include "font.h"

void deplace(Map *map, Mix_Chunk* cri) {

    int newPosX = character.pos_map->x;
    int newPosY = character.pos_map->y;
    int newOrientation = character.orientation;
    int newOffset = character.offset;
    int attaque = 0;

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    
    if (state[SDL_SCANCODE_SPACE]) {    // Attaque [ESPACE]
        attaque = 1;
    } else if (state[SDL_SCANCODE_RIGHT]) {
        newPosX += DISTANCE_DEPLACEMENT;
        newOrientation = 0;
        newOffset = (newOffset + 1) % 2;
    } else if (state[SDL_SCANCODE_LEFT]) {
        newPosX -= DISTANCE_DEPLACEMENT;
        newOrientation = 1;
        newOffset = (newOffset + 1) % 2;
    } else if (state[SDL_SCANCODE_DOWN]) {
        newPosY += DISTANCE_DEPLACEMENT;
        newOrientation = 2;
        newOffset = (newOffset + 1) % 2;
    } else if (state[SDL_SCANCODE_UP]) {
        newPosY -= DISTANCE_DEPLACEMENT;
        newOrientation = 3;
        newOffset = (newOffset + 1) % 2;
    } 
    
    else{
        character.allure->x = 0;    // On met le perso au repos sans changer l'orientation si aucune touche enfoncée
    }

    // Vérification des limites de la carte parce que ça m'a bien pris la tête
    if (newPosX < 0 || newPosX >= SCREEN_WIDTH - character.pos_map->w || newPosY < 0 || newPosY >= SCREEN_HEIGHT - character.pos_map->h) {
        printf("Tentative de déplacement hors des limites : newPosX = %d, newPosY = %d\n", newPosX, newPosY);
        return;
    }

    if (attaque){
        character.allure->x = 2 * character.allure->h;
        Mix_PlayChannel(-1, cri, 0);
    }

    if (!is_touching_obstacle(newPosX, newPosY, map)) {
        character.pos_map->x = newPosX;
        character.pos_map->y = newPosY;
        character.orientation = newOrientation;
        if (newOffset != character.offset) {
            character.offset += 1;
            character.offset %= 2;
            character.allure->y = (character.orientation) * character.allure->w;
            character.allure->x = (character.offset) * character.allure->h;
        }
    }
}

Character loadCharacter(const char* imagePath, Map* map) {
    Character character;
    SDL_Texture* spritetexture = IMG_LoadTexture(renderer, imagePath);

    if (!spritetexture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error in sprite texture init : %s", IMG_GetError());
        exit(-1);
    }
    
    character.sprite = spritetexture;

    int SpriteFullWidth, SpriteFullHeight;
    if (SDL_QueryTexture(character.sprite, NULL, NULL, &SpriteFullWidth, &SpriteFullHeight)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error in query texture : %s", SDL_GetError());
        exit(-1);
    }
    
    int spriteWidth = SpriteFullWidth / 3;
    int spriteHeight = SpriteFullHeight / 4;

    character.allure = malloc(sizeof(SDL_Rect));
    character.allure->x = 0; // Position en x du sprite sur l'image .png
    character.allure->y = 0;  // Position en y du sprite sur l'image .png
    character.allure->w = spriteWidth; // Largeur du sprite
    character.allure->h = spriteHeight; // Hauteur du sprite

    character.orientation = 0; // Orientation initiale
    character.offset = 0; // Image initiale

    // Positionner le personage au milieu de la carte/
    character.pos_map = malloc(sizeof(SDL_Rect));
    assert(character.pos_map != NULL);
    init_player(map, &character);

    // Donner les bonnes dimmensions à pos_map pour que la taille du rendu soit correcte :
    character.pos_map->w = character.allure->w;
    character.pos_map->h = character.allure->h;
    
    return character;
}

void init_player(Map *map, Character *character) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (map->tiles[i][j] == 3) {
                character->pos_map->x = j * TILE_SIZE;
                character->pos_map->y = i * TILE_SIZE;
                return;
            }
        }
    }
}

void clearCharacter(void){
    
    SDL_DestroyTexture(character.sprite);
    character.sprite = NULL;

    free(character.allure);
    character.allure = NULL;

    free(character.pos_map);
    character.pos_map = NULL;
}

// Vérifier si la tuile est un obstacle
bool is_touching_obstacle(int newPosX, int newPosY, Map *map) {

    // Rappel : 2 représente un obstacle

    // Vérification en haut à gauche
    int hautGX = (newPosX + 15) / TILE_SIZE;
    int hautGY = (newPosY + 25) / TILE_SIZE;

    // Vérifier les limites de la carte pour éviter les accès hors limites
    if (hautGX < 0 || hautGX >= map->width || hautGY < 0 || hautGY >= map->height) {
        return true;
    }
    if (map->tiles[hautGY][hautGX] == 2 || map->tiles[hautGY][hautGX] == 5){
        return true;
    }

    // Vérification en haut à droite
    int hautDX = (newPosX + (character.pos_map->w) - 15) / TILE_SIZE;
    int hautDY = (newPosY + 25) / TILE_SIZE;

    // Vérifier les limites de la carte pour éviter les accès hors limites
    if (hautDX < 0 || hautDX >= map->width || hautDY < 0 || hautDY >= map->height) {
        return true;
    }
    if (map->tiles[hautDY][hautDX] == 2 || map->tiles[hautDY][hautDX] == 5) {
        return true;
    }

    // Vérifier la collision en bas à droite du sprite
    int basDX = (newPosX + (character.pos_map->w) - 15) / TILE_SIZE;
    int basDY = (newPosY + (character.pos_map->h) - 7) / TILE_SIZE;

    // Vérifier les limites de la carte pour éviter les accès hors limites
    if (basDX < 0 || basDX >= map->width || basDY < 0 || basDY >= map->height) {
        return true;
    }
    if (map->tiles[basDY][basDX] == 2 || map->tiles[basDY][basDX] == 5) {
        return true;
    }

    // Vérifier la collision en bas à gauche du sprite
    int basGX = (newPosX + 15) / TILE_SIZE;
    int basGY = (newPosY + (character.pos_map->h) - 7) / TILE_SIZE;

    // Vérifier les limites de la carte pour éviter les accès hors limites
    if (basGX < 0 || basGX >= map->width || basGY < 0 || basGY >= map->height) {
        return true;
    }
    if (map->tiles[basGY][basGX] == 2 || map->tiles[basGY][basGX] == 5) {
        return true;
    }

    return false;
}

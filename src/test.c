#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "globals.h"
#include "move.h"
#include "font.h"
#include "monster.h"
#include "projectile.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *textures[NUM_IMAGES];

Map map;
Map monster_map;
int mapNum = 1;

Character character;
Character* monsters[MAX_MONSTERS];
Character* monster;

int inMenu = 1;
int running = 0;
int pause = 0;
int death = 0;

void test_loadImages(void) {
    loadImages();
    for (int i = 0; i < NUM_IMAGES; i++) {
        assert(textures[i] != NULL);
        printf("Texture %d loaded\n", i);
    }
}

void test_LoadMap(void) {
    assert(map.width == MAP_WIDTH);
    assert(map.height == MAP_HEIGHT);
    for (int i = 0; i < map.height; i++) {
        for (int j = 0; j < map.width; j++) {
            assert(map.tiles[i][j] >= 0 && map.tiles[i][j] <= NUM_IMAGES);
        }
    }
    printf("Map loaded\n");
}

void test_loadCharacter(void) {

    // Vérification que la texture a été correctement créée
    assert(character.sprite != NULL);

    // Vérification des dimensions de la texture et des rectangles
    int SpriteFullWidth, SpriteFullHeight;
    SDL_QueryTexture(character.sprite, NULL, NULL, &SpriteFullWidth, &SpriteFullHeight);

    int spriteWidth = SpriteFullWidth / 3;
    int spriteHeight = SpriteFullHeight / 4;

    assert(character.allure->w == spriteWidth);
    assert(character.allure->h == spriteHeight);
    assert(character.allure->x == 0);
    assert(character.allure->y == 0);

    // Vérification de la position initiale du personnage
    assert(character.pos_map->x == TILE_SIZE);    // Nouvelle position initiale (map1)
    assert(character.pos_map->y == 16*TILE_SIZE);   // Nouveau y initial (map1) normalement mais ça marche pas donc jsp
    assert(character.pos_map->w == character.allure->w);
    assert(character.pos_map->h == character.allure->h);

    printf("loadCharacter tested\n");
}

void test_loadMonster(void) {

    assert(monster->sprite != NULL);
    assert(monster->allure != NULL);
    assert(monster->pos_map != NULL);
    assert(monster->pos_map->x == 100);
    assert(monster->pos_map->y == 100);
    assert(monster->pos_map->w == TILE_SIZE);
    assert(monster->pos_map->h == TILE_SIZE);

    printf("loadMonster tested \n");
}

void test_is_touching_obstacle(void) {
    // Test cases
    character.pos_map->x = 0;
    character.pos_map->y = 0;
    assert(is_touching_obstacle(0, 0, &map) == true);  // Top-left corner, which is an obstacle

    character.pos_map->x = TILE_SIZE;
    character.pos_map->y = TILE_SIZE;
    assert(is_touching_obstacle(TILE_SIZE, TILE_SIZE, &map) == false);  // A tile inside the map, not an obstacle

    character.pos_map->x = TILE_SIZE * 2;
    character.pos_map->y = TILE_SIZE * 2;
    assert(is_touching_obstacle(TILE_SIZE, TILE_SIZE * 2, &map) == true);  // Another position, which is an obstacle

    character.pos_map->x = TILE_SIZE;
    character.pos_map->y = TILE_SIZE * 3;
    assert(is_touching_obstacle(TILE_SIZE, TILE_SIZE * 3, &map) == false);  // Another position, which is not an obstacle

    printf("is_touching_obstacle tested\n");
}

void test_projectile(void){
    Projectile* projectiles = createProjectile(0,0,0,0,1);
    addProjectile(&projectiles,NULL,*monster);
    assert(projectiles!=NULL);
    clearProjectiles(&projectiles);
    assert(projectiles==NULL);
    printf("projectile tested \n");
}


int main(void) {
    // Initialisation de SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    IMG_Init(IMG_INIT_PNG);

    // Initialiser la fenêtre et le renderer
    window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Initialisation de la map, du personnage et du monstre
    map = loadMap("res/fonts/map_1.txt", NUM_IMAGES);
    monster_map = loadMap("res/fonts/blob_1.txt", MAX_MONSTERS);
    character = loadCharacter("res/images/Rambo.png", &map);
    monster = loadMonster("res/images/Blob.png", 100, 100);

    test_loadImages();
    test_LoadMap();
    test_loadMonster();
    test_loadCharacter();
    test_is_touching_obstacle();
    test_projectile();

    printf("Tests réussis !\n");

    // Libération des ressources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    for (int i = 0; i < map.height; i++) {  // Libérer la mémoire allouée pour les tuiles de la carte
        free(map.tiles[i]);
    }
    
    free(map.tiles);                        // Libérer la mémoire allouée pour le tableau de pointeurs de lignes de tuiles

    SDL_DestroyTexture(character.sprite);
    free(character.allure);
    free(character.pos_map);

    IMG_Quit();
    SDL_Quit();

    return 0;
}

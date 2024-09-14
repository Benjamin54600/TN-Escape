#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define SCREEN_WIDTH 1800 // Largeur de l'écran
#define SCREEN_HEIGHT 900 // Hauteur de l'écran
#define TILE_SIZE 50 // Taille d'une tuile de la carte
#define MAP_WIDTH 36 // Nombre de tuiles en largeur
#define MAP_HEIGHT 18 // Nombre de tuiles en hauteur
#define NUM_IMAGES 5 // Nombre d'images à charger
#define DISTANCE_DEPLACEMENT 2 // C'est nécessairement un entier (mis à 2 pour l'instant pour les tests)
#define LIGHT_RADIUS 1
#define CHRONO 600 // Temps total en secondes
#define MAX_MONSTERS 5
#define CADENCE_DEPLACEMENT 1 // Les monstres ne se déplacent que tous les CADENCE_DEPLACEMENT tours de boucle

typedef struct {
    int width;
    int height;
    int numMonster;
    int **tiles; // Tableau 2D pour stocker les caractères de la carte
} Map;

// Structure pour représenter un personnage
typedef struct _character{
    SDL_Texture* sprite;
    SDL_Rect* allure; // Allure du personnage
    SDL_Rect* pos_map; // Position sur la carte
    int orientation; // 0 = droite, 1 = gauche, 2 = bas, 3 = haut
    int offset; // Désigne l'image que l'on pointe sur le sprite
} Character;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *textures[NUM_IMAGES];

extern Character character;
extern Character* monsters[MAX_MONSTERS];

extern Map map;
extern Map monster_map;
extern int mapNum;

extern int inMenu;
extern int running;
extern int pause;
extern int death;

#endif

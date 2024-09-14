#include <stdbool.h>
#include "globals.h"
#include "font.h"

// Fonction pour charger les images de tuiles
void loadImages(void) {
    for (int i = 0; i < NUM_IMAGES; i++) {
        char filename[40]; // La taille de la chaîne doit être suffisante
        sprintf(filename, "res/images/image%d.png", i + 1);
        textures[i] = IMG_LoadTexture(renderer, filename);
        if (textures[i] == NULL) {
            fprintf(stderr, "Failed to load texture %s: %s\n", filename, IMG_GetError());
        }
    }
}

// Fonction pour charger la carte depuis un fichier texte
Map loadMap(const char *filename, int nbsMaxIndices) {

    Map map;
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s\n", filename);
        exit(1);
    }

    fscanf(file, "%d %d %d", &map.width, &map.height, &map.numMonster); // Lecture des dimensions de la carte et du nombre de monstres depuis le fichier
    if (map.width != MAP_WIDTH || map.height != MAP_HEIGHT) {
        fprintf(stderr, "Erreur : Dimensions de carte invalides dans le fichier %s\n", filename);
        exit(1);
    }

    map.tiles = (int **)malloc(MAP_HEIGHT * sizeof(int *));
    assert(map.tiles != NULL);

    for(int y = 0; y < MAP_HEIGHT; y++) {

        map.tiles[y] = (int *)malloc(MAP_WIDTH * sizeof(int));
        assert(map.tiles[y] != NULL);

        for (int x = 0; x < MAP_WIDTH; x++) {

            fscanf(file, "%d", &map.tiles[y][x]); // Lecture de l'indice de l'image
            if (map.tiles[y][x] < 0 || map.tiles[y][x] > nbsMaxIndices) {
                fprintf(stderr, "Erreur : Indice d'image invalide dans le fichier %s\n", filename);
                exit(1);
            }
        }
    }

    fclose(file);

    return map;
}

// Fonction pour dessiner la carte sur l'écran SDL // J'ai ajouté le 3ème argument pour récupérer la pos du personnage et définir le filtre de vision
void drawMap(const Map map) {
    SDL_Rect destRect;
    destRect.w = TILE_SIZE;
    destRect.h = TILE_SIZE;

    // Convertir la position en pixels du personnage en coordonnées de tuiles
    int char_tile_x = character.pos_map->x / TILE_SIZE;
    int char_tile_y = character.pos_map->y / TILE_SIZE;

    for (int y = 0; y < map.height; y++) {
        for (int x = 0; x < map.width; x++) {
            destRect.x = x * TILE_SIZE;
            destRect.y = y * TILE_SIZE;

            // Calculer la distance depuis le personnage
            int dx = x - char_tile_x;
            int dy = y - char_tile_y;
            float distance_squared = dx * dx + dy * dy; // Je prends le carré car ça évite de passer par sqrt

            // Dessiner la tuile normalement
            SDL_RenderCopy(renderer, textures[map.tiles[y][x]-1], NULL, &destRect);

            // Si la tuile est hors de la portée de la lumière, assombrir la tuile
            if (distance_squared > (LIGHT_RADIUS+mapNum)*(LIGHT_RADIUS+mapNum)) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
                SDL_RenderFillRect(renderer, &destRect);
            }
        }
    }
}

// Initialisation de police - sert uniquement pour le chrono pour l'instant
void initFonts(void)
{
    if (TTF_Init() == -1)
    {
        fprintf(stderr, "Unable to initialize TTF: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

// Chargement de police - sert uniquement pour le chrono pour l'instant
TTF_Font *loadFont(const char *file, int ptsize)
{
    TTF_Font *font = TTF_OpenFont(file, ptsize);
    if (font == NULL)
    {
        fprintf(stderr, "Unable to load font %s: %s\n", file, TTF_GetError());
        exit(EXIT_FAILURE);
    }
    return font;
}

// Pour l'affichage du compte à rebours sur l'écran de jeu
void renderText(const char* message, TTF_Font* font, SDL_Color color, int x, int y)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, message, color);
    if (!surface)
    {
        fprintf(stderr, "Unable to create surface: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        fprintf(stderr, "Unable to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    SDL_Rect dstrect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Nettoyage des polices
void cleanupFonts(void)
{
    TTF_Quit();
}

void clearMap(Map map){
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        free(map.tiles[y]);
        map.tiles[y] = NULL;
    }
    free(map.tiles);
    map.tiles = NULL;
}

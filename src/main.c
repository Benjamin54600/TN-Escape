#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>

#include "globals.h"
#include "move.h"
#include "font.h"
#include "monster.h"
#include "projectile.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *textures[NUM_IMAGES];

Character character;
Character* monsters[MAX_MONSTERS];

Map map;
Map monster_map;
int mapNum = 1;

int inMenu = 1; // 4=defaite mort 5=defaite temps 6=victoire
int running = 0;
int pause = 0;
int death = 0;
//0=ça joue, 1=mort, 2=plus de temps, 3=victoire

int main(void) {

    // Init de SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error in init: %s", SDL_GetError());
        exit(-1);
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Erreur d'initialisation de SDL_image: %s", IMG_GetError());
        SDL_Quit();
        exit(-1);
    }

    if (TTF_Init()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error in font init: %s", TTF_GetError());
        exit(-1);
    }

    // Init fenêtre et renderer
    window = SDL_CreateWindow("TN Escape", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error in window init: %s", SDL_GetError());
        exit(-1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error in renderer init: %s", SDL_GetError());
        exit(-1);
    }

    // Chargement de la police du chrono
    TTF_Font *font = loadFont("res/fonts/police_chrono.ttf", 24);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "error loading font: %s", TTF_GetError());
        exit(-1);
    }
    
    // Initialiser la map
    char nomMap[100]; // Pour stocker temporairement le nom du fichier de la map
    char nomBlob[100]; // Pour stocker temporairement le nom du fichier des blobs
    sprintf(nomMap, "res/fonts/map_%d.txt", mapNum);
    sprintf(nomBlob, "res/fonts/blob_%d.txt", mapNum);
    Map map = loadMap(nomMap, NUM_IMAGES);
    Map monster_map = loadMap(nomBlob, MAX_MONSTERS);

    // Initialiser le personnage et les images
    character = loadCharacter("res/images/Rambo.png", &map);
    SDL_Rect allure_ini = *character.allure;
    SDL_Rect pos_map_ini = *character.pos_map; // Stockage de la position initiale pour réinitialiser en cas de mort
    loadImages();

    // Initialiser les monstres et les projectiles
    initMonsters(monsters, "res/images/Blob.png", "res/fonts/blob_1.txt");
    Projectile *projectileList = NULL;
    
    // Déclaration des variables de temps du jeu
    Uint32 startTime;
    Uint32 pause_start_Time;
    Uint32 elapsed_pauseTime = 0;
    Uint32 playTime;
    Uint32 elapsedTime;
    Uint32 remainingTime;
    srand(time(NULL)); // Initialisation de la fonction rand

    // Création des textures pour les options du menu
    SDL_Texture* menuTexture = IMG_LoadTexture(renderer, "res/images/menu.png");
    SDL_Texture* reglesTexture = IMG_LoadTexture(renderer, "res/images/regles.png");
    SDL_Texture* commandesTexture = IMG_LoadTexture(renderer, "res/images/commandes.png");
    SDL_Texture* defaite_mort = IMG_LoadTexture(renderer, "res/images/defaite_mort.png");
    SDL_Texture* defaite_temps = IMG_LoadTexture(renderer, "res/images/defaite_temps.png");
    SDL_Texture* victoire = IMG_LoadTexture(renderer, "res/images/victoire.png");

    int textureWidth, textureHeight;
    SDL_QueryTexture(menuTexture, NULL, NULL, &textureWidth, &textureHeight);

    // Initialisation musique (!!!!) et sons
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }

    Mix_Music *backgroundMusic = Mix_LoadMUS("res/sound/music.mpga");   // Musique de fond, on y croit personne va se foutre en l'air à force de l'écouter
    if (!backgroundMusic) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }
    Mix_PlayMusic(backgroundMusic, -1);

    // Pour les effets sonore on utilise Mix_Chunk sinon on ne peut pas superposer l'effet sonore avec la musique de fond
    Mix_Chunk *bruit_coup = Mix_LoadWAV("res/sound/coups.mpga");    // Petit son quand on frappe un mob et qu'il ne meurt pas
    if (!bruit_coup) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }

    Mix_Chunk *cri = Mix_LoadWAV("res/sound/cri_perso.mpga");               // Petit cri gênant quand il frappe avec son épée
    if (!cri) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }
    Mix_VolumeChunk(cri, MIX_MAX_VOLUME / 6); // Permet de baisser le volume du cri

    Mix_Music *deathMusic = Mix_LoadMUS("res/sound/mort.mpga");             // Bruit de la défaite. Si tu l'entends c'est que t'es mauvais Jack !
    if (!deathMusic) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }

    Mix_Music *winnersMusic = Mix_LoadMUS("res/sound/victoire.mpga");       // Bruit de la Victoire. C'est pas pour les faibles !
    if (!winnersMusic) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }

    Mix_Chunk *fireChunk = Mix_LoadWAV("res/sound/bouleDeFeu.mpga");        // Son quand le mob tire une boule de feu (NB : personne n'a pété dans le micro)
    if (!fireChunk) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }

    Mix_Chunk *mobDeathMusic = Mix_LoadWAV("res/sound/detruitmob.mpga");    // Son quand on tue un ennemi. J'aime écraser mes adversaire
    if (!mobDeathMusic) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        exit(-1);
    }

    // Initialisation des event
    SDL_Event event;

    int delaiChgmtDir = 1;  // Compteur pour ne pas changer la direction des mobs à chaque itération de la boucle while


    while(inMenu || running){   // Jeu toujours en cours si on est dans le menu ou en partie (pour pouvoir alterner entre les 2)

        while(inMenu) {
            
            if (inMenu == 1) { // Afficher le menu

                SDL_RenderCopy(renderer, menuTexture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(5);

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            inMenu = 0;
                            break;
                        case SDL_KEYDOWN:
                            switch (event.key.keysym.sym) {
                                case SDLK_RETURN:   // JOUER [ENTREE]
                                    inMenu = 0;
                                    running = 1;
                                    if (pause){
                                        elapsed_pauseTime += SDL_GetTicks()-pause_start_Time;   // Calcul du temps écoulé pendant la pause
                                        pause = 0;
                                    }
                                    else{
                                        startTime = SDL_GetTicks(); // Démarrage du chrono
                                    }
                                    break;
                                case SDLK_r:        // REGLES [R]
                                    inMenu = 2;
                                    break;
                                case SDLK_c:        // COMMANDES [C]
                                    inMenu = 3;
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }

            } else if (inMenu == 2) { // Afficher les règles

                SDL_RenderCopy(renderer, reglesTexture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(5);

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            inMenu = 0;
                            break;
                        case SDL_KEYDOWN:
                            switch (event.key.keysym.sym) {
                                case SDLK_RETURN:   // JOUER [ENTREE]
                                    inMenu = 0;
                                    running = 1;
                                    if (pause){
                                        elapsed_pauseTime += SDL_GetTicks()-pause_start_Time;   // Calcul du temps écoulé pendant la pause
                                        pause = 0;
                                    }
                                    else{
                                        startTime = SDL_GetTicks(); // Démarrage du chrono
                                    }
                                    break;
                                case SDLK_r:        // REGLES [R]
                                    inMenu = 2;
                                    break;
                                case SDLK_c:        // COMMANDES [C]
                                    inMenu = 3;
                                    break;
                                case SDLK_ESCAPE:   // Retour menu [ESC]
                                    inMenu = 1;
                                    break;
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }

            } else if (inMenu == 3) { // Afficher les commandes

                SDL_RenderCopy(renderer, commandesTexture, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(5);

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            inMenu = 0;
                            break;
                        case SDL_KEYDOWN:
                            switch (event.key.keysym.sym) {
                                case SDLK_RETURN:   // JOUER [ENTREE]
                                    inMenu = 0;
                                    running = 1;
                                    if (pause){
                                        elapsed_pauseTime += SDL_GetTicks()-pause_start_Time;   // Calcul du temps écoulé pendant la pause
                                        pause = 0;
                                    }
                                    else{
                                        startTime = SDL_GetTicks(); // Démarrage du chrono
                                    }
                                    break;
                                case SDLK_r:        // REGLES [R]
                                    inMenu = 2;
                                    break;
                                case SDLK_c:        // COMMANDES [C]
                                    inMenu = 3;
                                    break;
                                case SDLK_ESCAPE:   // Retour menu [ESC]
                                    inMenu = 1;
                                    break;
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }

            } else if (inMenu == 4) { // mort du joueur

                SDL_RenderCopy(renderer, defaite_mort, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(5);

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            inMenu = 0;
                            break;
                        case SDL_KEYDOWN:
                            switch (event.key.keysym.sym) {
                                case SDLK_RETURN:   // JOUER [ENTREE]
                                    inMenu = 0;
                                    running = 1;
                                    if (pause){
                                        elapsed_pauseTime += SDL_GetTicks()-pause_start_Time;   // Calcul du temps écoulé pendant la pause
                                        pause = 0;
                                    }
                                    else{
                                        startTime = SDL_GetTicks(); // Démarrage du chrono
                                    }
                                    break;
                                case SDLK_r:        // REGLES [R]
                                    inMenu = 2;
                                    break;
                                case SDLK_c:        // COMMANDES [C]
                                    inMenu = 3;
                                    break;
                                case SDLK_ESCAPE:   // Retour menu [ESC]
                                    inMenu = 1;
                                    break;
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }

            } else if (inMenu == 5) { // timeout

                SDL_RenderCopy(renderer, defaite_temps, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(5);

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            inMenu = 0;
                            break;
                        case SDL_KEYDOWN:
                            switch (event.key.keysym.sym) {
                                case SDLK_RETURN:   // JOUER [ENTREE]
                                    inMenu = 0;
                                    running = 1;
                                    if (pause){
                                        elapsed_pauseTime += SDL_GetTicks()-pause_start_Time;   // Calcul du temps écoulé pendant la pause
                                        pause = 0;
                                    }
                                    else{
                                        startTime = SDL_GetTicks(); // Démarrage du chrono
                                    }
                                    break;
                                case SDLK_r:        // REGLES [R]
                                    inMenu = 2;
                                    break;
                                case SDLK_c:        // COMMANDES [C]
                                    inMenu = 3;
                                    break;
                                case SDLK_ESCAPE:   // Retour menu [ESC]
                                    inMenu = 1;
                                    break;
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }

            } else if (inMenu == 6) { // victoire

                SDL_RenderCopy(renderer, victoire, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_Delay(5);

                while (SDL_PollEvent(&event)) {
                    switch (event.type) {
                        case SDL_QUIT:
                            inMenu = 0;
                            break;
                        case SDL_KEYDOWN:
                            switch (event.key.keysym.sym) {
                                case SDLK_RETURN:   // JOUER [ENTREE]
                                    inMenu = 0;
                                    running = 1;
                                    if (pause){
                                        elapsed_pauseTime += SDL_GetTicks()-pause_start_Time;   // Calcul du temps écoulé pendant la pause
                                        pause = 0;
                                    }
                                    else{
                                        startTime = SDL_GetTicks(); // Démarrage du chrono
                                    }
                                    break;
                                case SDLK_r:        // REGLES [R]
                                    inMenu = 2;
                                    break;
                                case SDLK_c:        // COMMANDES [C]
                                    inMenu = 3;
                                    break;
                                case SDLK_ESCAPE:   // Retour menu [ESC]
                                    inMenu = 1;
                                    break;
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                }

            }
        }

        while (running) { // Boucle de jeu dans le labyrinthe

            /* Actions du joueur (interactions et maj) */

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:      // Arrêt complet du jeu
                        running = 0;
                        inMenu = 0;
                        break;
                    case SDL_KEYDOWN:
                        if (event.key.keysym.sym == SDLK_ESCAPE)    // Pause [ESC]
                        {
                            pause = 1;
                            running = 0;
                            inMenu = 1;
                            pause_start_Time = SDL_GetTicks();
                        }
                        else if (event.key.keysym.sym == SDLK_SPACE)    // Tir joueur [ESPACE]
                        {
                            tirPerso(&projectileList, fireChunk);
                        }
                    break;
                }
            }

            deplace(&map, cri);



            /* Actions des mobs (interactions et maj) */

            delaiChgmtDir++;
            
            if (delaiChgmtDir % CADENCE_DEPLACEMENT == 0){
                deplaceMonster(&map, &monster_map, delaiChgmtDir);
            }



            /* Actions des projectiles (interactions et maj) */
            // On le met ici pour respecter la boucles des events présentés en CM (et c'est plus logique en plus)
            for (int i = 0; i < monster_map.numMonster; i++) {

                if (monsters[i] != NULL){

                    if (tirRandom()) {
                        addProjectile(&projectileList, fireChunk, *monsters[i]);
                    }

                }
            }


            moveProjectiles(&projectileList, &map, &monster_map, mobDeathMusic);



            /* Calcul des interactions et maj de l'état du jeu */

            playTime = SDL_GetTicks() - startTime - elapsed_pauseTime; // Temps de jeu = nombre de ms depuis lancement du programme - timer de début de partie (ajusté avec le temps passé en pause)
            elapsedTime = playTime / 1000;  // secondes
            remainingTime = CHRONO - elapsedTime;  // 10 minutes = 600 secondes - à modifier éventuellement selon la difficulté, voire à augmenter si bonus

            int tileX = (character.pos_map->x+25) / TILE_SIZE;
            int tileY = (character.pos_map->y+25) / TILE_SIZE;

            if (map.tiles[tileY][tileX] == 4) { // Changement de map si le personnage atteint la porte (case 4)

                clearMap(map);
                clearMap(monster_map);

                char nomMap[100]; // Pour stocker temporairement le nom du fichier de la map
                char nomBlob[100]; // Pour stocker temporairement le nom du fichier des blobs
                if (mapNum<3){
                    mapNum++;
                }
                else{
                    inMenu = 6;
                    running = 0;
                    mapNum = 1;
                    elapsed_pauseTime = 0;
                    Mix_PlayMusic(winnersMusic, 0);
                    SDL_Delay(1500);
                    Mix_PlayMusic(backgroundMusic, -1);
                }

                sprintf(nomMap, "res/fonts/map_%d.txt", mapNum);
                sprintf(nomBlob, "res/fonts/blob_%d.txt", mapNum);

                clearProjectiles(&projectileList);
                clearAllMonsters();
                
                map = loadMap(nomMap, NUM_IMAGES);
                monster_map = loadMap(nomBlob, MAX_MONSTERS);

                initMonsters(monsters, "res/images/Blob.png", nomBlob);
                init_player(&map, &character);
            }

            if (death==1) {   // Arrêt de la partie si temps restant à 0 ou mort du joueur
                // Game Over
                running = 0;
                inMenu = 4;
                death = 0;
                elapsed_pauseTime = 0;

                Mix_PlayMusic(deathMusic, 0);
                SDL_Delay(1500);
                Mix_PlayMusic(backgroundMusic, -1);

                *character.pos_map = pos_map_ini;
                *character.allure = allure_ini;

                // Recharger la map n°1 et les monstres de cette map
                char nomMap[100]; // Pour stocker temporairement le nom du fichier de la map
                char nomBlob[100]; // Pour stocker temporairement le nom du fichier des blobs
                mapNum = 1;

                sprintf(nomMap, "res/fonts/map_%d.txt", mapNum);
                sprintf(nomBlob, "res/fonts/blob_%d.txt", mapNum);

                clearMap(map);
                clearMap(monster_map);
                map = loadMap(nomMap, NUM_IMAGES);
                monster_map = loadMap(nomBlob, MAX_MONSTERS);

                initMonsters(monsters, "res/images/Blob.png", nomBlob);

                clearProjectiles(&projectileList);
            }

            if (remainingTime <= 5) {   // Arrêt de la partie si temps restant à 0 ou mort du joueur
                // Game Over
                running = 0;
                inMenu = 5;
                death = 0;
                elapsed_pauseTime = 0;

                Mix_PlayMusic(deathMusic, 0);
                SDL_Delay(1500);
                Mix_PlayMusic(backgroundMusic, -1);

                *character.pos_map = pos_map_ini;
                *character.allure = allure_ini;

                // Recharger la map n°1 et les monstres de cette map
                char nomMap[100]; // Pour stocker temporairement le nom du fichier de la map
                char nomBlob[100]; // Pour stocker temporairement le nom du fichier des blobs
                mapNum = 1;

                sprintf(nomMap, "res/fonts/map_%d.txt", mapNum);
                sprintf(nomBlob, "res/fonts/blob_%d.txt", mapNum);

                clearMap(map);
                clearMap(monster_map);
                map = loadMap(nomMap, NUM_IMAGES);
                monster_map = loadMap(nomBlob, MAX_MONSTERS);

                initMonsters(monsters, "res/images/Blob.png", nomBlob);

                clearProjectiles(&projectileList);
            }

            /* Mise à jour de l'état graphique */
            
            //Render game objects
            drawMap(map);
            SDL_RenderCopy(renderer, character.sprite, character.allure, character.pos_map);
            drawMonsters(&monster_map);
            renderProjectiles(projectileList, false);

            // Affichage du temps à l'écran
            char elapsedTimeText[50];
            snprintf(elapsedTimeText, sizeof(elapsedTimeText), "TEMPS ECOULE : %02d:%02d", elapsedTime / 60, elapsedTime % 60);

            char remainingTimeText[50];
            snprintf(remainingTimeText, sizeof(remainingTimeText), "TEMPS RESTANT : %02d:%02d", remainingTime / 60, remainingTime % 60);

            SDL_Color white = { 255, 255, 255, 255 };
            int textWidth = 0, textHeight = 0;
            TTF_SizeText(font, elapsedTimeText, &textWidth, &textHeight);
            
            renderText(remainingTimeText, font, white, 10, 10); // En haut à gauche
            renderText(elapsedTimeText, font, white, SCREEN_WIDTH - textWidth - 10, 10); // En haut à droite

            SDL_RenderPresent(renderer); // Fait l'échange des frames buffers et affiche le nouvel écran au joueur
            SDL_Delay(5); // délai de 5 ms pour ne pas que les boucles soient trop rapides
        }
    }
    
    /* Libération de la mémoire */
/*
    for(int y = 0; y < MAP_HEIGHT; y++) {
        free(map.tiles[y]);
    }
    free(map.tiles);
    for(int y = 0; y < MAP_HEIGHT; y++) {
        free(monster_map.tiles[y]);
    }
    free(monster_map.tiles);
    */
    Mix_FreeMusic(backgroundMusic);
    backgroundMusic = NULL;

    Mix_FreeChunk(bruit_coup);
    bruit_coup = NULL;

    Mix_FreeChunk(cri);
    cri = NULL;

    Mix_FreeMusic(deathMusic);
    deathMusic = NULL;
    
    Mix_FreeMusic(winnersMusic);
    winnersMusic = NULL;

    Mix_FreeChunk(fireChunk);
    fireChunk = NULL;

    Mix_FreeChunk(mobDeathMusic);
    mobDeathMusic = NULL;

    clearMap(map);
    clearMap(monster_map);
    clearCharacter();
    clearAllMonsters();
    clearProjectiles(&projectileList);
    
    SDL_DestroyTexture(menuTexture);
    SDL_DestroyTexture(reglesTexture);
    SDL_DestroyTexture(commandesTexture);    

    TTF_CloseFont(font);
    cleanupFonts();

    for (int i = 0; i < NUM_IMAGES; ++i) {
        if (textures[i] != NULL) {
            SDL_DestroyTexture(textures[i]);
            textures[i] = NULL;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

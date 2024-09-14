                                                            TN ESCAPE

This is a little game written in C with SDL.

Dependencies :
    - SDL2 SDL
    - SDL2 SDL_ttf
    - SDL2 SDL_image
    - SDL2 SDL_mixer

This project can run on MacOs, Windows or Linux.
We only ran the code on Linux, so these are the shell instructions to add dependencies on linux.

    - sudo apt install libsdl2-dev
    - sudo apt install libsdl2-ttf-dev
    - sudo apt install libsdl2-image-dev
    - sudo apt install libsdl2-mixer-dev

How to play ?
These are the three instructions in order to use the project. They have to be done at the root of the project.
    - make clean : remove main and main_test executables and also all .o files in the src/ directory
    - make all : create main and test_main executables
    - make run : create main and test_main executables and lunch main.

So to play, you just have to be on the root of the project and execute :
    make clean run


The purpose of the game is to find the exit without being killed by ennemies !
Good luck !

--- Made by Benjamin DUPIED, Orianne GAUTROT, Valentin SALQUEBRE, Timothée NANO-LAGADEC ---

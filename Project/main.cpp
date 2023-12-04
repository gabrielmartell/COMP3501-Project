/*
 *
 * A program that demonstrates texturing with normal mapping
 *
 * Copyright (c) 2018 Oliver van Kaick <Oliver.vanKaick@carleton.ca>, David Mould <mould@scs.carleton.ca>
 *
 */


#include <iostream>
#include <exception>
#include "game.h"

// Macro for printing exceptions
#define PrintException(exception_object)\
	std::cerr << exception_object.what() << std::endl

// Main function that builds and runs the game
int main(void){
    game::Game app; // Game application

    try {
        printf("LOADING GAME...\n");
        // Initialize game
        app.Init();
        // Setup the main resources and scene in the game
        app.SetupResources();
        printf("[1]. RESOURCES LOADED\n");
        app.SetupScene();
        printf("[2]. WORLD LOADED\n");

        // Run game
        app.MainLoop();
    }
    catch (std::exception &e){
        PrintException(e);
    }

    return 0;
}

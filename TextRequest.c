#include "TextRequest.h"


float getDificulty() {
    char* options[2];
    options[0] = "Ingresar dificultad.";
    options[1] = "Dejarlo a la suerte.";

    int selection = showMenu(options, 2);
    if (selection == USER_INPUT) {
        return getUserDificulty();
    } else {
        return getRandomDificulty();
    }
}

float getUserDificulty() {
    return askFloat("Ingrese la dificultad: ", 0.0f, 100.0f);
}

float getRandomDificulty() {
    return (rand() / 100) % 100;
}

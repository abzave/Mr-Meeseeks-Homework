#include "TextRequest.h"

/**
 * Ask the user for how to determine the dificulty of the task
 */
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

/**
 * Get the time elapsed between 2 time marks in seconds
 * @param initTime {clock_t} first marker
 * @param endTime {clock_t} second marker
 */
double timediff(clock_t initTime, clock_t endTime) {
    double elapsed;
    elapsed = ((double)endTime - initTime) / CLOCKS_PER_SEC * 1000000.0;
    timeElasep += elapsed;
    return elapsed;
}

/**
 * Check if the time elapsed is enough to declare global chaos
 */
int systemCollapsed(){
    return timeElasep >= timeLimitInSeconds;
}

/**
 * We have global chaos and now we need to kill all our chidren
 * Here for the chaos!
 */
void declareGlobalChaos() {
    printf("La solicitud es muy dificil hay caos global :S\n");
    for (int child = 0; child < lastChild; child++) {
        kill(children[child], SIGKILL);
    }
}
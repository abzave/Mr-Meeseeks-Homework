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
    srand(time(NULL));
    return (rand() % 100) / 100.0;
}

/**
 * Determines if a mr meeseek can solve the request proposed
 * It has a probability of solving the problem, but it get less
 * likely if the request is too short because of lack of information
 * and if the problem is taking too long to solve because they get upset
 * @param difficulty {float} Difficulty of the request
 * @param request {char*} Resquest given to the mr meeseek
 */ 
int problemSolved(float difficulty, char* request) {
    srand(time(NULL));
    float chance = (rand() % 100) / 100.0f;
    
    int requestLength = strlen(request);
    chance = chance / requestLength * timeElasep * 10;

    return chance < difficulty;
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
    return timeElasep >= timeLimitInSeconds || lastChild >= MAX_CHILDREN;
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

/**
 * Returns a random time where the mr meeseek will be working
 * @param min {double} minimum value allowed
 * @param max {double} maximum value allowed
 */
double getWorkingTime(double min, double max) {
    srand(time(NULL));
    return ((rand() % (int)((max - min) * 100)) / 100.0) + min;
}

/**
 * Calcute the amount of help the mr meekseek will ask for
 * @param difficulty {float} difficulty of the task
 */
int helpAmount(float difficulty) {
    int minAmount = 0;

    if (difficulty <= HARD) {
        minAmount += 3;
    } else if (difficulty <= MEDIUM) {
        minAmount += 1;
    }

    srand(time(NULL));
    return (rand() % (int)((minAmount * 2))) + minAmount;
}

/**
 * Re-calculates the difficulty based on the help requested
 * If the mr meeseek did not request help then the difficulty is kept
 * @param difficulty {float} difficulty of the task
 * @param helpAmount {int} Amount of mr meeseeks request for help
 */
float diluteDifficult(float difficulty, int helpAmount) {
    if (helpAmount == 0) {
        return difficulty;
    }
    return difficulty * (helpAmount + 1) / (1.0f + (difficulty / 100.0f));
}
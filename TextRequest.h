#ifndef TEXT_REQUEST
#define TEXT_REQUEST


#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

#include "CLI/interface.h"

#define USER_INPUT          1
#define LUCK_DIFICULTY      2

int timeLimitInSeconds = 300;
double timeElasep = 0;

pid_t children[300];        // TODO: Change magic constant
int lastChild = 0;

double minWorkingTime = 0.5;
double maxWorkingTime = 5;

float getDificulty(void);
float getUserDificulty(void);
float getRandomDificulty(void);

double timediff(clock_t, clock_t);
void declareGlobalChaos(void);

double getWorkingTime(double, double); 

#endif
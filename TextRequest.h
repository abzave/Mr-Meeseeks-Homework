#ifndef TEXT_REQUEST
#define TEXT_REQUEST


#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "CLI/interface.h"

#define USER_INPUT          1
#define LUCK_DIFICULTY      2
#define HARD                45
#define MEDIUM              85
#define EASY                100
#define MAX_CHILDREN        100

int timeLimitInSeconds = 300;
double timeElasep = 0;

pid_t children[MAX_CHILDREN];
int lastChild = 0;

double minWorkingTime = 0.5;
double maxWorkingTime = 5;

float getDificulty(void);
float getUserDificulty(void);
float getRandomDificulty(void);
float diluteDifficult(float, int);

double timediff(clock_t, clock_t);
void declareGlobalChaos(void);

double getWorkingTime(double, double); 
int problemSolved(float, char*);

#endif
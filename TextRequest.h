#ifndef TEXT_REQUEST
#define TEXT_REQUEST


#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "CLI/interface.h"

#define USER_INPUT          1
#define LUCK_DIFICULTY      2
#define HARD                45
#define MEDIUM              85
#define EASY                100
#define MAX_CHILDREN        100

extern int timeLimitInSeconds;
extern double timeElasep;

extern pid_t children[MAX_CHILDREN];
extern int lastChild;

extern double minWorkingTime;
extern double maxWorkingTime;

float getDificulty(void);
float getUserDificulty(void);
float getRandomDificulty(void);
float diluteDifficult(float, int);

double timediff(clock_t, clock_t);
void declareGlobalChaos(void);
int systemCollapsed(void);

double getWorkingTime(double, double); 
int problemSolved(float, char*);

int helpAmount(float);

#endif
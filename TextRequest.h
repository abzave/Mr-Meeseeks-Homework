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
#define HARD                45.0f
#define MEDIUM              85.0f
#define EASY                100.0f
#define MAX_CHILDREN        9

extern int timeLimitInSeconds;
extern double timeElasep;

extern int lastChild;

float getDificulty(void);
float getUserDificulty(void);
float getRandomDificulty(void);
float diluteDifficult(float, int);

double timediff(time_t, time_t);
void declareGlobalChaos(void);
int systemCollapsed(int);

double getWorkingTime(double, double); 
int problemSolved(float, char*);
void solveProblem(void);
void killChildren(pid_t children[MAX_CHILDREN], int);

int helpAmount(float);

#endif
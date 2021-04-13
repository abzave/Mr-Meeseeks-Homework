#ifndef TEXT_REQUEST
#define TEXT_REQUEST


#include <sys/types.h>
#include <time.h>
#include <stdlib.h>

#include "CLI/interface.h"

#define USER_INPUT          1
#define LUCK_DIFICULTY      2

float getDificulty(void);
float getUserDificulty(void);
float getRandomDificulty(void);

#endif
#ifndef INTERFACE
#define INTERFACE

#include <stdlib.h>
#include <stdio.h>

int showMenu(char**, int);
int askOption(int);
float askFloat(char*, float, float);
void write_output (FILE * stream);

#endif
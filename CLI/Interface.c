#include "interface.h"

int showMenu(char** options, int length) {
    // We want to make sure there is nothing else in the screen
    //system("clear");

    printf("********Menu********\n");
    for (int option = 1; option <= length; option++) {
        printf("%d. %s\n", option, options[option - 1]);
    }
    
    return askOption(length);
}

int askOption(int numberOfOptions) {
    printf("\nDigite una opcion: ");

    int option = 0;
    // Ask until the user gives a valid input
    while(1){
        scanf("%d", &option);
        if (option > 0 && option <= numberOfOptions){
            return option;
        }
        printf("Debe ingresar una opcion entre 1 y %d: ", numberOfOptions);
    }
}

float askFloat(char* message, float min, float max) {
    // We want to make sure there is nothing else in the screen
    //system("clear");
    
    printf("%s", message);

    float value = 0.0f;
    // Ask until the user gives a valid input
    while(1){
        scanf("%f", &value);
        if (value >= min && value <= max){
            return value;
        }
        printf("Debe ingresar una opcion entre %.2f y %.2f: ", min, max);
    }
}
    
void write_output (FILE * stream)
{
    char ch;
    while( (ch=fgetc(stream)) != EOF)
        putchar(ch);
}
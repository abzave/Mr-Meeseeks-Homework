#include <stdio.h>
#include <stdlib.h>

char* askForCommand() {
    char* str;
    printf("Please type the desired command: \n");
    scanf("%s", str);
    return str;
}

void write_output (FILE * stream)
{
    char ch;
    while( (ch=fgetc(stream)) != EOF)
        putchar(ch);
}

int main () {
    FILE *output;
    output = popen ("uname", "r");
    if (!output) {
        printf ("I can't solve your request\n");
        return 1;
    }
    write_output (output);
    if (pclose (output) != 0) {
        printf ("I can't solve your request\n");
    }
    return 0;
}

#include "ExpresionEval.h"
#include "TextRequest.h"

#define TEXT                1
#define OPERATION           2
#define EXTENAL_PROGRAM     3

static struct expr_func user_funcs[] = {
    {NULL, NULL, NULL, 0},
};

int main(void) {

    int timeLimitInSeconds = 300;
    double timeElasep = 0;

    pid_t children[MAX_CHILDREN];
    int lastChild = 0;

    double minWorkingTime = 0.5;
    double maxWorkingTime = 5;


    char* options[3];
    char request[300];

    options[0] = "Consulta Textual.";
    options[1] = "Operación Aritmetica o Logica.";
    options[2] = "Ejecución de Programa Externo.";

    int option = showMenu(options, 3);
    printf("%s", "Ingrese la consulta: ");
    scanf("%s", request);

    switch(option) {
        case TEXT: {
            float difficulty = getDificulty();
            double time = getWorkingTime(minWorkingTime, maxWorkingTime);
            int help = helpAmount(difficulty);
            difficulty = diluteDifficult(help, difficulty);

            // Forks
            usleep(time);
            if (problemSolved(difficulty, request)) {
                // Solved!
            } else if(systemCollapsed()) {
                declareGlobalChaos();
            }
            break;
        }
        case OPERATION:{
            const char *s = request;
            struct expr_var_list vars = {0};
            struct expr *e = expr_create(s, strlen(s), &vars, user_funcs);
            if (e == NULL) {
                printf("Syntax error");
                return 1;
            }

            float result = expr_eval(e);
            printf("result: %f\n", result);

            expr_destroy(e, &vars);
            break;
        }
        case EXTENAL_PROGRAM:{
            FILE *output;
            output = popen (request, "r");
            if (!output) {
                printf ("I can't solve your request\n");
                return 1;
            }
            write_output (output);
            if (pclose (output) != 0) {
                printf ("I can't solve your request\n");
            }
            break;
        }
    }
    
    return 0;
}
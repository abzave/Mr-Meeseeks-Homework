/*
 * TEC, Cartago, Escuela de Ingeniería en Computación, Principios de Sistemas Operativos
 * Tarea #2: Mr. Meeseeks
 * Abraham Meza Vega, 2018168174
 * Lindsay Morales Bonilla, 2018077301
 * 24/04/2021, I Semestre 2021, Prof. Esteban Arias Méndez
*/

#include "ExpresionEval.h"
#include "TextRequest.h"
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define TEXT                1
#define OPERATION           2
#define EXTENAL_PROGRAM     3

double minWorkingTime = 0.5;
double maxWorkingTime = 5;
double timeElasep = 0;
double generalTime = 0;
int requestsAmout = 0;
int timeLimitInSeconds = 300;

void createMrMeeseekHelp(int, float, char*, int, void*, time_t);
void textMrMeeseekWork(int, float, char*, int, void*, pid_t children[MAX_CHILDREN], int file_pipes[2], time_t, int, int);

static struct expr_func user_funcs[] = {
    {NULL, NULL, NULL, 0},
};

struct shared_use_st {
    int instance;
    int level;
    int chaos;
    pid_t solverPid;
    pid_t solverPpid;
    int solverLevel;
    int solverInstance;
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int createSharedMem() {
    int shmid;

    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    return shmid;
}

void* attachSharedMemory(int sharedMemoryId) {
    void *shared_memory = shmat(sharedMemoryId, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    return shared_memory;
}

static void set_semvalue(int sem_id) {
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
    }
}

static int semaphore_p(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;  /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

static int semaphore_v(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}

void createMrMeeseekHelp(int help, float difficulty, char* request, int sem_id, void* sharedMemory, time_t requestStart) {
    pid_t pid;
    char status[300];
    pid_t children[MAX_CHILDREN];
    
    semaphore_p(sem_id);
    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
    sharedData->level = sharedData->level + 1;
    semaphore_v(sem_id);

    difficulty = diluteDifficult(difficulty, help);
    
    for (int childIndex = 0; childIndex < help; childIndex++) {
        int file_pipes[2];
        if (pipe(file_pipes) != 0) {
            printf("Failed to open pipe\n");
            exit(EXIT_FAILURE);
        }
        if (help > 1){
            pid = fork();
        } else {
            help = 0;
            pid = 0;
        }
        double time = getWorkingTime(minWorkingTime, maxWorkingTime) * 1000000;
        if (pid < 0) {
            fprintf(stderr, "Fork %d failed\n", childIndex);
        } else if (pid == 0 && help > 0) {

            int processInstance;
            int processLevel;

            semaphore_p(sem_id);
            struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
            processInstance = sharedData->instance;
            processLevel = sharedData->level;
            printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), processInstance, processLevel);
            sharedData->instance = sharedData->instance + 1;
            semaphore_v(sem_id);

            printf("Tiempo de espera: %f\n", time);
            usleep(time);
            read(file_pipes[0], request,300);
            read(file_pipes[0], status, 300);

            printf("Request: %s\n", request);
            printf("Status: %s\n", status);
            if (status == "solved") {
                write(file_pipes[1], request, 300);
                write(file_pipes[1], "solved", 300);
                exit(0);
            }
            textMrMeeseekWork(childIndex, difficulty, request, sem_id, sharedMemory, children, file_pipes, requestStart, processInstance, processLevel);
            exit(0);       // We return so the child do not fork
        } else {
            int processInstance;
            int processLevel;

            write(file_pipes[1], request, 300);
            write(file_pipes[1], "unsolved", 300);

            semaphore_p(sem_id);
            struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
            processInstance = sharedData->instance;
            processLevel = sharedData->level;
            printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), sharedData->level, sharedData->instance);
            sharedData->instance = sharedData->instance + 1;
            semaphore_v(sem_id);

            children[childIndex] = pid;


            printf("Tiempo de espera: %f\n", time);
            usleep(time);
            read(file_pipes[0], request,300);
            read(file_pipes[0], status,300);

            if (status == "solved") {
                semaphore_p(sem_id);
                struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                sharedData->level = sharedData->level - 1;
                semaphore_v(sem_id);

                killChildren(children, help);
                write(file_pipes[1], request, 300);
                write(file_pipes[1], "solved", 300);
                exit(0);
            }
            textMrMeeseekWork(childIndex, difficulty, request, sem_id, sharedMemory, children, file_pipes, requestStart, processInstance, processLevel);
        }
    }

    for (int childIndex = 0; childIndex < help; childIndex++) {
        wait(NULL);
    }
}

void textMrMeeseekWork(int help, float difficulty, char* request, int sem_id, void* sharedMemory, pid_t children[MAX_CHILDREN], int file_pipes[2], time_t requestStart, int instance, int level) {
    semaphore_p(sem_id);
    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;

    if (sharedData->chaos == 1){    
        semaphore_v(sem_id);

        killChildren(children, help);
        exit(0);
    } else if (problemSolved(difficulty, request)) {
        write(file_pipes[1], request, 300);
        write(file_pipes[1], "solved", 300);
        
        sharedData->solverPid = getpid();
        sharedData->solverPpid = getppid();
        sharedData->solverLevel = level;
        sharedData->solverInstance = instance;
        semaphore_v(sem_id);

        solveProblem();
        killChildren(children, help);
        exit(0);
    } else if(systemCollapsed(sharedData->level, requestStart)) {
        sharedData->chaos = 1;
        semaphore_v(sem_id);

        declareGlobalChaos();
        killChildren(children, help);
        exit(0);
    } else {
        semaphore_v(sem_id);
        write(file_pipes[1], request, 300);
        write(file_pipes[1], "unsolved", 300);
        help = helpAmount(difficulty) + 1;
        createMrMeeseekHelp(help, difficulty, request, sem_id, sharedMemory, requestStart);
    }
}

void printReporting(char* requests[300], double requestTime[300], int requestStatus[300]) {
    printf("Tiempo transcurrido: %f segundos\n", generalTime);
    printf("Tiempo aleatorio de espera: %f segundos\n", timeElasep);
    printf("Consultas realizadas: %d\n", requestsAmout);

    for(int requestIndex = 0; requestIndex < requestsAmout; requestIndex++) {
        printf("Consulta: %s, Tomo: %f segundos, Se resolvio: %s\n", requests[requestIndex], requestTime[requestIndex], !requestStatus[requestIndex] ? "Si" : "No");
    }
}

int main(int argc, char *argv[]) {

    if (argc > 1 && argc < 3) {
        printf("No hay suficientes parametros. Experado:\n");
        printf("Tiempo-de-espera-minimo Tiempo-de-espera-maximo Tiempo-para-caos-global\n");
        exit(EXIT_FAILURE);
    }

    if (argc > 1) {
        minWorkingTime = atof(argv[1]);
        maxWorkingTime = atof(argv[2]);
        timeLimitInSeconds = atof(argv[3]);
    }

    time_t startTimer = time(NULL);

    pid_t children[MAX_CHILDREN];

    char* options[4];
    char request[300];
    char *requests[300];
    double requestTime[300];
    int requestStatus[300];

    pid_t childPid;
    pid_t boxPid = getpid();

    int shareMemoryId = createSharedMem();
    void* sharedMemory = attachSharedMemory(shareMemoryId);

    int sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    set_semvalue(sem_id);

    semaphore_p(sem_id);
    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
    sharedData->instance = 0;
    sharedData->level = 0;
    sharedData->chaos = 0;
    semaphore_v(sem_id);

    options[0] = "Consulta Textual.";
    options[1] = "Operación Aritmetica o Logica.";
    options[2] = "Ejecución de Programa Externo.";
    options[3] = "Salir.";
    int option = 0;

    while(1){
        semaphore_p(sem_id);
        struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
        sharedData->chaos = 0;
        semaphore_v(sem_id);

        option = showMenu(options, 4);
        if(option == 4) {
            break;
        }

        char temp;
        printf("%s", "Ingrese la consulta: ");
        scanf("%c",&temp); // temp statement to clear buffer
	    scanf("%[^\n]",request);
        requestsAmout++;

        switch(option) {
            case TEXT: {
                int file_pipes[2];
                if (pipe(file_pipes) != 0) {
                    printf("Failed to open pipe\n");
                    exit(EXIT_FAILURE);
                }
                time_t requestStart = time(NULL);
                childPid = fork();
                if (childPid < 0) {
                    printf("Failed to fork process");
                    exit(EXIT_FAILURE);
                } else if (childPid == 0) {
                    float difficulty = getDificulty();
                    int help = helpAmount(difficulty) + 1;
                    printf("Ayuda solicitad: %d\n", help);
                    
                    read(file_pipes[0], request,300);
                    createMrMeeseekHelp(help, difficulty, request, sem_id, sharedMemory, requestStart);
                    
                    return 0;
                } else {
                    write(file_pipes[1], request, 300);
                }

                int returnStatus;
                waitpid(childPid, &returnStatus, 0);
                if (boxPid != getpid()) {
                    return 0;
                }
                
                time_t requestEnd = time(NULL);
                double timeTaken = timediff(requestStart, requestEnd);
                timeElasep += timeTaken;
                
                int hadChaos;

                semaphore_p(sem_id);
                struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                hadChaos = sharedData->chaos;
                if (!hadChaos) {
                    printf("Consulta resulta por: (%d, %d, %d, %d)\n", sharedData->solverPid, sharedData->solverPpid, sharedData->solverLevel, sharedData->solverInstance);
                }
                semaphore_v(sem_id);

                requests[requestsAmout - 1] = strdup(request);
                requestTime[requestsAmout - 1] = timeTaken;
                requestStatus[requestsAmout - 1] = hadChaos;

                if (hadChaos) {
                    printReporting(requests, requestTime, requestStatus);
                    exit(EXIT_FAILURE);
                }

                break;
            }
            case OPERATION:{

                time_t requestStart = time(NULL);
                int file_pipes[2];
                if (pipe(file_pipes) != 0) {
                    printf("Failed to open pipe\n");
                    exit(EXIT_FAILURE);
                }
                childPid = fork();
                if (childPid < 0) {
                    printf("Failed to fork process");
                    exit(EXIT_FAILURE);
                } else if (childPid == 0) {
                    
                    read(file_pipes[0], request, 300);

                    semaphore_p(sem_id);
                    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                    printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getpid(), sharedData->level, sharedData->instance);
                    semaphore_v(sem_id);

                    struct expr_var_list vars = {0};
                    struct expr *e = expr_create(request, strlen(request), &vars, user_funcs);
                    if (e == NULL) {
                        semaphore_p(sem_id);
                        struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                        sharedData->chaos = 1;
                        semaphore_v(sem_id);

                        printf("Syntax error\n");
                        return 1;
                    }

                    float result = expr_eval(e);
                    printf("result: %f\n", result);

                    expr_destroy(e, &vars);
                    return 0;
                } else {
                    semaphore_p(sem_id);
                    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                    sharedData->instance = sharedData->instance + 1;
                    sharedData->level = sharedData->level + 1;
                    semaphore_v(sem_id);

                    write(file_pipes[1], request, 300);

                    wait(NULL);
                    time_t requestEnd = time(NULL);

                    semaphore_p(sem_id);
                    requestStatus[requestsAmout - 1] = sharedData->chaos;
                    semaphore_v(sem_id);

                    requests[requestsAmout - 1] = strdup(request);
                    requestTime[requestsAmout - 1] = timediff(requestStart, requestEnd);

                    semaphore_p(sem_id);
                    sharedData->level = sharedData->level - 1;  
                    semaphore_v(sem_id);
                }
                break;
            }
            case EXTENAL_PROGRAM:{
                semaphore_p(sem_id);
                struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                sharedData->instance = sharedData->instance + 1;
                sharedData->level = sharedData->level + 1;
                printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), sharedData->level, sharedData->instance);
                semaphore_v(sem_id);
                time_t requestStart = time(NULL);

                FILE *output;
                output = popen (request, "r");
                if (!output) {
                    semaphore_p(sem_id);
                    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                    sharedData->chaos = 1;
                    semaphore_v(sem_id);

                    printf ("I can't solve your request\n");
                    return 1;
                }
                write_output (output);
                if (pclose (output) != 0) {
                    semaphore_p(sem_id);
                    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                    sharedData->chaos = 1;
                    semaphore_v(sem_id);

                    printf ("I can't solve your request\n");
                }

                time_t requestEnd = time(NULL);

                semaphore_p(sem_id);
                requestStatus[requestsAmout - 1] = sharedData->chaos;
                semaphore_v(sem_id);

                requests[requestsAmout - 1] = strdup(request);
                requestTime[requestsAmout - 1] = timediff(requestStart, requestEnd);

                semaphore_p(sem_id);
                sharedData->level = sharedData->level - 1;
                semaphore_v(sem_id);
                break;
            }
        }
    }

    if (shmdt(sharedMemory) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shareMemoryId, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }

    time_t endTimer = time(NULL);
    generalTime = timediff(startTimer, endTimer) - timeElasep;
    printReporting(requests, requestTime, requestStatus);
    
    return 0;
}
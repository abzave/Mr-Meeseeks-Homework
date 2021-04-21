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

void createMrMeeseekHelp(int, float, char*, int, void*);
void textMrMeeseekWork(int, float, char*, int, void*, pid_t children[MAX_CHILDREN], int file_pipes[2]);

static struct expr_func user_funcs[] = {
    {NULL, NULL, NULL, 0},
};

struct shared_use_st {
    int instance;
    int level;
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

void createMrMeeseekHelp(int help, float difficulty, char* request, int sem_id, void* sharedMemory) {
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
        pid = fork();
        double time = getWorkingTime(minWorkingTime, maxWorkingTime) * 1000000;
        if (pid < 0) {
            fprintf(stderr, "Fork %d failed", childIndex);
        } else if (pid == 0) {

            semaphore_p(sem_id);
            struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
            printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), sharedData->level, sharedData->instance);
            sharedData->instance = sharedData->instance + 1;
            semaphore_v(sem_id);

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
            textMrMeeseekWork(help, difficulty, request, sem_id, sharedMemory, children, file_pipes);
            exit(0);       // We return so the child do not fork
        } else {
            children[childIndex] = pid;

            write(file_pipes[1], request, 300);
            write(file_pipes[1], "unsolved", 300);

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
            textMrMeeseekWork(help, difficulty, request, sem_id, sharedMemory, children, file_pipes);
        }
    }

    for (int childIndex = 0; childIndex < help; childIndex++) {
        wait(NULL);
    }
}

void textMrMeeseekWork(int help, float difficulty, char* request, int sem_id, void* sharedMemory, pid_t children[MAX_CHILDREN], int file_pipes[2]) {
    semaphore_p(sem_id);
    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                
    if (problemSolved(difficulty, request)) {
        semaphore_v(sem_id);
        solveProblem();
        write(file_pipes[1], request, 300);
        write(file_pipes[1], "solved", 300);
        exit(0);
    } else if(systemCollapsed(sharedData->level)) {
        semaphore_v(sem_id);
        declareGlobalChaos();
        killChildren(children, help);
        exit(0);
    } else {
        semaphore_v(sem_id);
        write(file_pipes[1], request, 300);
        write(file_pipes[1], "unsolved", 300);
        createMrMeeseekHelp(help, difficulty, request, sem_id, sharedMemory);
    }
}

int main(void) {

    time_t startTimer = time(NULL);

    int timeLimitInSeconds = 300;
    double timeElasep = 0;
    double generalTime = 0;

    pid_t children[MAX_CHILDREN];

    char* options[4];
    char request[300];

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
    semaphore_v(sem_id);

    options[0] = "Consulta Textual.";
    options[1] = "Operación Aritmetica o Logica.";
    options[2] = "Ejecución de Programa Externo.";
    options[3] = "Salir.";
    int option = 0;

    while(1){
        option = showMenu(options, 4);
        if(option == 4) {
            break;
        }

        printf("%s", "Ingrese la consulta: ");
        scanf("%s", request);

        switch(option) {
            case TEXT: {
                int file_pipes[2];
                if (pipe(file_pipes) != 0) {
                    printf("Failed to open pipe\n");
                    exit(EXIT_FAILURE);
                }
                childPid = fork();
                time_t requestStart = time(NULL);
                if (childPid < 0) {
                    printf("Failed to fork process");
                    exit(EXIT_FAILURE);
                } else if (childPid == 0) {
                    float difficulty = getDificulty();
                    int help = helpAmount(difficulty) + 1;
                    
                    read(file_pipes[0], request,300);
                    createMrMeeseekHelp(help, difficulty, request, sem_id, sharedMemory);
                    
                    return 0;
                } else {
                    write(file_pipes[1], request, 300);
                }

                wait(NULL);
                if (boxPid != getpid()) {
                    return 0;
                }
                
                time_t requestEnd = time(NULL);
                timeElasep += timediff(requestStart, requestEnd);
                break;
            }
            case OPERATION:{

                int file_pipes[2];
                if (pipe(file_pipes) != 0) {
                    printf("Failed to open pipe\n");
                    exit(EXIT_FAILURE);
                }
                childPid = fork();
                if (childPid < 0) {
                    printf("Failed to fork process");
                    exit(EXIT_FAILURE);
                } else if (childPid) {
                    
                    read(file_pipes[0], request, 300);

                    semaphore_p(sem_id);
                    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                    printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getpid(), sharedData->level, sharedData->instance);
                    semaphore_v(sem_id);

                    struct expr_var_list vars = {0};
                    struct expr *e = expr_create(request, strlen(request), &vars, user_funcs);
                    if (e == NULL) {
                        printf("Syntax error");
                        return 1;
                    }

                    float result = expr_eval(e);
                    printf("result: %f\n", result);

                    expr_destroy(e, &vars);
                } else {
                    semaphore_p(sem_id);
                    struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                    sharedData->instance = sharedData->instance + 1;
                    sharedData->level = sharedData->level + 1;
                    semaphore_v(sem_id);

                    write(file_pipes[1], request, 300);
                }
                break;
            }
            case EXTENAL_PROGRAM:{
                semaphore_p(sem_id);
                struct shared_use_st* sharedData = (struct shared_use_st *)sharedMemory;
                printf("Hi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getpid(), sharedData->level, sharedData->instance);
                semaphore_v(sem_id);

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
    printf("Tiempo transcurrido: %f segundos\n", generalTime);
    printf("Tiempo aleatorio de espera: %f segundos\n", timeElasep);
    
    return 0;
}
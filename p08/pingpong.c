#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>
#define STACKSIZE 32768
#define TICKS 20
#define TIME_INTERVAL 1000

int tid = 0;

task_t mainTask;
task_t dispatcher;

task_t* actualTask;
task_t* currentTask;
task_t* next;

unsigned int sysTimerCount;

queue_t* taskQueue = NULL;
queue_t* suspendedQueue = NULL;

struct sigaction action ;

struct itimerval timer;

unsigned int systime () {
    return sysTimerCount;
}

task_t* FCFS() {
    return queue_remove(&taskQueue, taskQueue);
}

task_t* agingPrio() {

    task_t* lowPrio = taskQueue;
    task_t* aux = taskQueue->next;

    do{
        if(lowPrio->dinamicPrio > aux->dinamicPrio) {
            lowPrio = aux;
        }
        aux = aux->next;
    }while(lowPrio != aux);

    aux = lowPrio->next;
    do{
        if(aux != lowPrio){
            aux->dinamicPrio = aux->dinamicPrio - 1;
        }
        aux = aux->next;
    }while(aux != lowPrio);

    lowPrio->dinamicPrio = lowPrio->staticPrio;
    return queue_remove(&taskQueue, lowPrio);

}

task_t* scheduler () {
    return FCFS();
}

void tratador() {
    sysTimerCount++;

    if(currentTask->sysTask) {
        return;
    }
    currentTask->ticks--;
    if(currentTask->ticks <= 0) {
        task_yield();
    }
}

void updateTask(task_t* task, bool isDeleting) {
    if(isDeleting) {
        task->taskExecutionTime = systime() - task->taskInitTime;
        task->taskProcessorTime += (systime() - task->taskStartRunningTime);
        printf("Task %d exit: running time %5d ms, cpu time     %5d ms, %5d activations\n", task->tid, task->taskExecutionTime, task->taskProcessorTime, task->activations);
    } else {
        currentTask->taskProcessorTime += (systime() - currentTask->taskStartRunningTime);

        task->activations++;
        task->taskStartRunningTime = systime();
    }
}

void dispatcher_body () {
    while(queue_size(taskQueue) > 0) {
        next = scheduler();
        if(next) {
            task_switch(next);
        }
        else {
            break;
        }
    }
    updateTask(&dispatcher, true);
    task_switch(&mainTask);
}

void timerInit() {
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0) {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }
    timer.it_value.tv_usec = TIME_INTERVAL ;
    timer.it_value.tv_sec  = 0 ;
    timer.it_interval.tv_usec = TIME_INTERVAL ;
    timer.it_interval.tv_sec  = 0 ;
    if (setitimer (ITIMER_REAL, &timer, 0) < 0) {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
}

void pingpong_init () {
    sysTimerCount = 0;
    timerInit();

    setvbuf (stdout, 0, _IONBF, 0) ;


    currentTask = &mainTask;
    mainTask.activations = 0;
    mainTask.taskInitTime = systime();
    mainTask.taskStartRunningTime = 0;
    mainTask.taskExecutionTime = 0;
    mainTask.taskProcessorTime = 0;
    mainTask.sysTask = false;
    mainTask.tid = tid;
    mainTask.waitingFor = NULL;
    mainTask.dinamicPrio = 0;
    mainTask.staticPrio = 0;
    mainTask.ticks = TICKS;

    tid = tid + 1;

    queue_append(&taskQueue, &mainTask);


    task_create(&dispatcher, dispatcher_body, NULL);
    queue_remove(&taskQueue, &dispatcher);
    dispatcher.sysTask = true;

    task_yield();
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {

    task->activations = 0;
    task->taskInitTime = systime();
    task->taskStartRunningTime = 0;
    task->taskExecutionTime = 0;
    task->taskProcessorTime = 0;

    getcontext (&(task->taskContext));
    char *stack ;
    stack = malloc (STACKSIZE) ;
    if (stack) {
        task->taskContext.uc_stack.ss_sp = stack;
        task->taskContext.uc_stack.ss_size = STACKSIZE;
        task->taskContext.uc_stack.ss_flags = 0;
        task->taskContext.uc_link = 0;
    }
    else {
        return -1;
    }
    makecontext (&(task->taskContext), (void*)(*start_func), 1, arg);

    task->tid = tid;
    task->dinamicPrio = 0;
    task->staticPrio = 0;
    task->sysTask = false;
    task->ticks = TICKS;
    task->waitingFor = NULL;

    tid = tid + 1;

    queue_append(&taskQueue, task);

    return tid;
}

int task_switch (task_t *task) {
    updateTask(task, false);
    if(!currentTask->sysTask) {
        currentTask->ticks = TICKS;
    }
    ucontext_t* auxContext = &currentTask->taskContext;
    currentTask = task;
    swapcontext (auxContext, &task->taskContext);
    return 0;
}

void task_exit (int exitCode) {
    currentTask->exitCode = exitCode;
    updateTask(currentTask, true);

    //na task_exit checar a fila de suspensas e retornar as que estao suspensas pela que esta finalizando


    if(suspendedQueue != NULL) {
        queue_t *aux = suspendedQueue;
        do {
            task_t* auxTask = aux;
            if(auxTask->waitingFor == currentTask) {
                task_t* toRemove = aux;
                toRemove->waitingFor = NULL;
                aux = aux->next;
                queue_append(&taskQueue, queue_remove(&suspendedQueue, toRemove));
            } else {
                aux = aux->next;
            }
        } while (suspendedQueue != aux);
    }

    task_switch(&dispatcher);
}

int task_id () {
    return currentTask->tid;
}

void task_yield () {
    //na task_yield tem que checar se eu estou suspenso para nao me colocar dvolta na fila taskQueue
    if(currentTask->waitingFor == NULL){
        queue_append(&taskQueue, currentTask);
    }
    task_switch(&dispatcher);
}

void task_setprio (task_t *task, int prio) {
    if (prio < -20) {
        prio = -20;
    } else if (prio > 20) {
        prio = 20;
    }
    task->dinamicPrio = prio;
    task->staticPrio = prio;
}

int task_getprio (task_t *task) {
    if(task){
        return task->staticPrio;
    }
    return currentTask->staticPrio;
}

int task_join (task_t *task) {
    bool taskExists = false;

    //checar se task esta na fila de prontas ou de suspensas, senao retorna -1
    if(taskQueue != NULL){
        queue_t *aux = taskQueue;
        do {
            if(aux == task) {
                taskExists = true;
                break;
            }
            aux = aux->next;
        } while (aux != taskQueue);
    }
    if(suspendedQueue != NULL){
        queue_t *aux = suspendedQueue;
        do {
            if(aux == task) {
                taskExists = true;
                break;
            }
            aux = aux->next;
        } while (aux != suspendedQueue);
    }
    if(!taskExists) {
        return -1;
    }
    //currentTask ja esta fora da fila taskQueue, colocar ela na fila de suspensas e chamar task_yield
    queue_append(&suspendedQueue, currentTask);
    currentTask->waitingFor = task;
    task_yield();



    //retornar task->exitCode
    return task->exitCode;
}

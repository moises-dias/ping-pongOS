#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>
#define STACKSIZE 32768
#define TICKS 20
#define TIME_INTERVAL 1000

int tid = 1;

task_t mainTask;
task_t dispatcher;

task_t* actualTask;
task_t* currentTask;
task_t* next;

queue_t* taskQueue;

struct sigaction action ;

struct itimerval timer;

task_t* FCFS() {
    return queue_remove(&taskQueue, taskQueue);
}

//task_t* agingPrio() {
//    task_t* aux = taskQueue;
//    do{
//        if(previousTask == &mainTask) {
//            break;
//        }
//        if(aux != previousTask){
//            aux->dinamicPrio = aux->dinamicPrio - 1;
//        }
//        aux = aux->next;
//    }while(taskQueue != aux);
//
//    aux = firstTask->next;
//    task_t* lowPrio = firstTask;
//
//    do{
//        if(lowPrio->dinamicPrio > aux->dinamicPrio) {
//            lowPrio = aux;
//        }
//        aux = aux->next;
//    }while(firstTask != aux);
//
//    lowPrio->dinamicPrio = lowPrio->staticPrio;
//    return lowPrio;
//
//}

task_t* scheduler () {
    return FCFS();
}

void tratador() {
    if(currentTask->sysTask) {
        return;
    }
    currentTask->ticks--;
    if(currentTask->ticks <= 0) {
        task_yield();
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
    setvbuf (stdout, 0, _IONBF, 0) ;
    currentTask = &mainTask;
    mainTask.sysTask = true;
    task_create(&dispatcher, dispatcher_body, NULL);
    queue_remove(&taskQueue, &dispatcher);
    dispatcher.sysTask = true;

    timerInit();
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {
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

    tid = tid + 1;

    queue_append(&taskQueue, task);

    return tid;
}

int task_switch (task_t *task) {
    if(!currentTask->sysTask) {
        currentTask->ticks = TICKS;
    }
    ucontext_t* auxContext = &currentTask->taskContext;
    currentTask = task;
    swapcontext (auxContext, &task->taskContext);
    return 0;
}

void task_exit (int exitCode) {
    task_switch(&dispatcher);
}

int task_id () {
    return currentTask->tid;
}

void task_yield () {
    if(currentTask != &mainTask){
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
    return actualTask->staticPrio;
}


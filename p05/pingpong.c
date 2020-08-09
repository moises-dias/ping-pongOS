#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"
#include "queue.h"
#include <signal.h>
#include <sys/time.h>
#define STACKSIZE 32768


int tid = 1;

task_t mainTask;
task_t* actualTask;
task_t* previousTask;
task_t* firstTask;
task_t dispatcher;
queue_t* taskQueue;
task_t* next;
struct sigaction action ;
struct itimerval timer;

task_t* scheduler () {
//
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

    if (previousTask == taskQueue) {
        taskQueue = taskQueue->next;
        return taskQueue;
    }
    else {
        return taskQueue;
    }

}

void tratador() {
    if(actualTask->sysTask) {
        return;
    }
    actualTask->ticks = actualTask->ticks - 1;
    if(actualTask->ticks <= 0) {
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

void pingpong_init () {
    setvbuf (stdout, 0, _IONBF, 0) ;
    actualTask = &mainTask;
    task_create(&dispatcher, dispatcher_body, NULL);
    queue_remove(&taskQueue, &dispatcher);
    dispatcher.sysTask = true;

    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0) {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos
    if (setitimer (ITIMER_REAL, &timer, 0) < 0) {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
}

int task_create (task_t *task,			// descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg) {            // argumentos para a tarefa

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
    task->ticks = 20;
    tid = tid + 1;


    queue_append(&taskQueue, task);
    if(queue_size(taskQueue) == 1) {
        firstTask = task;
    }

    return tid;
}

int task_switch (task_t *task) {
    actualTask->ticks = 20;
    ucontext_t* auxContext = &actualTask->taskContext;
    previousTask = actualTask;
    actualTask = task;
    swapcontext (auxContext, &task->taskContext);
    return 0;
}

void task_exit (int exitCode) {
    if(firstTask == actualTask) {
        firstTask = firstTask->next;
    }
    queue_remove(&taskQueue, actualTask);
    task_switch(&dispatcher);
}

int task_id () {
    return actualTask->tid;
}

void task_yield () {
    task_switch(&dispatcher);
}

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) {
    if (prio < -20) {
        prio = -20;
    } else if (prio > 20) {
        prio = 20;
    }
    task->dinamicPrio = prio;
    task->staticPrio = prio;
}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) {
    if(task){
        return task->staticPrio;
    }
    return actualTask->staticPrio;
}


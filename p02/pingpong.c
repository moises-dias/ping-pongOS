#include <stdio.h>
#include <stdlib.h>
#include "pingpong.h"

#define STACKSIZE 32768


int tid = 1;
//ucontext_t mainContext;
//ucontext_t* actualContext;
task_t mainTask;
task_t* actualTask;

void pingpong_init () {
    setvbuf (stdout, 0, _IONBF, 0) ;

//    actualContext = &mainContext;
    actualTask = &mainTask;
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
    tid = tid + 1;
    // printf ("%d\n", tid) ;
    return tid;
}

int task_switch (task_t *task) {

    //printf ("atual %d -- destino %d\n", &(*actualContext), &(task->taskContext)) ;
    //getcontext(&(*actualContext));
    //setcontext(&(task->taskContext));

    ucontext_t* auxContext = &actualTask->taskContext;
//    actualContext = &(task->taskContext);
    actualTask = task;
    //printf ("atual %d -- destino %d\n", auxContext, &(task->taskContext)) ;
    //printf("before\n");
    swapcontext (auxContext, &task->taskContext);
    //printf("after\n");
    return 0;
}

void task_exit (int exitCode) {
    task_switch(&mainTask);
}

int task_id () {
    return actualTask->tid;
}
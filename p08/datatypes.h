// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DAINF UTFPR
// Versão 1.0 -- Março de 2015
//
// Estruturas de dados internas do sistema operacional

#ifndef __DATATYPES__
#define __DATATYPES__

#include <ucontext.h>
#include <stdbool.h>

// Estrutura que define uma tarefa
typedef struct task_t
{
    struct task_t *prev, *next ;
    struct task_t *waitingFor;
    int tid ;
    int staticPrio;
    int dinamicPrio;
    ucontext_t taskContext;
    bool sysTask;
    int ticks;
    int exitCode;
    unsigned int activations;
    unsigned int taskInitTime;
    unsigned int taskStartRunningTime;
    unsigned int taskExecutionTime;
    unsigned int taskProcessorTime;

  // preencher quando necessário
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif

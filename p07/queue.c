#include <stdbool.h>
#include "queue.h"

// Insere um elemento no final da fila.

void queue_append (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        //ERRO: a fila deve existir
        return;
    }
    if(elem == NULL) {
        //ERRO: o elemento deve existir
        return;
    }
    if(elem->next != NULL || elem->prev != NULL) {
        //ERRO: o elemento não deve estar em outra fila
        return;
    }

    if(*queue == NULL) {
        *queue = elem;
        elem->next = elem;
        elem->prev = elem;
        return;
    }

    queue_t* aux = *queue;

    while(aux->next != *queue) {
        aux = aux->next;
    }

    elem->next = *queue;
    elem->prev = aux;
    (*queue)->prev = elem;
    aux->next = elem;

    return;
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.

queue_t *queue_remove (queue_t **queue, queue_t *elem) {
    if(queue == NULL) {
        //ERRO: a fila deve existir
        return NULL;
    }
    if(*queue == NULL) {
        //ERRO: a fila não deve estar vazia
        return NULL;
    }
    if(elem == NULL) {
        //ERRO: o elemento deve existir
        return NULL;
    }

    queue_t *aux = *queue;
    bool elementInQueue = false;
    do {
        if(aux == elem) {
            elementInQueue = true;
            break;
        }
        aux = aux->next;
    } while(aux != *queue);

    if(!elementInQueue) {
        //ERRO: o elemento deve pertencer a fila indicada
        return NULL;
    }

    (elem->next)->prev = elem->prev;
    (elem->prev)->next = elem->next;

    if(elem == *queue) {
        *queue = (*queue)->next;
    }

    if(elem->next == elem) {
        *queue = NULL;
    }

    elem->next = NULL;
    elem->prev = NULL;

    return elem;

}

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue) {

    if(queue == NULL) {
        return 0;
    }
    queue_t *aux = queue;
    int count = 1;
    while(aux->next != queue) {
        aux = aux->next;
        count += 1;
    }
    return count;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    if(queue == NULL) {
        return;
    }
    queue_t *aux = queue;
    do{
        print_elem(aux);
        aux = aux->next;
    } while(aux != queue);
    return;
}



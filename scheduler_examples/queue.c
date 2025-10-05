#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

pcb_t *new_pcb(pid_t pid, uint32_t sockfd, uint32_t time_ms) {
    pcb_t * new_task = malloc(sizeof(pcb_t));

    if (!new_task) return NULL;

    new_task->pid = pid;
    new_task->status = TASK_COMMAND;
    new_task->slice_start_ms = 0;
    new_task->sockfd = sockfd;
    new_task->time_ms = time_ms;
    new_task->ellapsed_time_ms = 0;
    new_task->priority = 0;
    return new_task;
}



int enqueue_pcb(queue_t* q, pcb_t* task) {
    queue_elem_t* elem = malloc(sizeof(queue_elem_t));
    if (!elem) return 0;

    elem->pcb = task;
    elem->next = NULL;

    if (q->tail) {
        q->tail->next = elem;
    } else {
        q->head = elem;
    }
    q->tail = elem;
    return 1;
}

/* Remove e retorna o pcb com menor tempo ms.
 * Retorna NULL se q==NULL ou fila vazia.
 * Liberta o nó da fila (queue_elem_t) mas NÃO libera o pcb_t retornado.
 */
pcb_t* dequeue_shortest_job(queue_t* q) {
    if (!q || !q->head) return NULL;   // fila inválida ou vazia

    // assumimos inicialmente que o head é o melhor candidato
    queue_elem_t *best = q->head;
    queue_elem_t *best_prev = NULL;

    // prev e it para percorrer a lista a partir do segundo nó
    queue_elem_t *prev = q->head;
    queue_elem_t *it   = q->head->next;

    // percorre a lista e encontra o nó cuja pcb->time_ms é o menor
    while (it) {
        if (it->pcb->time_ms < best->pcb->time_ms) {
            best = it;
            best_prev = prev;
        }
        prev = it;
        it   = it->next;
    }

    // remove 'best' da lista encadeada
    if (best_prev) {
        best_prev->next = best->next;     // pula o best
    } else {
        // best era o head
        q->head = best->next;
    }

    // se best era o tail, atualiza tail
    if (best == q->tail) {
        q->tail = best_prev;
    }

    // extrai o pcb do nó, libera o nó e retorna o pcb
    pcb_t *task = best->pcb;
    free(best);
    return task;
}


pcb_t* dequeue_pcb(queue_t* q) {
    if (!q || !q->head) return NULL;

    queue_elem_t* node = q->head;
    pcb_t* task = node->pcb;

    q->head = node->next;
    if (!q->head)
        q->tail = NULL;

    free(node);
    return task;
}

queue_elem_t *remove_queue_elem(queue_t* q, queue_elem_t* elem) {
    queue_elem_t* it = q->head;
    queue_elem_t* prev = NULL;
    while (it != NULL) {
        if (it == elem) {
            // Remove elem from queue
            if (prev) {
                prev->next = it->next;
            } else {
                q->head = it->next;
            }
            if (it == q->tail) {
                q->tail = prev;
            }
            return it;
        }
        prev = it;
        it = it->next;
    }
    printf("Queue element not found in queue\n");
    return NULL;
}
/**
 * @brief Move um PCB da blocked queue para outra fila (por exemplo, ready queue do MLFQ).
 *
 * Esta função assume que o PCB já está na blocked queue e deve ser removido
 * quando o tempo de bloqueio termina.
 */
void move_from_blocked_to_ready(queue_t *blocked_q, queue_t *dest_q, pcb_t *pcb) {
    if (!blocked_q || !dest_q || !pcb) return;

    // percorre a blocked queue para encontrar e remover o PCB
    queue_elem_t *it = blocked_q->head;
    queue_elem_t *prev = NULL;

    while (it) {
        if (it->pcb == pcb) {
            // remove da blocked queue
            if (prev)
                prev->next = it->next;
            else
                blocked_q->head = it->next;

            if (it == blocked_q->tail)
                blocked_q->tail = prev;

            free(it); // liberta o nó, mas não o PCB
            break;
        }
        prev = it;
        it = it->next;
    }
    enqueue_pcb(dest_q, pcb);
}

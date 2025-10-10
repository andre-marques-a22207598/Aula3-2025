#include "mlfq.h"
#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

//#define ALL_UP 5000 //tempo para todos subirem para prioridade máxima

static int mlfq_inicializado = 0;

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, queue_t *command_queue, pcb_t **cpu_task) {
    static mlfq_t mlfq;
    if (!mlfq_inicializado) {
        for (int i = 0; i < MLFQ_LEVELS; ++i) {
            mlfq.queues[i].head = NULL;
            mlfq.queues[i].tail = NULL;
        }
        mlfq_inicializado = 1;
    }

    // Move todos os processos da ready queue (rp) para o nível 0 do MLFQ
    while (rq->head != NULL) {
        pcb_t *pcb = dequeue_pcb(rq);
        pcb->priority = 0;  // nível mais alto
        enqueue_pcb(&mlfq.queues[0], pcb);
    }

    //se há algum processo a correr
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Task finished
            // Send msg to application
            msg_t msg = {
                    .pid = (*cpu_task)->pid,
                    .request = PROCESS_REQUEST_DONE,
                    .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            
            enqueue_pcb(command_queue, *cpu_task);
            (*cpu_task) = NULL;

        } else if (current_time_ms - (*cpu_task)->slice_start_ms >= TIME_SLICE) {
            int nivel = (*cpu_task)->priority;
            if (nivel < MLFQ_LEVELS -1) {
                nivel++;
            }

            (*cpu_task)->priority = nivel;

            printf("[MLFQ] Process %d quantum expired, demoted to level %d\n",
                   (*cpu_task)->pid, nivel);

            enqueue_pcb(&mlfq.queues[nivel], *cpu_task);
            *cpu_task = NULL;
        }
    }

    if (*cpu_task == NULL) {
        for (int nivel = 0; nivel < MLFQ_LEVELS; nivel++) {
            if (mlfq.queues[nivel].head != NULL) {
                *cpu_task = dequeue_pcb(&mlfq.queues[nivel]);
                if (*cpu_task) {
                    (*cpu_task)->slice_start_ms = current_time_ms;
                    printf("[MLFQ] Process %d starts running at %u ms (level %d)\n",
                           (*cpu_task)->pid, current_time_ms, nivel);
                }
                break; // pega apenas um processo
            }
        }
    }

}

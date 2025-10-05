#include "rr.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Se há um processo a correr
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;  // Incrementa tempo de CPU usado
        // Processo terminou
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            printf("[RR] Process %d finished at %u ms, CPU used: %u ms\n",
                   (*cpu_task)->pid, current_time_ms, (*cpu_task)->ellapsed_time_ms);

            msg_t msg = {
                    .pid = (*cpu_task)->pid,
                    .request = PROCESS_REQUEST_DONE,
                    .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            free(*cpu_task);
            *cpu_task = NULL;
        }
            // Quantum expirou
        else if (current_time_ms - (*cpu_task)->slice_start_ms >= TIME_SLICE) {
            printf("[RR] Process %d quantum ended at %u ms, preempting and moving to end of queue\n",
                   (*cpu_task)->pid, current_time_ms);

            // Envia processo para o fim da fila
            enqueue_pcb(rq, *cpu_task);
            *cpu_task = NULL;  // CPU fica livre
        }
    }

    // Se CPU está livre, pega próximo processo da fila
    if (*cpu_task == NULL) {
        *cpu_task = dequeue_pcb(rq);
        if (*cpu_task) {
            (*cpu_task)->slice_start_ms = current_time_ms;  // reinicia quantum
            printf("[RR] Process %d starts running at %u ms\n",
                   (*cpu_task)->pid, current_time_ms);
        }
    }
}

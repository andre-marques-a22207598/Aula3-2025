#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"
#define MLFQ_LEVELS 8

//a ordem importa!!

typedef struct {
    queue_t queues[MLFQ_LEVELS];
} mlfq_t;

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, queue_t *blocked_q, pcb_t **cpu_task);

#endif

#include "header.h"

queue* initQueue(){
    queue* _queue = calloc(1, sizeof(queue));
    return _queue;
}

node createQueueCell(qtee tee, int depth){
    node cell = calloc(1, sizeof(Node));
    if (cell){
        cell->tee = tee;
        cell->depth = depth;
    }

    return cell;
}

void addToQueue(queue *_queue, qtee tee, int depth){
    node cell = createQueueCell(tee, depth);
    if (!(*_queue).head)
        (*_queue).head = cell;
    if ((*_queue).tail)
        (*_queue).tail->next = cell;
    (*_queue).tail = cell;
}

node removeFromQueue(queue *_queue){
    node head = _queue->head;
    _queue->head = head->next;
    if (!_queue->head)
        _queue->tail = NULL;
    
    return head;
}

void clearQueue(queue *_queue){
    node cell = (*_queue).head;
    while (cell) {
        node op = cell;
        cell = cell->next;
        free(op);
    }
    free(_queue);
}

int isQueueEmpty(queue *queue){
    int result = 0;
    if (!queue || !queue->head)
        result = 1;
    
    return result;
}
#include "task.h"


TaskQueue empty_task_queue()
{
    TaskQueue queue = {
        .head = NULL,
        .tail = NULL
    };
    return queue;
}

bool is_empty(TaskQueue queue)
{
    return queue.head == NULL;
}

void push_task(TaskQueue* queue, Edge task)
{
    TaskQueueItem* queue_item = malloc(sizeof(TaskQueueItem));
    queue_item->task = task;
    queue_item->next = NULL;

    if (is_empty(*queue)) {
        queue->head = queue->tail = queue_item;
    }
    else {
        assert(queue->tail->next == NULL);
        queue->tail->next = queue_item;
        queue->tail = queue_item;
    }
}

Edge pop_task(TaskQueue* queue)
{
    assert(!is_empty(*queue));

    TaskQueueItem queue_item = *queue->head;
    free(queue->head);
    queue->head = queue_item.next;

    return queue_item.task;
}

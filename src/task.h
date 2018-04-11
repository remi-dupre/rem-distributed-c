#include <assert.h>
#include <stdbool.h>

#include "graph.h"


/**
 * Represent a single item in the queue.
 */
typedef struct TaskQueueItem
{
    Edge task;            // value of this item
    struct TaskQueueItem* next;  // next item of the queue
} TaskQueueItem;

/**
 * Represent a queue, items or placed in separate sections of memory.
 */
typedef struct TaskQueue
{
    TaskQueueItem* head;
    TaskQueueItem* tail;
} TaskQueue;


/**
 * Create a new empty queue of tasks.
 */
TaskQueue empty_task_queue();

/**
 * Check if a queue is empty.
 */
bool is_empty(TaskQueue queue);

/**
 * Add an element to the queue.
 */
void push_task(TaskQueue* queue, Edge task);

/**
 * Pop an element from the queue.
 */
Edge pop_task(TaskQueue* queue);

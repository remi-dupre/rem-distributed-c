#include <assert.h>
#include <stdbool.h>

#include "graph.h"


typedef struct TaskHeap
{
    Edge* tasks;
    size_t nb_tasks;
    size_t container_size;
} TaskHeap;


/**
 * Create a new empty task heap.
 */
TaskHeap empty_task_heap();

/**
 * Check if the heap is empty.
 */
#define is_empty_heap(heap) ((heap).nb_tasks == 0)

/**
 * Add an element to the heap.
 */
void push_task(TaskHeap* heap, Edge task);

/**
 * Pop an element from the heap.
 */
Edge pop_task(TaskHeap* heap);

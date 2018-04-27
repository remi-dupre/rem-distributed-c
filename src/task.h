#ifndef TASK
#define TASK

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
 * Push a list of tasks to the heap.
 */
void push_tasks(TaskHeap* heap, Edge* tasks, int nb_tasks);

/**
 * Include inline functions:
 *   bool is_empty_heap(TaskHeap heap)
 *   Edge pop_task(TaskHeap* heap)
 */
#include "task.inl"

#endif

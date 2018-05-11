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
 * Include inline functions:
 *   bool is_empty_heap(TaskHeap heap)
 *   Edge pop_task(TaskHeap* heap)
 *   void push_tasks(TaskHeap* heap, Edge* tasks, int nb_tasks)
 *   void push_task(TaskHeap* heap, Edge tasks)
 */
#include "task.inl"

#endif

#include "task.h"


#define INIT_SIZE 10000  // Initial container size of a task heap


TaskHeap empty_task_heap()
{
    TaskHeap heap = {
        .tasks = malloc(INIT_SIZE * sizeof(Edge)),
        .nb_tasks = 0,
        .container_size = INIT_SIZE
    };
    return heap;
}

void push_task(TaskHeap* heap, Edge task)
{
    if (heap->container_size == heap->nb_tasks) {
        Edge* new_container = malloc(heap->container_size * 2 * sizeof(Edge));
        memcpy(new_container, heap->tasks, heap->container_size * sizeof(Edge));

        free(heap->tasks);
        heap->tasks = new_container;
        heap->container_size *= 2;
    }

    heap->tasks[heap->nb_tasks] = task;
    heap->nb_tasks++;
}

Edge pop_task(TaskHeap* heap)
{
    assert(!is_empty_heap(*heap));

    heap->nb_tasks--;
    return heap->tasks[heap->nb_tasks];
}

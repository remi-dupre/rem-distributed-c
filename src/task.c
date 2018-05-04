#include "task.h"


TaskHeap empty_task_heap()
{
    TaskHeap heap = {
        .tasks = NULL,
        .nb_tasks = 0,
        .container_size = 0
    };
    return heap;
}

void push_tasks(TaskHeap* heap, Edge* tasks, int nb_tasks)
{
    if (heap->container_size < heap->nb_tasks + nb_tasks) {
        while (heap->container_size < heap->nb_tasks + nb_tasks)
            heap->container_size = 2 * heap->container_size + 1;

        heap->tasks = realloc(heap->tasks, heap->container_size * sizeof(Edge));
    }

    memcpy(heap->tasks + heap->nb_tasks, tasks, nb_tasks * sizeof(Edge));
    heap->nb_tasks += nb_tasks;
}

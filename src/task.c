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

void push_tasks(TaskHeap* heap, Edge* tasks, int nb_tasks)
{
    if (heap->container_size < heap->nb_tasks + nb_tasks) {
        while (heap->container_size < heap->nb_tasks + nb_tasks)
            heap->container_size *= 2;

        Edge* new_container = malloc(heap->container_size * sizeof(Edge));
        memcpy(new_container, heap->tasks, heap->nb_tasks * sizeof(Edge));

        free(heap->tasks);
        heap->tasks = new_container;
    }

    memcpy(heap->tasks + heap->nb_tasks, tasks, nb_tasks * sizeof(Edge));
    heap->nb_tasks += nb_tasks;
}

/**
 * Implementation of inline functions.
 */

static inline bool is_empty_heap(TaskHeap heap)
{
    return heap.nb_tasks == 0;
}

static inline Edge pop_task(TaskHeap* heap)
{
    assert(!is_empty_heap(*heap));

    heap->nb_tasks--;
    return heap->tasks[heap->nb_tasks];
}

static inline void push_tasks(TaskHeap* heap, Edge* tasks, int nb_tasks)
{
    if (heap->container_size < heap->nb_tasks + nb_tasks) {
        while (heap->container_size < heap->nb_tasks + nb_tasks)
            heap->container_size = 2 * heap->container_size + 1;

        heap->tasks = realloc(heap->tasks, heap->container_size * sizeof(Edge));

        if(heap->tasks == NULL) {
            perror("Not enough memory available (pushng task)");
            exit(-1);
        }
    }

    memcpy(heap->tasks + heap->nb_tasks, tasks, nb_tasks * sizeof(Edge));
    heap->nb_tasks += nb_tasks;
}

static inline void push_task(TaskHeap* heap, Edge task)
{
    push_tasks(heap, &task, 1);
}

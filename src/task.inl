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

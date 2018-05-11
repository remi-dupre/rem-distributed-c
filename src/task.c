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

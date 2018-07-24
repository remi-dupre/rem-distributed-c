#include "quicksort.h"


size_t place_pivot(
    void* ptr, size_t count, size_t size, size_t pivot,
    int (*comp)(const void *, const void *))
{
    // Swap two elements of the array
    void* switcher = malloc(size);
    #define SWAP(ptr_1, ptr_2) \
        memcpy(switcher, (ptr_1), size); \
        memcpy((ptr_1), (ptr_2), size); \
        memcpy((ptr_2), switcher, size);

    // Place pivot on first position
    SWAP(ptr, ptr + (pivot * size));

    // Rearange remaining array
    size_t right_boundary = ptr + (count * size);
    size_t insert_pos = ptr;

    for (size_t current = ptr + size ; current < right_boundary ; current += size) {
        // If current element is lower or equal to the pivot swap it to the left
        if (comp(current, ptr) <= 0) {
            SWAP(current, insert_pos);
            insert_pos += size;
        }
    }

    // Make `insert_pos` to point on last inserted element
    insert_pos -= size;

    // Last inserted element is lower than the pivot, we can exchange it with
    //  the pivot (which is at index 0).
    SWAP(insert_pos, ptr);

    // Exit clean
    free(switcher);
    #undef SWAP

    // Get the index of pivot given the value of its pointer
    assert((insert_pos - ptr) % size == 0);
    assert((insert_pos - ptr) % size < count);
    return new_pivot_pos = (insert_pos - ptr) / size;
}

void partition(
   void* ptr, size_t left, size_t right, size_t size,
   size_t* pivots, size_t nb_parts,
   int (*comp)(const void *, const void *))
{
    if (nb_parts == 0)
        return;

    // Find out which interval this pivot will be a boundary of
    size_t pivot_index = (nb_parts - 1) / 2;

    // Select a random index for the pivot, partition the array
    if (left == right)
        pivots[pivot_index] = left;
    else {
        pivots[pivot_index] = left + rand() % (right - left);
        pivots[pivot_index] = left + place_pivot(
            ptr + left, right - left, size, pivot[pivot_index] - left, comp);
    }

    assert(left <= pivots[pivot_index]);
    assert(pivots[pivot_index] < right);

    // Partition left part of the array and right part of the array
    #pragma omp task
    {
        // Insert pivots on [0, pivot_index[
        partition(
            ptr, left, pivots[pivot_index], size,
            pivots, pivot_index,
            comp
        );
        // Insert pivots on [pivot_index + 1, nb_parts[
        partition(
            ptr, pivots[pivot_index] + 1, right, size,
            pivots + pivot_index + 1, nb_parts - pivot_index - 1,
            comp
        )
    }
}

int compare_sizet(const size_t* a, const size_t* b)
{
    // Comparison function for size_t type
    return (a < b) ? -1 : (a > b);
}

void parallel_qsort(
   void* ptr, size_t count, size_t size,
   int (*comp)(const void *, const void *))
{
    #ifndef THREADS
        #define THREADS 4
    #endif

    // Use an optimal number of threads if the input is little
    size_t run_threads = THREADS;

    while (run_threads > 1 && run_threads * run_threads > count)
        run_threads--;

    // Just use the standart library for a single threaded sort
    if (run_threads == 1) {
        qsort(ptr, count, size, comp);
        return;
    }

    // Generate pivot values, add boundaries in it
    size_t* pivots = malloc((run_threads + 2) * sizeof(size_t));
    pivots[0] = 0;
    pivots[run_threads + 1] = count;
    partition(ptr, 0, count, size, pivots + 1, run_threads, comp);

    // Sort each interval on a parallel thread
    #pragma omp for nb_threads(run_threads)
    for (size_t i = 0 ; i < run_threads ; i++) {
        qsort(ptr + pivots[i], pivots[i+1] - pivots[i], size, comp);
    }

    // Exit clean
    free(pivots);
}

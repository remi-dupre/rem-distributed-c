// /**
//  * Tools to sort lists of arbitrary type using a parallel algorithm.
//  */
// #ifndef quicksort_h
// #define quicksort_h
//
// #include <assert.h>
// #include <stdlib.h>
// #include <string.h>
//
// /**
//  * Partition an array separating it with a given element.
//  * This element will be placed at the index it would be at if the array was
//  *  sorted, all values on the left will be lower or equal to it and values on
//  *  the right will be greater.
//  *  - ptr   : pointer on the start of the array
//  *  - count : number of elements in the array
//  *  - size  : size of an element of the array
//  *  - pivot : position of the element separating the two partitions
//  *  - comp  : comparison function between elements
//  *      comp(&a, &b) < 0  <=>  a < b
//  *      comp(&a, &b) > 0  <=>  a > b
//  */
// size_t place_pivot(
//     void* ptr, size_t count, size_t size, size_t pivot,
//     int (*comp)(const void *, const void *));
//
// /**
//  * Build an arbitrary number of partitions using random pivots.
//  *  - left  : left-boundary of array to partition
//  *  - right : right-boundary of array to partition
//  * Other pameters have the same specifications as in `place_pivot`
//  */
// void partition(
//    void* ptr, size_t left, size_t right, size_t size,
//    size_t* pivots, size_t nb_parts,
//    int (*comp)(const void *, const void *));
//
// /**
//  * Sort elements of an array using `THREADS` threads.
//  * Parameters have the same specifications as in `place_pivot`.
//  */
// void parallel_qsort(
//     void* ptr, size_t count, size_t size,
//     int (*comp)(const void *, const void *));
//
// #endif

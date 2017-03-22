/*
* Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
*/

#include <stdio.h>
#include <stdlib.h>
#if defined(__MACH__) && !defined(CLOCK_REALTIME)
#include <sys/time.h>

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1

int clock_gettime(int clk_id, struct timespec* t) {
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) return rv;
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#else
#include <time.h>
#endif

#include "skiplist_with_rank.h"

#define N 1024 * 1024 * 2
//#define SKIPLIST_DEBUG

int
main(void)
{
    int i;
    struct timespec start, end;

    int *key = (int *)malloc(N * sizeof(int));
    if (key == NULL) {
        exit(-1);
    }

    struct skiplist *list = skiplist_new();
    if (list == NULL) {
        exit(-1);
    }

    printf("Test start!\n");
    printf("Add %d nodes...\n", N);

    /* Insert test */
    srandom(time(NULL));
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < N; i++) {
        int value = key[i] = (int)random();
        skiplist_insert(list, key[i], value);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);
    #ifdef SKIPLIST_DEBUG
    skiplist_dump(list);
    #endif

    /* Search test 1 */
    printf("Now search each node by key...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < N; i++) {
        struct skipnode *node = skiplist_search_by_key(list, key[i]);
        if (node != NULL) {
            #ifdef SKIPLIST_DEBUG
            printf("key:0x%08x value:0x%08x\n", node->key, node->value);
            #endif
        } else {
            printf("Not found:0x%08x\n", key[i]);
        }
        #ifdef SKIPLIST_DEBUG
        printf("key rank:%d\n", skiplist_key_rank(list, key[i]));
        #else
        //skiplist_key_rank(list, key[i]);
        #endif
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);

    /* Search test 2 */
    printf("Now search each node by rank...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < N; i++) {
        struct skipnode *node = skiplist_search_by_rank(list, i + 1);
        if (node != NULL) {
            #ifdef SKIPLIST_DEBUG
            printf("rank:%d value:0x%08x\n", i + 1, node->value);
            #endif
        } else {
            printf("Not found:%d\n", i + 1);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);

    /* Delete test */
    printf("Now remove all nodes...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < N; i++) {
        skiplist_remove(list, key[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("time span: %ldms\n", (end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);
    #ifdef SKIPLIST_DEBUG
    skiplist_dump(list);
    #endif

    printf("End of Test.\n");
    skiplist_delete(list);

    free(key);

    return 0;
}

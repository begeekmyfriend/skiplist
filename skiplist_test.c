/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "skiplist.h"

#define N 2 * 1024 * 1024
// #define SKIPLIST_DEBUG

int
main(void)
{
        int i;
        struct timespec start, end;

	int *key = malloc(N * sizeof(int));
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

        /* Search test */
        printf("Now search each node...\n");
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (i = 0; i < N; i++) {
                struct skipnode *node = skiplist_search(list, key[i]);
                if (node != NULL) {
#ifdef SKIPLIST_DEBUG
                        printf("key:0x%08x value:0x%08x\n", node->key, node->value);
#endif
                } else {
                        printf("Not found:0x%08x\n", key[i]);
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

        return 0;
}

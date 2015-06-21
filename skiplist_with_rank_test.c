/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include "skiplist_with_rank.h"

#define N 1024 * 1024

int
main(void)
{
        int i;

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
        for (i = 0; i < N; i++) {
                int value = key[i] = (int)random();
                skiplist_add(list, key[i], value);
        }
#ifdef SKIPLIST_DEBUG
        skiplist_dump(list);
#endif

        /* Search test 1 */
        printf("Now search each node by key...\n");
        for (i = 0; i < N; i++) {
                int value = skiplist_search_by_key(list, key[i]);
                if (value != -1) {
#ifdef SKIPLIST_DEBUG
                        printf("key:0x%08x value:0x%08x\n", key[i], value);
#endif
                } else {
                        printf("Not found:0x%08x\n", key[i]);
                }
#ifdef SKIPLIST_DEBUG
                printf("key rank:%d\n", skiplist_key_rank(list, key[i]));
#else
                skiplist_key_rank(list, key[i]);
#endif
        }

        /* Search test 2 */
        printf("Now search each node by rank...\n");
        for (i = 0; i < N; i++) {
                int value = skiplist_search_by_rank(list, i + 1);
                if (value != -1) {
#ifdef SKIPLIST_DEBUG
                        printf("rank:%d value:0x%08x\n", i + 1, value);
#endif
                } else {
                        printf("Not found:%d\n", i + 1);
                }
        }

        /* Delete test */
        printf("Now remove all nodes...\n");
        for (i = 0; i < N; i++) {
                skiplist_remove(list, key[i]);
        }
#ifdef SKIPLIST_DEBUG
        skiplist_dump(list);
#endif

        printf("End of Test.\n");
        skiplist_delete(list);

        return 0;        
}

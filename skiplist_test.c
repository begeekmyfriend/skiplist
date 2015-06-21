/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include "skiplist.h"

#define N 1024 * 1024

int
main(void)
{
        int i;

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
        for (i = 0; i < N; i++) {
                int value = key[i] = (int)random();
                skiplist_add(list, key[i], value);
        }
#ifdef SKIPLIST_DEBUG
        skiplist_dump(list);
#endif

        /* Search test */
        printf("Now search each node...\n");
        for (i = 0; i < N; i++) {
                int value = skiplist_search(list, key[i]);
                if (value != -1) {
#ifdef SKIPLIST_DEBUG
                        printf("key:0x%08x value:0x%08x\n", key[i], value);
#endif
                } else {
                        printf("Not found:0x%08x\n", key[i]);
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

/*
* Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
*/

#ifndef _SKIPLIST_H
#define _SKIPLIST_H

struct sk_link {
    struct sk_link *next, *prev;
    unsigned int span;
};

static inline void
list_init(struct sk_link *link)
{
    link->prev = link;
    link->next = link;
}

static inline void
__list_add(struct sk_link *link, struct sk_link *prev, struct sk_link *next)
{
    link->next = next;
    link->prev = prev;
    next->prev = link;
    prev->next = link;
}

static inline void
__list_del(struct sk_link *prev, struct sk_link *next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void
list_add(struct sk_link *link, struct sk_link *next)
{
    __list_add(link, next->prev, next);
}

static inline void
list_del(struct sk_link *link)
{
    __list_del(link->prev, link->next);
    list_init(link);
}

static inline int
list_empty(struct sk_link *link)
{
    return link->next == link;
}

#define list_entry(ptr, type, member) \
((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

#define skiplist_foreach_forward(pos, end) \
for (; pos != end; pos = pos->next)

#define skiplist_foreach_forward_safe(pos, n, end) \
for (n = pos->next; pos != end; pos = n, n = pos->next)

#define skiplist_foreach_backward(pos, end) \
for (; pos != end; pos = pos->prev)

#define skiplist_foreach_backward_safe(pos, n, end) \
for (n = pos->prev; pos != end; pos = n, n = pos->prev)

#define MAX_LEVEL 32  /* Should be enough for 2^32 elements */

struct range_spec {
    int min, max;
    int minex, maxex;
};

struct skiplist {
    int level;
    int count;
    struct sk_link head[MAX_LEVEL];
};

struct skipnode {
    int key;
    int value;
    struct sk_link link[0];
};

static struct skipnode *
skipnode_new(int level, int key, int value)
{
    struct skipnode *node = malloc(sizeof(*node) + level * sizeof(struct sk_link));
    if (node != NULL) {
        node->key = key;
        node->value = value;
    }
    return node;
}

static void
skipnode_delete(struct skipnode *node)
{
    free(node);
}

static struct skiplist *
skiplist_new(void)
{
    int i;
    struct skiplist *list = malloc(sizeof(*list));
    if (list != NULL) {
        list->level = 1;
        list->count = 0;
        for (i = 0; i < sizeof(list->head) / sizeof(list->head[0]); i++) {
            list_init(&list->head[i]);
            list->head[i].span = 0;
        }
    }
    return list;
}

static void
skiplist_delete(struct skiplist *list)
{
    struct sk_link *n;
    struct sk_link *pos = list->head[0].next;
    skiplist_foreach_forward_safe(pos, n, &list->head[0]) {
        struct skipnode *node = list_entry(pos, struct skipnode, link[0]);
        skipnode_delete(node);
    }
    free(list);
}

static int
random_level(void)
{
    const double p = 0.25;
    int level = 1;
    while ((random() & 0xffff) < 0xffff * p) {
        level++;
    }
    return level > MAX_LEVEL ? MAX_LEVEL : level;
}

static struct skipnode *
skiplist_insert(struct skiplist *list, int key, int value)
{
    int rank[MAX_LEVEL];
    struct sk_link *update[MAX_LEVEL];

    int level = random_level();
    if (level > list->level) {
        list->level = level;
    }

    struct skipnode *node = skipnode_new(level, key, value);
    if (node != NULL) {
        int i = list->level - 1;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];

        for (; i >= 0; i--) {
            rank[i] = i == list->level - 1 ? 0 : rank[i + 1];
            pos = pos->next;
            skiplist_foreach_forward(pos, end) {
                struct skipnode *nd = list_entry(pos, struct skipnode, link[i]);
                if (nd->key >= key) {
                    end = &nd->link[i];
                    break;
                }
                rank[i] += nd->link[i].span;
            }

            update[i] = end;
            pos = end->prev;
            pos--;
            end--;
        }

        for (i = 0; i < list->level; i++) {
            if (i < level) {
                list_add(&node->link[i], update[i]);
                node->link[i].span = rank[0] - rank[i] + 1;
                update[i]->span -= node->link[i].span - 1;
            } else {
                update[i]->span++;
            }
        }

        list->count++;
    }

    return node;
}

static void
__remove(struct skiplist *list, struct skipnode *node, int level, struct sk_link **update)
{
    int i;
    int remain_level = list->level;
    for (i = 0; i < list->level; i++) {
        if (i < level) {
            list_del(&node->link[i]);
            update[i] = node->link[i].next;
            update[i]->span += node->link[i].span - 1;
        } else {
            update[i]->span--;
        }

        if (list_empty(&list->head[i])) {
            if (remain_level == list->level) {
                remain_level = i + 1;
            }
        }
    }

    skipnode_delete(node);
    list->count--;
    list->level = remain_level;
}

static void
skiplist_remove(struct skiplist *list, int key)
{
    int i;
    struct skipnode *node;
    struct sk_link *pos, *n, *end, *update[MAX_LEVEL];

    i = list->level - 1;
    pos = &list->head[i];
    end = &list->head[i];

    for (; i >= 0; i--) {
        pos = pos->next;
        skiplist_foreach_forward_safe(pos, n, end) {
            node = list_entry(pos, struct skipnode, link[i]);
            if (node->key > key) {
                end = &node->link[i];
                break;
            } else if (node->key == key) {
                __remove(list, node, i + 1, update);
                return;
            }
        }
        update[i] = end;
        pos = end->prev;
        pos--;
        end--;
    }
}

static int
key_gte_min(int key, struct range_spec *range)
{
    return range->maxex ? (key > range->max) : (key >= range->max);
}

static int
key_lte_max(int key, struct range_spec *range)
{
    return range->minex ? (key < range->min) : (key <= range->min);
}

/* Returns if there is node key in range */
static int
key_in_range(struct skiplist *list, struct range_spec *range)
{
    struct skipnode *node;
    struct sk_link *link;

    if (range->min > range->max ||
        (range->min == range->max && (range->minex || range->maxex)))
        return 0;

        if (list_empty(&list->head[0]))
        return 0;

        link = list->head[0].next;
        node = list_entry(link, struct skipnode, link[0]);
        if (!key_lte_max(node->key, range))
        return 0;

        link = list->head[0].prev;
        node = list_entry(link, struct skipnode, link[0]);
        if (!key_gte_min(node->key, range))
        return 0;

        return 1;
    }


    /* search the first node key that is contained in the specified range
    * where min and max are inclusive. */
    static struct skipnode *
    first_in_range(struct skiplist *list, struct range_spec *range)
    {
        if (!key_in_range(list, range))
        return NULL;

        int i = list->level - 1;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];
        struct skipnode *node = NULL;

        for (; i >= 0; i--) {
            pos = pos->next;
            skiplist_foreach_forward(pos, end) {
                node = list_entry(pos, struct skipnode, link[i]);
                if (key_gte_min(node->key, range)) {
                    pos = node->link[i].prev;
                    end = node->link[i].next;
                    goto CONTINUE;
                }
            }
            pos = end->prev;
            CONTINUE:
            pos--;
            end--;
        }

        return node;
    }

    /* search the last node key that is contained in the specified range
    * where min and max are inclusive. */
    static struct skipnode *
    last_in_range(struct skiplist *list, struct range_spec *range)
    {
        if (!key_in_range(list, range))
        return NULL;

        int i = list->level - 1;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];
        struct skipnode *node = NULL;

        for (; i >= 0; i--) {
            pos = pos->prev;
            skiplist_foreach_backward(pos, end) {
                node = list_entry(pos, struct skipnode, link[i]);
                if (key_lte_max(node->key, range)) {
                    pos = node->link[i].next;
                    end = node->link[i].prev;
                    goto CONTINUE;
                }
            }
            pos = end->next;
            CONTINUE:
            pos--;
            end--;
        }

        return node;
    }

    /* remove all the nodes with key in range
    * where min and max are inclusive. */
    static unsigned int
    remove_in_range(struct skiplist *list, struct range_spec *range)
    {
        int i;
        unsigned int removed = 0;
        struct sk_link *pos, *n, *end, *update[MAX_LEVEL];

        if (key_in_range(list, range))
        return 0;

        i = list->level - 1;
        pos = &list->head[i];
        end = &list->head[i];

        for (; i >= 0; i--) {
            pos = pos->next;
            skiplist_foreach_forward_safe(pos, n, end) {
                struct skipnode *node = list_entry(pos, struct skipnode, link[i]);
                if (!key_lte_max(node->key, range)) {
                    end = &node->link[i];
                    break;
                } else if (key_gte_min(node->key, range)) {
                    /* No return because we allow nodes with same key. */
                    __remove(list, node, i + 1, update);
                    removed++;
                }
            }
            update[i] = end;
            pos = end->prev;
            pos--;
            end--;
        }

        return removed;
    }

    /* remove all the nodes with key rank in range
    * where start and stop are inclusive. */
    static unsigned int
    remove_in_rank(struct skiplist *list, unsigned int start, unsigned int stop)
    {
        int i;
        unsigned int removed = 0, traversed = 0;
        struct sk_link *pos, *n, *end, *update[MAX_LEVEL];

        if (start <= 0 || stop <= 0 || start > list->count)
        return 0;

        i = list->level - 1;
        pos = &list->head[i];
        end = &list->head[i];

        for (; i >= 0; i--) {
            pos = pos->next;
            skiplist_foreach_forward_safe(pos, n, end) {
                struct skipnode *node = list_entry(pos, struct skipnode, link[i]);
                if (traversed + node->link[i].span > stop) {
                    end = &node->link[i];
                    break;
                } else if (traversed + node->link[i].span >= start) {
                    /* No return because we allow nodes with same key. */
                    __remove(list, node, i + 1, update);
                    removed++;
                    continue;
                }
                traversed += node->link[i].span;
            }
            update[i] = end;
            pos = end->prev;
            pos--;
            end--;
        }

        return removed;
    }

    /* Get the node key rank */
    static unsigned int
    skiplist_key_rank(struct skiplist *list, int key)
    {
        unsigned int rank = 0;
        int i = list->level - 1;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];
        struct skipnode *node;

        for (; i >= 0; i--) {
            pos = pos->next;
            skiplist_foreach_forward(pos, end) {
                node = list_entry(pos, struct skipnode, link[i]);
                if (node->key >= key) {
                    end = &node->link[i];
                    break;
                }
                rank += node->link[i].span;
            }
            if (node->key == key) {
                return rank + node->link[i].span;
            }
            pos = end->prev;
            pos--;
            end--;
        }

        return 0;
    }

    /* search the node with specified key. */
    static struct skipnode *
    skiplist_search_by_key(struct skiplist *list, int key)
    {
        int i = list->level - 1;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];
        struct skipnode *node;

        for (; i >= 0; i--) {
            pos = pos->next;
            skiplist_foreach_forward(pos, end) {
                node = list_entry(pos, struct skipnode, link[i]);
                if (node->key >= key) {
                    end = &node->link[i];
                    break;
                }
            }
            if (node->key == key) {
                return node;
            }
            pos = end->prev;
            pos--;
            end--;
        }

        return NULL;
    }

    /* search the node with specified key rank. */
    static struct skipnode *
    skiplist_search_by_rank(struct skiplist *list, unsigned int rank)
    {
        if (rank == 0 || rank > list->count) {
            return NULL;
        }

        int i = list->level - 1;
        unsigned int traversed = 0;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];
        struct skipnode *node;

        for (; i >= 0; i--) {
            pos = pos->next;
            skiplist_foreach_forward(pos, end) {
                node = list_entry(pos, struct skipnode, link[i]);
                if (traversed + node->link[i].span > rank) {
                    end = &node->link[i];
                    break;
                }
                traversed += node->link[i].span;
            }
            if (rank == traversed) {
                return node;
            }
            pos = end->prev;
            pos--;
            end--;
        }

        return NULL;
    }

    static void
    skiplist_dump(struct skiplist *list)
    {
        int i = list->level - 1;
        unsigned int traversed = 0;
        struct sk_link *pos = &list->head[i];
        struct sk_link *end = &list->head[i];

        printf("\nTotal %d nodes: \n", list->count);
        for (; i >= 0; i--) {
            traversed = 0;
            pos = pos->next;
            skiplist_foreach_forward(pos, end) {
                struct skipnode *node = list_entry(pos, struct skipnode, link[i]);
                traversed += node->link[i].span;
                printf("level:%d key:0x%08x value:0x%08x rank:%u\n",
                i + 1, node->key, node->value, traversed);
            }
            pos = &list->head[i];
            pos--;
            end--;
        }
    }

    #endif  /* _SKIPLIST_H */

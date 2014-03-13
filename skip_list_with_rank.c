/**********************
* Licence: MIT
* Author: Leo Ma
* Date: 2014-03-12
*********************/

#include <stdio.h>
#include <stdlib.h>

#define MAX_LEVEL 32  /* Should be enough for 2^32 elements */

struct sk_link {
  struct sk_link *next, *prev;
  unsigned int span;
};

struct range_spec {
  int min, max;
  int minex, maxex;
};

struct man {
  int level;
  int dog_num;
  struct sk_link collar[MAX_LEVEL];
};

struct dog {
  int price;
  struct sk_link link[];
};

static inline void
sklist_init(struct sk_link *link)
{
  link->prev = link;
  link->next = link;
}

static inline void
__sklist_add(struct sk_link *link, struct sk_link *prev, struct sk_link *next)
{
  link->next = next;
  link->prev = prev;
  next->prev = link;
  prev->next = link;
}

static inline void
__sklist_del(struct sk_link *prev, struct sk_link *next)
{
  prev->next = next;
  next->prev = prev; 
}

static inline void
sklist_add(struct sk_link *link, struct sk_link *next)
{
  __sklist_add(link, next->prev, next);
}

static inline void
sklist_del(struct sk_link *link)
{
  __sklist_del(link->prev, link->next);
  sklist_init(link);
}

static inline int
sklist_empty(struct sk_link *link)
{
  return link->next == link;
}

#define sklist_entry(ptr, type, member) \
  ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define sklist_foreach_forward(pos, end) \
  for (; pos != end; pos = pos->next)

#define sklist_foreach_forward_safe(pos, n, end) \
  for (n = pos->next; pos != end; pos = n, n = pos->next)

#define sklist_foreach_backward(pos, end) \
  for (; pos != end; pos = pos->prev)

#define sklist_foreach_backward_safe(pos, n, end) \
  for (n = pos->prev; pos != end; pos = n, n = pos->prev)

// Wow!
struct dog *
dog_birth(int level, int price)
{
  struct dog *dog;

  dog = (struct dog *)malloc(sizeof(*dog) + level * sizeof(struct sk_link));
  if (dog == NULL)
    return NULL;

  dog->price = price;
  return dog;
}

// Wow! Wow! Wow! Wowwwwwwwwwwwwwww...
void
dog_kill(struct dog *dog)
{
  free(dog);
}

// Come on, baby!
struct man *
man_birth()
{
  int i;
  struct man *man;

  man = (struct man *)malloc(sizeof(*man));
  if (man == NULL)
    return NULL;

  man->level = 1;
  man->dog_num = 0;
  for (i = 0; i < sizeof(man->collar) / sizeof(man->collar[0]); i++) {
    sklist_init(&man->collar[i]);
    man->collar[i].span = 0;
  }

  return man;
}

// Please don't, Bro! please!
void
man_kill(struct man *man)
{
  struct sk_link *pos, *n;
  struct dog *dog;

  pos = man->collar[0].next;
  sklist_foreach_forward_safe(pos, n, &man->collar[0]) {
    dog = sklist_entry(pos, struct dog, link[0]);
    dog_kill(dog);
  }
  
  free(man);
}

static int
random_level(void)
{
  const double SKIPLIST_P = 0.25;
  int level = 1;
  while ((random() & 0xffff) < 0xffff * SKIPLIST_P)
    level++;
  return level > MAX_LEVEL ? MAX_LEVEL : level;
}

/* O^_^O */
void
adopt(struct man *man, int price)
{
  int i, level, rank[MAX_LEVEL];
  struct sk_link *pos, *end, *update[MAX_LEVEL];
  struct dog *dog;

  level = random_level();
  if (level > man->level)
    man->level = level;

  dog = dog_birth(level, price);
  if (dog == NULL)
    return;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    rank[i] = i == man->level - 1 ? 0 : rank[i + 1];
    pos = pos->next;
    sklist_foreach_forward(pos, end) {
      struct dog *d = sklist_entry(pos, struct dog, link[i]);
      if (d->price >= price) {
        end = &d->link[i];
        break;
      }
      rank[i] += d->link[i].span;
    }

    update[i] = end;
    pos = end->prev;
    pos--;
    end--;
  }

  for (i = 0; i < man->level; i++) {
    if (i < level) {
      sklist_add(&dog->link[i], update[i]);
      dog->link[i].span = rank[0] - rank[i] + 1;
      update[i]->span -= dog->link[i].span - 1;
    } else {
      update[i]->span++;
    }
  }

  man->dog_num++;
}

/* T_T */
void
__abandon(struct man *man, struct dog *dog, int level, struct sk_link **update)
{
  int i;

  for (i = 0; i < man->level; i++) {
    if (i < level) {
      sklist_del(&dog->link[i]);
      update[i] = dog->link[i].next;
      update[i]->span += dog->link[i].span - 1;
    } else {
      update[i]->span--;
    }

    if (sklist_empty(&man->collar[i])) {
      man->level--;
    }
  }

  dog_kill(dog);
  man->dog_num--;
}

/* T_T */
void
abandon(struct man *man, int price)
{
  int i;
  struct sk_link *pos, *n, *end, *update[MAX_LEVEL];

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward_safe(pos, n, end) {
      struct dog *dog = sklist_entry(pos, struct dog, link[i]);
      if (dog->price > price) {
        end = &dog->link[i];
        break;
      } else if (dog->price == price) {
        // Here's no break statement because we allow dogs with same price.
        __abandon(man, dog, i + 1, update);
      }
    }
    update[i] = end;
    pos = end->prev;
    pos--;
    end--;
  }
}

int
price_gte_min(int price, struct range_spec *range)
{
  return range->maxex ? (price > range->max) : (price >= range->max);
}

int
price_lte_max(int price, struct range_spec *range)
{
  return range->minex ? (price < range->min) : (price <= range->min);
}

/* Returns if there is dog price in range */
int
price_in_range(struct man *man, struct range_spec *range)
{
  struct dog *dog;
  struct sk_link *link;

  if (range->min > range->max ||
    (range->min == range->max && (range->minex || range->maxex)))
    return 0;

  if (sklist_empty(&man->collar[0]))
    return 0;

  link = man->collar[0].next;
  dog = sklist_entry(link, struct dog, link[0]);
  if (!price_lte_max(dog->price, range))
    return 0;

  link = man->collar[0].prev;
  dog = sklist_entry(link, struct dog, link[0]);
  if (!price_gte_min(dog->price, range))
    return 0;

  return 1;
}


/* Find the first dog price that is contained in the specified range
 * where min and max are inclusive. */
struct dog *
first_in_range(struct man *man, struct range_spec *range)
{
  int i;
  struct dog *dog = NULL;
  struct sk_link *pos, *end;

  if (!price_in_range(man, range))
    return NULL;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward(pos, end) {
      dog = sklist_entry(pos, struct dog, link[i]);
      if (price_gte_min(dog->price, range)) {
        pos = dog->link[i].prev;
        end = dog->link[i].next;
        goto CONTINUE;
      }
    }
    pos = end->prev;
CONTINUE:
    pos--;
    end--;
  }

  return dog;
}

/* Find the last dog price that is contained in the specified range
 * where min and max are inclusive. */
struct dog *
last_in_range(struct man *man, struct range_spec *range)
{
  int i;
  struct dog *dog = NULL;
  struct sk_link *pos, *end;

  if (!price_in_range(man, range))
    return NULL;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->prev;
    sklist_foreach_backward(pos, end) {
      dog = sklist_entry(pos, struct dog, link[i]);
      if (price_lte_max(dog->price, range)) {
        pos = dog->link[i].next;
        end = dog->link[i].prev;
        goto CONTINUE;
      }
    }
    pos = end->next;
CONTINUE:
    pos--;
    end--;
  }

  return dog;
}

/* Abandon all the dogs with price in range
 * where min and max are inclusive. */
unsigned int
abandon_in_range(struct man *man, struct range_spec *range)
{
  int i;
  unsigned int removed = 0;
  struct dog *dog = NULL;
  struct sk_link *pos, *n, *end, *update[MAX_LEVEL];

  if (price_in_range(man, range))
    return 0;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];
  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward_safe(pos, n, end) {
      dog = sklist_entry(pos, struct dog, link[i]);
      if (!price_lte_max(dog->price, range)) {
        end = &dog->link[i];
        break;
      } else if (price_gte_min(dog->price, range)) {
        /* Here's no break statement because we allow dogs with same price. */
        __abandon(man, dog, i + 1, update);
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

/* Abandon all the dogs with price rank in range
 * where start and stop are inclusive. */
unsigned int
abandon_in_rank(struct man *man, unsigned int start, unsigned int stop)
{
  int i;
  unsigned int removed = 0, traversed = 0;
  struct sk_link *pos, *n, *end, *update[MAX_LEVEL];

  if (start <= 0 || stop <= 0 || start > man->dog_num)
    return 0;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward_safe(pos, n, end) {
      struct dog *dog = sklist_entry(pos, struct dog, link[i]);
      if (traversed + dog->link[i].span > stop) {
        end = &dog->link[i];
        break;
      } else if (traversed + dog->link[i].span >= start) {
        /* Here's no break statement because we allow dogs with same price. */
        __abandon(man, dog, i + 1, update);
        removed++;
        continue;
      }
      traversed += dog->link[i].span;
    }
    update[i] = end;
    pos = end->prev;
    pos--;
    end--;
  }

  return removed;
}

/* Get the dog price rank */
unsigned int
price_rank(struct man *man, int price)
{
  int i;
  unsigned int rank = 0;
  struct sk_link *pos, *end;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward(pos, end) {
      struct dog *dog = sklist_entry(pos, struct dog, link[i]);
      if (dog->price > price) {
        end = &dog->link[i];
        break;
      } else if (dog->price == price) {
        return rank + dog->link[i].span;
      }
      rank += dog->link[i].span;
    }
    pos = end->prev;
    pos--;
    end--;
  }

  return 0;
}

/* Find the dog with specified price. */
struct dog *
find_by_price(struct man *man, int price)
{
  int i;
  struct sk_link *pos, *end;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward(pos, end) {
      struct dog *dog = sklist_entry(pos, struct dog, link[i]);
      if (dog->price == price) {
        return dog;
      } else if (dog->price > price) {
        end = &dog->link[i];
        break;
      }
    }
    pos = end->prev;
    pos--;
    end--;
  }

  return NULL;
}

/* Find the dog with specified price rank. */
struct dog *
find_by_rank(struct man *man, unsigned int rank)
{
  int i;
  unsigned int traversed = 0;
  struct sk_link *pos, *end;

  if (rank == 0 || rank > man->dog_num)
    return NULL;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    pos = pos->next;
    sklist_foreach_forward(pos, end) {
      struct dog *dog = sklist_entry(pos, struct dog, link[i]);
      if (rank == traversed + dog->link[i].span) {
        return dog;
      } else if (traversed + dog->link[i].span > rank) {
        end = &dog->link[i];
        break;
      }
      traversed += dog->link[i].span;
    }
    pos = end->prev;
    pos--;
    end--;
  }

  return NULL;
}

void
print(struct man *man)
{
  int i;
  struct sk_link *pos, *n, *end;
  unsigned int traversed = 0;

  printf("\nTotal %d dogs: \n", man->dog_num);

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    traversed = 0;
    pos = pos->next;
    sklist_foreach_forward_safe(pos, n, end) {
      struct dog *dog = sklist_entry(pos, struct dog, link[i]);
      traversed += dog->link[i].span;
      printf("level:%d price:0x%08x rank:%u\n", i + 1, dog->price, traversed);
    }
    pos = &man->collar[i];
    pos--;
    end--;
  }
}

int
main(void)
{
#define NUM 1024 * 1024

  struct man *man;
  struct dog *dog;
  int i;
  int *price;

  price = (int *)malloc(NUM * sizeof(int));
  if (price == NULL)
    exit(-1);

  // Hello man!
  man = man_birth();
  if (man == NULL)
    exit(-1);

  printf("Test start!\n");
  printf("Start to adopt %d dogs...\n", NUM);

  // Insert test.
  for (i = 0; i < NUM; i++) {
    price[i] = (int)rand();
    adopt(man, price[i]);
  }

  //print(man);
  printf("Adoption finished. Now play with each dog...\n");

  // Search test 1.
  for (i = 0; i < NUM; i++) {
    dog = find_by_price(man, price[i]);
    if (dog != NULL) {
      //printf("dog price:0x%08x\n", dog->price);
    } else {
      printf("Not found:0x%08x\n", price[i]);
    }
    price_rank(man, price[i]);
    //printf("price rank:%d\n", price_rank(man, price[i]));
  }

  // Search test 2.
  for (i = 0; i < NUM; i++) {
    dog = find_by_rank(man, i + 1);
    if (dog != NULL) {
      //printf("dog price:0x%08x\n", dog->price);
    } else {
      printf("Not found:0x%08x\n", price[i]);
    }
  }

  printf("Play finished. Now abandon some dogs...\n");

  // Delete test.
  for (i = 0; i < NUM; i += 2) {
    abandon(man, price[i]);
  }

  //print(man);
  printf("Abandon Finished.\nEnd of Test\n");

  // Goodbye man!
  man_kill(man);

  return 0;  
}

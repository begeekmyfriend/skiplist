/**********************
* Licence: MIT
* Author: Leo Ma
* Date: 2014-03-01
*********************/

#ifndef _SKIP_LIST_H_
#define _SKIP_LIST_H_

struct sk_link {
  struct sk_link *next, *prev;
};

static inline void
__skip_list_add(struct sk_link *link, struct sk_link *prev, struct sk_link *next)
{
  link->next = next;
  link->prev = prev;
  next->prev = link;
  prev->next = link;
}

static inline void
__skip_list_del(struct sk_link *prev, struct sk_link *next)
{
  prev->next = next;
  next->prev = prev; 
}

static inline void
skip_list_add(struct sk_link *link, struct sk_link *prev)
{
  __skip_list_add(link, prev, prev->next);
}

static inline void
skip_list_del(struct sk_link *link)
{
  __skip_list_del(link->prev, link->next);
}

static inline void
skip_list_init(struct sk_link *link)
{
  link->prev = link;
  link->next = link;
}

#define skip_list_entry(ptr, type, member) \
  ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define skip_list_for_each(pos, end) \
  for (; pos != end; pos = pos->next)

#define skip_list_for_each_safe(pos, n, end) \
  for (n = pos->next; pos != end; pos = n, n = pos->next)

#endif /* _SKIP_LIST_H_ */

#include <stdio.h>
#include <stdlib.h>

#define MAX_LEVEL  8

struct man {
  int rank;  // higher rank more dogs
  int dog_num;
  struct sk_link collar[MAX_LEVEL];
};

struct dog {
  int rank;  // highest rank in collar.
  int price;
  struct sk_link link[MAX_LEVEL];
};

// Wow!
struct dog *
dog_birth()
{
  int i;
  struct dog *dog;
  dog = (struct dog *)malloc(sizeof(*dog));
  if (dog == NULL)
    return NULL;

  dog->rank = 1;
  dog->price = 0;
  for (i = 0; i < sizeof(dog->link) / sizeof(dog->link[0]); i++)
    skip_list_init(&dog->link[i]);

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

  man->rank = 1;
  man->dog_num = 0;
  for (i = 0; i < sizeof(man->collar) / sizeof(man->collar[0]); i++)
    skip_list_init(&man->collar[i]);

  return man;
}

// Please don't, Bro! please!
void
man_kill(struct man *man)
{
  struct sk_link *pos, *n;
  struct dog *dog;
  int i;

  pos = man->collar[0].next;
  skip_list_for_each_safe(pos, n, &man->collar[0]) {
    dog = skip_list_entry(pos, struct dog, link[0]);
    for (i = dog->rank - 1; i >= 0; i--) {
      skip_list_del(&dog->link[i]);
    }
    dog_kill(dog);
  }
  
  free(man);
}

static int
random_level(void)
{
  int level = 1;
  while ((random() & 0xffff) < 0xffff / 2)
    level++;
  return level > MAX_LEVEL ? MAX_LEVEL : level;
}

struct dog * /* $_$ */
find(struct man *man, int price)
{
  int i;
  struct sk_link *pos, *end;

  i = man->rank - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    struct dog *dog = NULL;
    pos = pos->next;
    skip_list_for_each(pos, end) {
      dog = skip_list_entry(pos, struct dog, link[i]);
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

void /* O^_^O */
adopt(struct man *man, struct dog *dog)
{
  int i, rank;
  struct sk_link *pos, *end;

  rank = random_level();
  dog->rank = rank;

  if (rank > man->rank) {
    man->rank = rank;
  }

  i = rank - 1;
  pos = &man->collar[i];
  end = &man->collar[i];
  for (; i >= 0; i--) {
    struct dog *d = NULL;
    pos = pos->next;
    skip_list_for_each(pos, end) {
      d = skip_list_entry(pos, struct dog, link[i]);
      if (d->price >= dog->price) {
        end = &d->link[i];
        break;
      }
    }
    pos = end->prev;
    skip_list_add(&dog->link[i], pos);
    pos--;
    end--;
  }

  man->dog_num++;
}

void /* T_T */
abandon(struct man *man, struct dog *dog)
{
  int i;
  for (i = dog->rank - 1; i >= 0; i--)
    skip_list_del(&dog->link[i]);

  man->dog_num--;
}

void
print(struct man *man)
{
  struct sk_link *pos, *n;
  struct dog *dog;
  //int i;

  printf("\nTotal %d dogs: \n", man->dog_num);
  pos = man->collar[0].next;
  skip_list_for_each_safe(pos, n, &man->collar[0]) {
    dog = skip_list_entry(pos, struct dog, link[0]);
    printf("price:0x%08x\n", dog->price);
  }
}

int main(void)
{
#define NUM 10000

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

  // Insert test.
  for (i = 0; i < NUM; i++) {
    dog = dog_birth();
    if (dog == NULL)
      exit(-1);
    price[i] = dog->price = (int)rand();
    adopt(man, dog);
  }

  // Show
  //print(man);

  // Search test.
  for (i = 0; i < NUM; i++) {
    dog = find(man, price[i]);
    if (dog != NULL) {
      //printf("dog price:0x%08x\n", dog->price);
    } else {
      printf("Not found:0x%08x\n", price[i]);
    }
  }

  // Delete test.
  for (i = 0; i < NUM; i += 3) {
    dog = find(man, price[i]);
    if (dog != NULL) {
      abandon(man, dog);
    } else {
      printf("Not found:0x%08x\n", price[i]);
    }
  }

  // Show
  //print(man);

  // Goodbye man!
  man_kill(man);

  return 0;  
}

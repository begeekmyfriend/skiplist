/**********************
* Licence: MIT
* Author: Leo Ma
* Date: 2014-03-01
*********************/
#include <stdio.h>
#include <stdlib.h>


struct sk_link {
  struct sk_link *next, *prev;
};

static inline void
skip_list_init(struct sk_link *link)
{
  link->prev = link;
  link->next = link;
}

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
  skip_list_init(link);
}

static inline int
skip_list_empty(struct sk_link *link)
{
  return link->next == link;
}

#define skip_list_entry(ptr, type, member) \
  ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define skip_list_foreach(pos, end) \
  for (; pos != end; pos = pos->next)

#define skip_list_foreach_safe(pos, n, end) \
  for (n = pos->next; pos != end; pos = n, n = pos->next)

#define MAX_LEVEL 32 

struct man {
  int level;
  int dog_num;
  struct sk_link collar[MAX_LEVEL];
};

struct dog {
  int price;
  struct sk_link link[];
};

// Wow!
struct dog *
dog_birth(int level, int price)
{
  int i;
  struct dog *dog;

  dog = (struct dog *)malloc(sizeof(*dog) + level * sizeof(struct sk_link));
  if (dog == NULL)
    return NULL;

  dog->price = price;
  for (i = 0; i < level; i++)
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

  man->level = 1;
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

  pos = man->collar[0].next;
  skip_list_foreach_safe(pos, n, &man->collar[0]) {
    dog = skip_list_entry(pos, struct dog, link[0]);
    dog_kill(dog);
  }
  
  free(man);
}

static int
random_level(void)
{
  int level = 1;
  while ((random() & 0xffff) < 0xffff / 4)
    level++;
  return level > MAX_LEVEL ? MAX_LEVEL : level;
}

/* $_$ */
struct dog *
find(struct man *man, int price)
{
  int i;
  struct sk_link *pos, *end;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    struct dog *dog;
    pos = pos->next;
    skip_list_foreach(pos, end) {
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

/* O^_^O */
void
adopt(struct man *man, int price)
{
  int i, level;
  struct sk_link *pos, *end;
  struct dog *dog;

  level = random_level();
  if (level > man->level) {
    man->level = level;
  }

  dog = dog_birth(level, price);
  if (dog == NULL) {
    return;
  }

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    struct dog *d;
    pos = pos->next;
    skip_list_foreach(pos, end) {
      d = skip_list_entry(pos, struct dog, link[i]);
      if (d->price >= price) {
        end = &d->link[i];
        break;
      }
    }
    pos = end->prev;
    if (i < level) {
      __skip_list_add(&dog->link[i], pos, end);
    }
    pos--;
    end--;
  }

  man->dog_num++;
}

/* T_T */
void
__abandon(struct man *man, struct dog *dog, int level)
{
  int i;
  for (i = 0; i < level; i++) {
    skip_list_del(&dog->link[i]);
    if (skip_list_empty(&man->collar[i])) {
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
  struct sk_link *pos, *n, *end;

  i = man->level - 1;
  pos = &man->collar[i];
  end = &man->collar[i];

  for (; i >= 0; i--) {
    struct dog *dog;
    pos = pos->next;
    skip_list_foreach_safe(pos, n, end) {
      dog = skip_list_entry(pos, struct dog, link[i]);
      if (dog->price == price) {
        // Here's no break statement because we allow dogs with same price.
        __abandon(man, dog, i + 1);
      }
    }
    pos = end->prev;
    pos--;
    end--;
  }
}

void
print(struct man *man)
{
  struct sk_link *pos, *n;
  struct dog *dog;

  printf("\nTotal %d dogs: \n", man->dog_num);
  pos = man->collar[0].next;
  skip_list_foreach_safe(pos, n, &man->collar[0]) {
    dog = skip_list_entry(pos, struct dog, link[0]);
    printf("price:0x%08x\n", dog->price);
  }
}

int main(void)
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

  // Search test.
  for (i = 0; i < NUM; i++) {
    dog = find(man, price[i]);
    if (dog != NULL) {
      //printf("dog price:0x%08x\n", dog->price);
    } else {
      printf("Not found:0x%08x\n", price[i]);
    }
  }

  printf("Play finished. Now abandon some dogs...\n");

  // Delete test.
  for (i = 0; i < NUM; i += 3) {
    abandon(man, price[i]);
  }

  //print(man);
  printf("Abandon Finished.\nEnd of Test\n");

  // Goodbye man!
  man_kill(man);

  return 0;  
}

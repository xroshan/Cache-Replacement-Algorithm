#ifndef CACHE_H
#define CACHE_H

#include <stdlib.h>

#include "map.h"
#include "list.h"

typedef enum
{
  LRU,
  ARC,
  LIRS
} cache_type;

typedef struct
{
  list *l;
  map *pages;
  int capacity;
} cache_lru;

typedef struct
{
  list *t1, *t2, *b1, *b2;
  map *ghosts, *pages;
  double p;
  int capacity;
} cache_arc;

typedef struct
{
  list *s, *q;
  map *ms, *mq;
  int Llirs;
  int Lhirs;
  int Lsize;
  int Hsize;
  int NRsize;

} cache_lirs;

typedef struct
{

  cache_type t;

  union {
    cache_lru *lru;
    cache_arc *arc;
    cache_lirs *lirs;
  } data;

  unsigned long requests;
  unsigned long hits;

} cache;

cache *cache_create(cache_type t, int capacity, int Lhirs_size);
void cache_free(cache *c);

void cache_get(cache *c, int addr);
void cache_print_stats(cache *c);

cache_lru *cache_lru_create(int capacity);
int cache_lru_get(cache_lru *lru, int addr);
void cache_lru_free(cache_lru *lru);

cache_lirs *cache_lirs_create(int capacity, int Lhirs_size);
void stack_pruning(cache_lirs *lirs, list *l, map *m);
int cache_lirs_get(cache_lirs *lirs, int addr);
void cache_lirs_free(cache_lirs *lirs);

cache_arc *cache_arc_create(int capacity);
void replace(cache_arc *arc, int addr);
int cache_arc_get(cache_arc *arc, int addr);
void cache_arc_free(cache_arc *arc);

#endif

#include <stdio.h>

#include "../include/cache.h"

cache *cache_create(cache_type t, int capacity, int Lhirs_size)
{
  cache *c = malloc(sizeof(cache));

  c->t = t;

  switch (t)
  {
  case LRU:
    c->data.lru = cache_lru_create(capacity);
    break;
  case ARC:
    c->data.arc = cache_arc_create(capacity);
    break;
  case LIRS:
    c->data.lirs = cache_lirs_create(capacity, Lhirs_size);
    break;
  }

  c->requests = 0;
  c->hits = 0;

  return c;
}

void cache_get(cache *c, int addr)
{
  c->requests++;
  int res = 0;
  switch (c->t)
  {
  case LRU:
    res = cache_lru_get(c->data.lru, addr);
    break;
  case ARC:
    res = cache_arc_get(c->data.arc, addr);
    break;
  case LIRS:
    res = cache_lirs_get(c->data.lirs, addr);
    break;
  }
  if (res == 1)
  {
    c->hits++;
  }
}

void cache_free(cache *c)
{
  switch (c->t)
  {
  case LRU:
    cache_lru_free(c->data.lru);
    break;
  case ARC:
    cache_arc_free(c->data.arc);
    break;
  case LIRS:
    cache_lirs_free(c->data.lirs);
    break;
  }
  free(c);
}

void cache_print_stats(cache *c)
{
  printf(
      "requests: %8d, "
      "hits: %8d, "
      "ratio: %5.2f\n",
      c->requests,
      c->hits,
      c->hits * 100 / (float)c->requests);
}

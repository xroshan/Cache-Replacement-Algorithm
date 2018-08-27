#include "../include/cache.h"

cache_lru *cache_lru_create(int capacity)
{
    cache_lru *lru = malloc(sizeof(cache_lru));

    lru->l = list_create();
    lru->pages = map_create_n(capacity);
    lru->capacity = capacity;

    return lru;
}

int cache_lru_get(cache_lru *lru, int addr)
{

    // is page already in cache
    if (map_has(lru->pages, addr))
    {

        page *p = map_get(lru->pages, addr);

        list_remove(lru->l, p);
        list_push_front(lru->l, p);

        return 1;
    }

    page *p = page_create(addr);
    map_set(lru->pages, addr, p);

    list_push_front(lru->l, p);

    // is the cache not full
    if (lru->l->size > lru->capacity)
    {
        page *victim = list_pop_back(lru->l);
        map_unset(lru->pages, victim->addr);
        page_free(victim);
    }

    return 0;
}

void cache_lru_free(cache_lru *lru)
{
    list_free(lru->l);
    map_free(lru->pages);
    free(lru);
}

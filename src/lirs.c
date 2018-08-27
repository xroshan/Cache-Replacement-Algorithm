#include "../include/cache.h"

cache_lirs *cache_lirs_create(int capacity, int Lhirs_size)
{
    cache_lirs *lirs = malloc(sizeof(cache_lirs));

    lirs->Lhirs = (int)Lhirs_size * capacity / 100;
    lirs->Llirs = capacity - lirs->Lhirs;
    lirs->Lsize = 0;
    lirs->Hsize = 0;
    lirs->NRsize = 0;

    lirs->s = list_create();
    lirs->q = list_create();
    lirs->ms = map_create_n(capacity);
    lirs->mq = map_create_n(capacity);

    return lirs;
}

void stack_pruning(cache_lirs *lirs, list *l, map *m)
{
    while (l->tail->type != 1)
    {
        if (l->tail->type == 2)
        {
            lirs->Hsize--;
        }
        else
        {
            lirs->NRsize--;
        }
        page *p = list_pop_back(l);
        map_unset(m, p->addr);
        page_free(p);
    }
}

int cache_lirs_get(cache_lirs *lirs, int addr)
{
    if (lirs->Lsize < lirs->Llirs) // L block set not full
    {
        if (map_has(lirs->ms, addr)) // LIR hit
        {
            page *p = map_get(lirs->ms, addr);
            list_remove(lirs->s, p);
            list_push_front(lirs->s, p);
            return 1;
        }
        else //No hit
        {
            page *p = page_create(addr);
            p->type = 1;
            map_set(lirs->ms, addr, p);
            list_push_front(lirs->s, p);
            lirs->Lsize++;
            return 0;
        }
    }
    else if (lirs->q->size < lirs->Lhirs) //H block set not full
    {

        if (map_has(lirs->ms, addr)) //Stack S hit
        {

            page *p = map_get(lirs->ms, addr);
            if (p->type == 1) //LIR hit
            {
                list_remove(lirs->s, p);
                list_push_front(lirs->s, p);
                stack_pruning(lirs, lirs->s, lirs->ms);
                return 1;
            }
            else //HIR hit
            {
                page *q = map_get(lirs->mq, addr);
                list_remove(lirs->q, q);
                map_unset(lirs->mq, addr);
                page_free(q);

                list_remove(lirs->s, p);
                p->type = 1;
                list_push_front(lirs->s, p);
                lirs->Hsize--;

                page *t = list_pop_back(lirs->s);
                map_unset(lirs->ms, t->addr);

                page *r = page_create(t->addr);
                list_push_front(lirs->q, r);
                map_set(lirs->mq, r->addr, r);

                page_free(t);
                stack_pruning(lirs, lirs->s, lirs->ms);

                return 1;
            }
        }
        else if (map_has(lirs->mq, addr)) //H block hit but not in Stack S
        {
            page *p = map_get(lirs->mq, addr);
            list_remove(lirs->q, p);
            list_push_front(lirs->q, p);

            page *q = page_create(addr);
            q->type = 2;
            list_push_front(lirs->s, q);
            map_set(lirs->ms, addr, q);
            lirs->Hsize++;

            return 1;
        }
        else //No hit
        {
            page *p = page_create(addr);
            p->type = 2;
            map_set(lirs->ms, addr, p);
            list_push_front(lirs->s, p);
            lirs->Hsize++;

            page *q = page_create(addr);
            map_set(lirs->mq, addr, q);
            list_push_front(lirs->q, q);
            return 0;
        }
    }
    else // L and H block set full
    {
        if (map_has(lirs->ms, addr)) //Stack S hit
        {
            page *p = map_get(lirs->ms, addr);
            if (p->type == 1) //L block hit
            {
                list_remove(lirs->s, p);
                list_push_front(lirs->s, p);
                stack_pruning(lirs, lirs->s, lirs->ms);
                return 1;
            }

            else if (p->type == 2) //H block hit
            {
                page *q = map_get(lirs->mq, addr);
                list_remove(lirs->q, q);
                map_unset(lirs->mq, addr);
                page_free(q);

                list_remove(lirs->s, p);
                p->type = 1;
                list_push_front(lirs->s, p);
                lirs->Hsize--;

                page *t = list_pop_back(lirs->s);
                map_unset(lirs->ms, t->addr);

                page *r = page_create(t->addr);
                list_push_front(lirs->q, r);
                map_set(lirs->mq, r->addr, r);

                page_free(t);
                stack_pruning(lirs, lirs->s, lirs->ms);

                return 1;
            }
            else //NR block hit
            {
                page *q = list_pop_back(lirs->q);
                map_unset(lirs->mq, q->addr);

                if (map_has(lirs->ms, q->addr))
                {
                    page *t = map_get(lirs->ms, q->addr);
                    t->type = 3;
                    lirs->Hsize--;
                    lirs->NRsize++;
                }
                page_free(q);

                list_remove(lirs->s, p);
                p->type = 1;
                list_push_front(lirs->s, p);
                lirs->NRsize--;

                page *t = list_pop_back(lirs->s);
                map_unset(lirs->ms, t->addr);

                page *r = page_create(t->addr);
                list_push_front(lirs->q, r);
                map_set(lirs->mq, r->addr, r);

                page_free(t);
                stack_pruning(lirs, lirs->s, lirs->ms);

                return 0;
            }
        }
        else //not in stack
        {
            if (map_has(lirs->mq, addr)) // H block hit but not in stack
            {
                page *p = map_get(lirs->mq, addr);
                list_remove(lirs->q, p);
                list_push_front(lirs->q, p);

                page *q = page_create(addr);
                q->type = 2;
                list_push_front(lirs->s, q);
                map_set(lirs->ms, addr, q);
                lirs->Hsize++;
                return 1;
            }
            else //victim replacement
            {
                page *q = list_pop_back(lirs->q);
                map_unset(lirs->mq, q->addr);

                if (map_has(lirs->ms, q->addr))
                {
                    page *t = map_get(lirs->ms, q->addr);
                    t->type = 3;
                    lirs->Hsize--;
                    lirs->NRsize++;
                }
                page_free(q);

                page *p = page_create(addr);
                p->type = 2;
                list_push_front(lirs->s, p);
                map_set(lirs->ms, addr, p);
                lirs->Hsize++;

                page *r = page_create(addr);
                list_push_front(lirs->q, r);
                map_set(lirs->mq, addr, r);

                return 0;
            }
        }
    }
}

void cache_lirs_free(cache_lirs *lirs)
{
    list_free(lirs->s);
    list_free(lirs->q);

    map_free(lirs->ms);
    map_free(lirs->mq);

    free(lirs);
}

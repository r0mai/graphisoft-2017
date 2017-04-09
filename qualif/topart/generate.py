#!/usr/bin/env python
from __future__ import print_function
from random import randint


def city_name(k):
    return 'G{}'.format(k)


def random_edge(n):
    x = randint(1, n * (n + 1) / 2 - 1)
    if x >= n:
        x += 1  # filter 0 -> n
    d = n
    p = 0
    while x > d:
        p += 1
        x -= d
        d -= 1

    return (p, p + x)


def add_edge(edges, p, q, d):
    edges[p] = edges.get(p, {})
    edges[p][q] = min(edges[p].get(q, d), d)


def minimum_t(n, ds, es):
    edges = {}
    for p, q, d in es:
        add_edge(edges, p, q, d)

    for i, d in enumerate(ds):
        add_edge(edges, i, i + 1, d)

    ws = [0 for i in range(n + 1)]
    ps = set([0])
    qs = set(range(1, n+1))

    while qs:
        cs = []
        for p in ps:
            if p not in edges:
                continue
            for q, d in edges[p].iteritems():
                if q not in qs:
                    continue
                cs.append((ws[p] + d, q))
        w, q = min(cs)
        ws[q] = w
        ps.add(q)
        qs.remove(q)

    return ws[n]


if __name__ == '__main__':
    # N, M: 1-10000, D: 1-1000

    max_n = 10000
    max_m = 10000

    max_d = 1000

    n = randint(2, max_n)   # single city is invalid
    m = randint(1, max_m)

    # n = max_n
    # m = max_m

    ds = [randint(1, max_d) for i in range(n)]
    es = [random_edge(n) + (randint(1, max_d),) for i in range(m)]

    min_t = minimum_t(n, ds, es)
    t = randint(min_t, sum(ds))

    # t = min_t

    print(n)
    for i in range(n):
        print(city_name(i))
    print(' '.join(map(str, ds)))
    print(m)

    for p, q, d in es:
        print(city_name(p), city_name(q % n), d)

    print(t)

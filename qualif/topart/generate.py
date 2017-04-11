#!/usr/bin/env python
from __future__ import print_function
from random import randint
import sys


def city_name(k):
    return 'G{}'.format(k)


def random_edge1(n):
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


def random_edge(n):
    p = randint(0, n - 1)
    q = randint(p + 1, max(p + 10, n))
    return (p, q)


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


def random_distance(v):
    d_min = int(1 * v + 500 * (1-v))
    d_max = int(1000 * v + 500 * (1-v))
    return randint(d_min, d_max)


def random_route(n, v):
    return [random_distance(v) for i in range(n)]


def random_edges(ds, m):
    n = len(ds)
    ss = [0]
    for i in range(n):
        ss.append(ss[-1] + ds[i])

    es = set()
    while len(es) < m:
        d = randint(1, 10)
        p = randint(0, n - d)
        q = p + d
        w = int((ss[q] - ss[p]) / 10)
        es.add((p, q, w))

    return es


if __name__ == '__main__':
    # N, M: 1-10000, D: 1-1000

    max_n = 1000
    max_m = 1000

    max_d = 1000

    n = randint(2, max_n)   # single city is invalid
    m = randint(1, max_m)

    n = 100
    m = 200

    # n = max_n
    # m = max_m

    ds = random_route(n, 0.5)
    es = random_edges(ds, m)

    min_t = minimum_t(n, ds, es)
    max_t = sum(ds)

    q = 0.25
    t = int(min_t * (1 - q) + max_t * q)

    print(n)
    for i in range(n):
        print(city_name(i))
    print(' '.join(map(str, ds)))
    print(m)

    for p, q, d in es:
        print(city_name(p), city_name(q % n), d)

    print(t)
    print(t, file=sys.stderr)

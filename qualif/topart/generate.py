#!/usr/bin/env python
from __future__ import print_function
from random import randint


# (1 <= N <= 10000, 1 <= M <= 10000,
#    1 <= idopontok ket telepules kozott <= 1000, 1 <= T <= 10000000)

# 5
# Graphivaros
# Graphit
# Graphihaza
# Graphipark
# Graphiujfalu
# 30 15 60 45 60
# 1
# Graphihaza Graphiujfalu 30
# 180


def city_name(k):
    return 'G_{}'.format(k)


def random_edge(n):
    x = randint(1, n * (n - 1) / 2)
    d = n - 1
    p = 0
    while x > d:
        p += 1
        x -= d
        d -= 1

    return (p, p + x)


if __name__ == '__main__':
    max_n = 10
    max_m = 10
    max_d = 100
    max_t = max_d * max_n

    n = randint(2, max_n)   # single city is invalid
    m = randint(1, max_m)   #
    ds = [randint(1, max_d) for i in range(n)]
    t = randint(1, max_t)

    print(n)
    for i in range(n):
        print(city_name(i))
    print(' '.join(map(str, ds)))
    print(m)

    for i in range(m):
        p, q = random_edge(n)
        print(city_name(p), city_name(q), randint(1, max_d))
    print(t)

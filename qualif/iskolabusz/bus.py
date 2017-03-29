from __future__ import print_function


def solve0(n):
    d = 1
    xs = range(d, n + d)
    ys = []
    while xs:
        ys.append(xs[0])
        if len(xs) > 1:
            xs = xs[2:] + [xs[1]]
        else:
            xs = []
    return ys[-1], ys


def solve1(n):
    d = 1
    xs = range(d, n + d)
    while len(xs) > 1:
        # print(xs)
        if len(xs) % 2 == 1:
            xs = xs[3::2] + [xs[1]]
        else:
            xs = xs[1::2]

    return xs[0]


def _solve2(n):
    if n > 1000:
        assert(False)
    xd = 0
    xs = range(xd, n + xd)

    i = 0
    d = 1
    s = n
    ys = []
    while i < n and s > 1:
        vs = xs[i::d]
        print(vs + ys, ys)  # , i, s)
        i = i + d

        k = s - len(ys)
        # print('.', k % 2, len(ys))
        if (s - len(ys)) % 2 == 0:
            # ys = []
            ys = ys[1::2]
        else:
            ys = ys[0::2]

        # print('=', ys)
        if s % 2 == 1:
            ys += [i]
            i += 2 * d

        d *= 2
        s /= 2


def solve3(n):
    i = 1
    d = 1
    s = n
    y = n

    while s > 1:
        i += d
        if s % 2 == 1:
            y = i
            i += 2 * d

        d *= 2
        s /= 2

    assert(s == 1)
    return y


if __name__ == '__main__':
    for n in xrange(1, 101):
        y, ys = solve0(n)
        x = solve3(n)
        assert(x == y)
        print('{}: {}'.format(n, y))

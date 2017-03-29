from __future__ import print_function
import sys
import os
from subprocess import PIPE, Popen


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
    return ys[-1]


def compare(n):
    p = Popen(['a.out'], stdin=PIPE, stdout=PIPE)
    out, err = p.communicate(input='{}'.format(n))
    v = int(out)
    v0 = solve0(n)
    if v != v0:
        print('error: n = {}\n expected: {}\nactual: {}'.format(
            n, v0, v), file=sys.stderr)


if __name__ == '__main__':
    for i in xrange(1, 101):
        compare(i)

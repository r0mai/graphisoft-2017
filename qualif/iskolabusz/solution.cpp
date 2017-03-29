#include <cstdio>

int main() {
    int i = 1, d = 1, y = 0, p = 1, s, n;
    scanf("%d", &n);
    s = n;

    while (i <= n && s > 1) {
        i += d;
        p = i;
        d *= 2;
        if (s % 2) y = i, i += d;
        s /= 2;
    }

    printf("%d\n", s == 1 && y ? y : p);
}

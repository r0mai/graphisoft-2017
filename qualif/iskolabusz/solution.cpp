#include <cstdio>

int main() {
    int i = 1, d = 1, y, s;
    scanf("%d", &s);
    y = s;

    while (s > 1) {
        i += d;
        d *= 2;
        if (s % 2) y = i, i += d;
        s /= 2;
    }

    printf("%d\n", y);
}

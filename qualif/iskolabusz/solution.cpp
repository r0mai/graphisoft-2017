#include <cstdio>

int main() {
    int h = 1, n;
    scanf("%d", &n);
    while (n > h) h *= 2;
    printf("%d\n", 2 * n - h);
}

#import <ios>

main() {
    int n, h = scanf("%d", &n);
    for (;n > h;) h *= 2;
    printf("%d", 2 * n - h);
}

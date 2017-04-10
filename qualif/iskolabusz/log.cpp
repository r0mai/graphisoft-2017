#import <complex>

main() {
    int n;
    scanf("%d", &n);
    printf("%d", 2*n - pow(2, ilogb(n)));
}

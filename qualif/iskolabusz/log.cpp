#import <complex>

main() {
    int n;
    scanf("%d", &n);
    printf("%d",(n*=2,n-=1<<ilogb(n-1)));
}

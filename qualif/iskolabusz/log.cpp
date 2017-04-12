#import <complex>

main(int n) {
    scanf("%d", &n),n*=2;
    printf("%d",n-=1<<ilogb(n-1));
}

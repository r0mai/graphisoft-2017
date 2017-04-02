#import <iostream>

main(int h, int n) {
    std::cin >> n;
    while (n > h) h *= 2;
    std::cout << 2 * n - h;
}

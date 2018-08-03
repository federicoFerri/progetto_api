#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int a, b, r;
    r = scanf("%d %d", &a, &b);
    printf("%d", a + b);
    return 0;
}

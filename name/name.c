#include <stdio.h>
#include <stdlib.h>

int main(void) {

    char hostname[50];
    gethostname(hostname, 50);

    puts(hostname);

    return EXIT_SUCCESS;
}

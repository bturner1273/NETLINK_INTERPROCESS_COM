#include <stdio.h>
#include <stdlib.h>

int main () {
    printf("Enter a message: ");
    char message[1023];
    gets(message);
    printf("Your message: %s\n", message);

    return 0;
}

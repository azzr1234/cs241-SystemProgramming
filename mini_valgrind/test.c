/**
* Mini Valgrind Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>

int main() {
    // Your tests here using malloc and free

    char* anything  = malloc(1000);
  anything =  strcpy(anything,"dskfidsfjiso");
 anything = realloc(anything, 1600);

    char * fuk = calloc(100,1);

    free(anything);
    free(fuk);



    return 0;
}

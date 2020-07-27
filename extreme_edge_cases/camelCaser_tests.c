/**
* Extreme Edge Cases Lab
* CS 241 - Fall 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.


    char* input_s = "DID. yOU .know wh.aTt 12 3Abc.\0";
    char* input = malloc(strlen(input_s)+10);
    strcpy(input,input_s);
    char** showme = (*camelCaser)(input);
    char*solution[5];
    _Bool correct = 1;
    solution[0] = "did";
    solution[1] = "you";
    solution[2] = "knowWh";
    solution[3] = "att123Abc";
    solution[4] = NULL;






    for (int i=0; i<4; i++){
      printf("%s\n",showme[i] );
        printf("%s\n",solution[i] );
      if (strcmp(solution[i],showme[i]))
      correct =0;
    }

    printf("%lu\n", sizeof(solution)/sizeof(char*));
    // if ((sizeof(solution)/sizeof(solution[0]))!=(sizeof(showme)/sizeof(showme[0]))){
    //
    //   printf("\n");
    //   free(input);
    //
    //   destroy(showme);
    //   return 0;
    // }
    free(input);
    destroy(showme);
if (correct)
    return 1;

return 0;
}

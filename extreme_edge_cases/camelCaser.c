/**
* Extreme Edge Cases Lab
* CS 241 - Fall 2018
*/


#include "camelCaser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

char **camel_caser(const char *input_str) {

_Bool needcapital = 0;
_Bool notbegin = 0;
_Bool notend= 1;
// char* input = malloc(strlen(input_str)+100);
// strcpy(input, input_str);
char* backup = strdup(input_str);
char* input = backup;
int i =0;
int j =0;
char**output = (char**)malloc(sizeof(char*)*20);
char temp;
for (int x=0;x<20; x++){
 output[x] = malloc(100);
}
printf("%lu\n",sizeof(output)/sizeof(output[0]));

while (*input){

if(ispunct(*input)){

  if (notend){
    output[j][i] = '\0';
    input++;
    j++;
    notbegin = 0;
    i = 0;
}
  else{

    notend= 1;
    j++;
    input++;
    notbegin=0;
    i=0;
  }
}
else if(isspace(*input)){
  needcapital =1;
  input++;
}
else if(isalpha(*input)){
  if (notbegin&&needcapital){
    needcapital =0;
    temp = toupper(*input);
    output[j][i] = temp;
    i++;
    notend = 0;
    input++;

  }else{
    temp =tolower( *input);
    output[j][i] = temp;
    i++;
    notend = 0;
    notbegin = 1;
    needcapital = 0;
    input++;

  }
}else{
  temp=*input;
  output[j][i] = temp;
  i++;
  input++;
  notend = 0;
}

}

if (!ispunct(input_str[strlen(input_str)-1])){
  output[j] = NULL;
}else{
  output[j+1]= NULL;
}

// printf("%s\n",output[0]);
// printf("%s\n",output[1]);
// printf("%s\n",output[2]);
// printf("%s\n",output[3]);
// printf("%s\n",output[4]);
// printf("%s\n",output[5]);

free(backup);
return output;


}

void destroy(char **result) {
if (result==NULL){
    return;
}
char** backup_res = result;
while(*result){
  free(*result);
  result++;
}
free(backup_res);
return;
}

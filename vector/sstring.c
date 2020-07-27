/**
* Vector Lab
* CS 241 - Fall 2018
*/

#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
  char* str;
};

sstring *cstr_to_sstring(char *input) {
    // your code goes here
    sstring *ptr = (sstring*)malloc(sizeof(sstring));
    ptr->str = (char*)malloc(sizeof(char)*(strlen(input)+1));
   strcpy(ptr->str, input);
    return ptr;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char* output = (char*)malloc(sizeof(strlen(input->str)+1));
    strcpy(output,input->str);
    return output;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
char* first = this->str;
char* second = addition->str;

strcat(first,second);

return strlen(first);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
vector* ret=string_vector_create();
char * token = this->str;

char* temp = token;
while (*temp){

  if (*temp==delimiter){
    char* new_begin = temp+1;
    *temp = 0;
    vector_push_back(ret,token);
    token = new_begin;
  }
  temp++;
}
    return ret;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    char* str = this->str;
    size_t count =0;

    if (strstr(str, target)==NULL)
		return -1;

    char* temp = str;
    int undone= 1;
    while (undone){
      char* occur = strstr(temp,target);
      if (occur==NULL) return -1;
      while (temp!=occur){
        temp++;
        count++;
      }
      if (count>offset){

        char* secondhalf = occur+sizeof(target);
        *temp = 0;

        strcat(str,substitution);
        strcat(str,secondhalf);

        undone = 0;
        return count;
      }else{
        temp+=sizeof(target);
        count+=sizeof(target);


      }
    }

    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    char* represent = (char*)malloc(end-start+1);
for (int i = start; i<end; i++){
*(represent+i-start) = this->str[i];

}
return represent;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this->str);
   this->str = NULL;
   free(this);
    this = NULL;
}

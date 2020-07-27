/**
* Password Cracker Lab
* CS 241 - Fall 2018
*/

#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <crypt.h>
#include <sys/types.h>



typedef struct input_raw{
    char* name;
    char* p_hash;
    char* known;
    int remain;
}input_raw;

typedef struct output{
    size_t id;
    size_t numRecovered;
    size_t numFailed;
 }output;

 static queue*  total_job;
 static pthread_t* total_thread;
 static output* thread_id;

 char* getfinal(char* initial){
   char* final = strdup(initial);
   char* temp = final;
   while (*initial){
     if (*initial =='.'){
       *initial = 'a';
       *temp = 'z';
     }
     temp++;
     initial++;
   }
   return final;
 }

void process(void* input){
  output* in = input;

  int result;
  struct crypt_data cdata;
  int hashcount;
  double timecost;
  cdata.initialized = 0;
  input_raw* current;
  char* hash;

  while ((current = queue_pull(total_job))){

    hashcount=0;
    result=0;
    timecost = getThreadCPUTime();
    v1_print_thread_start(in->id,current->name);
    char* lasttry = getfinal(current->known);

    while (1){
    //  end = strcmp(current->known,lasttry);
      hash  = crypt_r(current->known,"xx", &cdata);
      hashcount++;
      if (!strcmp(hash,current->p_hash)){
        result =1;
        break;
      }
      if (!strcmp(current->known,lasttry)){
        result = 0;
        break;
      }

      incrementString(current->known);
    }
    timecost = getThreadCPUTime()-timecost;
    if (result){
      in->numRecovered++;
    }else{
      in->numFailed++;
    }
    v1_print_thread_result(in->id, current->name, current->known, hashcount, timecost,!result);
    free(lasttry);
    free(current->name);
    free(current);
  }
}



input_raw* decode(){

  char* hash;
  char* known;
  char* name ;


  char *line = NULL;
  size_t len = 0;
  ssize_t a;
  while ((a=getline(&line,&len,stdin))!=-1){
     line[a-1] = '\0';
    // input_raw* result=malloc(sizeof(input_raw));
    // name  = line;
    // result->name  = name;
    //
    // hash = strchr(name,' ');
    // hash[0] ='\0';
    // hash+=1;
    //
    // pass = strchr(hash,' ');
    // pass[0] = '\0';
    // pass+= 1;
    // result->p_hash = hash;
    // result->known = pass;

     input_raw* result=malloc(sizeof(input_raw));
    name  = line;
    result->name  = name;

    char* temp = name;
    while (*temp !=' '){
      temp++;
    }
   hash  = temp+1;
    *temp = '\0';


   char* temp2 = hash;
   while (*temp2 !=' '){
     temp2++;
   }

  known =  temp2+1;
   *temp2 ='\0';
   result-> p_hash = hash;

   // char* temp3 = known;
   // while (*temp3 !='.'){
   //   temp3++;
   // }
   //char* dot  = temp3+1;
   //*temp3 = 0;
   result->known = known;
   //
   // int count =1;
   // while (*dot){
   //   if (*dot == '.'){
   //     count++;
   //     //*dot = 'a';
   //     dot++;
   //   }else{
   //     *dot ='\0';
   //     break;
   //   }
   //
   // }
   // result->remain = count;
   // *(line+strlen(name)-1) = '\0';

    return result;
  }

free(line);
  return NULL;
}


void cracking (size_t n){
input_raw* temp;
while ((temp=decode())){
  queue_push(total_job,temp);
}
while (n){
  queue_push(total_job,NULL);
  n--;
}

}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

total_job = queue_create(64);
total_thread  = malloc (sizeof(pthread_t)*thread_count);
thread_id = malloc(sizeof(output)*thread_count);
size_t numRecovered =0;
size_t numFailed =0;

for (size_t i = 0; i < thread_count; i++) {

    thread_id[i].id           = i + 1;
    thread_id[i].numRecovered = 0;
    thread_id[i].numFailed    = 0;

    pthread_create(total_thread + i, NULL, (void *) &process, (void *) (thread_id + i));
}
cracking(thread_count);



for (size_t i =0; i<thread_count; i++){
  pthread_join(total_thread[i],NULL);
  numRecovered += thread_id[i].numRecovered;
  numFailed+= thread_id[i].numFailed;
}

v1_print_summary(numRecovered,numFailed);

queue_destroy(total_job);
free(total_thread);
free(thread_id);

// char* fu = malloc(100);
// fu = "donna xxC4UjY9eNri6 hello...";
// input_raw* sth = decode(fu);
// int c = sth->remain;
// printf("%s\n",sth->name );
// printf("%s\n",sth-> p_hash );
// printf("%s\n",sth->known );
//
// printf("%d\n",c );


    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}

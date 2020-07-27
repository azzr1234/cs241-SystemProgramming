/**
* Mini Valgrind Lab
* CS 241 - Fall 2018
*/

#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>
size_t total_memory_freed;
size_t total_memory_requested;
size_t invalid_addresses;
meta_data* head;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    if (request_size==0){
      return NULL;
    }

    size_t need = request_size+sizeof(meta_data);

    meta_data* mallocd = malloc(need);
    mallocd->request_size = request_size;
    mallocd->filename = filename;
    mallocd-> instruction = instruction;
    mallocd->next = head;
    head = mallocd;
  //  fprintf(stderr,"%lu\n",sizeof(meta_data));


    total_memory_requested += request_size;
//   fprintf(stderr, "%d\n",&head);
    void* omg = mallocd+1;
    return omg;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (num_elements==0||element_size==0){
      return NULL;
    }
    size_t need = (num_elements*element_size);
    meta_data* callocd  = calloc(need+sizeof(meta_data),1);
    if (callocd==NULL){
      return NULL;
    }
    callocd-> filename = filename;
    callocd-> instruction = instruction;
    callocd->request_size  =  need;
    callocd->next = head;
    head = callocd;
    total_memory_requested += need;
    return head+1;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (payload ==NULL){
      if (request_size==0){
        return NULL;
      }
      void* ret = mini_malloc(request_size,filename,instruction);
      return ret;
    }
    else if (request_size ==0){
      mini_free(payload);
      return NULL;
    }

//      fprintf(stderr, "r1\n");

//   START FROM HERE

    _Bool inside =0;

   meta_data*  louis = payload-sizeof(meta_data);
    meta_data * temp = head;
    while (temp){
      if(louis == temp)
      inside = 1;
      temp = temp->next;
    }
    if (!inside){
      invalid_addresses++;
      return NULL;
    }
    if (louis==NULL){
      invalid_addresses++;
      return NULL;
    }

  //  fprintf(stderr, "r2\n");
    meta_data* start = louis;
    if (start->request_size>request_size){
          //fprintf(stderr, "r3\n");
      total_memory_freed+= (start->request_size-request_size);
    }else{
          //fprintf(stderr, "r4\n");
      total_memory_requested += (request_size-start->request_size);
        }
    size_t need = request_size+sizeof(meta_data);
        //fprintf(stderr, "r5\n");
    start = realloc(start,need);
    start->request_size = request_size;
    if (start ==NULL){
      return NULL;
    }


    return start+1;
}

void mini_free(void *payload) {
    // your code here
    if (payload==NULL){
      return;
    }
    meta_data* caito  = payload-sizeof(meta_data);

    _Bool inside =0;
    meta_data * temp = head;
    meta_data * findprev;
    findprev = head;

    while (temp){
      if(caito == temp){
      inside = 1;
      break;
    }
      temp = temp->next;
    }
    if (!inside){
        //fprintf(stderr, "AM I FREEING2\n");
      invalid_addresses++;
      return;
    }
    if (caito==NULL){
        //fprintf(stderr, "AM I FREEING3\n");
      invalid_addresses++;
      return;
    }


      //fprintf(stderr, "AM I FREEING\n");
      size_t needtofree = caito->request_size;
      //fprintf(stderr, "1\n");
      total_memory_freed+= needtofree;

      while (findprev){
        if (findprev->next ==NULL){
          break;
        }
        if(caito == findprev->next){
        findprev->next = caito->next;
        break;
      }
        findprev = findprev->next;
      }

      if (head==caito)
        head = caito->next;

      // meta_data* start = caito;


      //((meta_data*)(payload-sizeof(meta_data))) = ((meta_data*)(payload-sizeof(meta_data)))->next;
      meta_data* eliminated = caito;
      free(eliminated);

      eliminated = NULL;


}

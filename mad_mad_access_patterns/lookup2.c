/**
* Mad Mad Access Patterns Lab
* CS 241 - Fall 2018
*/

#include "tree.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
 #include <sys/mman.h>
 #include <sys/types.h>
     #include <sys/stat.h>
     #include <fcntl.h>
         #include <unistd.h>
          #include <sys/stat.h>

static void* file;
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/
void recurse (int fd, uint32_t offset, char* word){
  if (offset<1){
     printNotFound(word);
     return;
  }
  BinaryTreeNode *node = file+offset;
  printf("%s\nwwww",node->word );
  int res = strcmp(word, node->word);
  if (res==0){
    printFound(word,node->count, node->price);
    return;
  }
  else if (res>0){
    recurse(fd, node->right_child,word);
  }
  else {
    recurse(fd,node->left_child,word);
  }
  //
  // if (file[offset+16]==NULL) exit(1);
  // int i =0;
  // while (*(file+i+offset+16)!=NULL){
  //   if (*(file+i+offset+16)==*word){i++;}
  //   else if(*(file+i+offset+16)<*word){
  //
  //   }
  //   else if (*(file+i+offset+16)> *word){
  //
  //   }
  // }

  }

int main(int argc, char **argv) {

  if (argc<3){perror("input correct arguments");}
  char* c = argv[1];
  // FILE* file =fopen(c,"r+");
  // fseek(file,100,SEEK_END);
  // long size = ftell(file);
  //
  int fd = open(c,O_RDONLY);
  //off_t currentPos = lseek(fd, (size_t)0, SEEK_CUR);
  struct stat detail;
 stat(c,&detail);

printf("%ld\n",detail.st_size);
 //printf("%ld\n sizeis", size);

  file = mmap(NULL,4096,PROT_READ,MAP_SHARED,fd,0);

  if (file == MAP_FAILED) {
         mmapFail(c);
         exit(3);
     }
    close(fd);
  for (int i=2; i<argc; i++){
    recurse(fd,4,argv[i]);
  }


    return 0;
}

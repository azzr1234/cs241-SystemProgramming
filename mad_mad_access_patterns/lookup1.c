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
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

void recurse (FILE* file, char* word, uint32_t offset){
  BinaryTreeNode node;
  if (offset<1){
     printNotFound(word);
     return;
  }
  if (fseek(file,offset,SEEK_SET)==-1){
    exit(1);
  }

char nodeword[20];

fread(&node,sizeof(BinaryTreeNode),1,file);

// printf("%s\n",word);
// printf("%s\n",nodeword );

fseek(file,sizeof(BinaryTreeNode)+offset,SEEK_SET);
fgets(nodeword,20,file);
  int res = strcmp(word,nodeword);
  if (res==0){
    printFound(word, node.count,node.price);
    return;
  }
  else if(res>0){
    recurse(file,word,node.right_child);
  }
  else{
    recurse(file,word,node.left_child);
  }

  return;
}


int main(int argc, char **argv) {
  if (argc<3){perror("input correct arguments");}
  char* c = argv[1];
  FILE* file =fopen(c,"r+");
  //void* temp = calloc(1,100);
  for (int i=2; i<argc; i++){

    char* word =argv[i];
    printf("%d\n",argc );
    recurse(file,word,4);
  //   //size_t size = strlen(word);
  //   BinaryTreeNode node;
  //   fread(&node,sizeof(BinaryTreeNode),1,file);
  // //  BinaryTreeNode *node = ptr;
  //
  //   while (nfind&&node.word){
  //   int res = strcmp(word,node.word);
  //      printf("%f\n", node.price);
  //   if (res==0){ // we find one!
  //     printf("%s:", word);
  //     printf("%d", node.count);
  //     printf(" at ");
  //     printf("$%f", node.price);
  //     nfind = 0;
  //   }
  //   else if(res>0){
  //     printf("left!");
  //     uint32_t offset = node.left_child;
  //
  //     fseek(file,offset+sizeof(BinaryTreeNode),SEEK_CUR);
  //     fread(&node,1,27,file);
  //
  //
  //   }
  //   else if (res<0){
  //         printf("right!");
  //     uint32_t offset = node.right_child;
  //
  //     if(fseek(file,offset+sizeof(BinaryTreeNode),SEEK_CUR)==-1){
  //         formatFail(word);
  //     }
  //     fread(&node,1,27,file);
  //
  //   }
  // }
  //   // if (fseek(file,27,SEEK_SET)!=0)
  //   //   perror("cannt read");
  //

  }
  fclose(file);
    return 0;
}

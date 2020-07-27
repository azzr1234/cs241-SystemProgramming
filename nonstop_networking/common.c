/**
* Networking Lab
* CS 241 - Fall 2018
*/

#include "common.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "format.h"
#include "dictionary.h"

void sig_handler(int s)
{
  s += 1;
  printf("%d\n",s );

}

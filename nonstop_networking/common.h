/**
* Networking Lab
* CS 241 - Fall 2018
*/

#pragma once
#include <stddef.h>
#include <sys/types.h>


#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"
#include <unistd.h>
#include <errno.h>


#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;
typedef struct addrinfo_sockd {
    FILE* haze_pointof_file;
    char* verb;
    int dided;
    const char* buffer;
    ssize_t remaining_bits;
    verb type;
    char* fzt_char;

    size_t buffer_size;
  } addrinfo_sockd;

void sig_handler(int s);

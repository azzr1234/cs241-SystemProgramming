#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#define SIZEOFSIZE_T (8)
#define EACH_READ_SIZE (1024)
char **parse_args(int argc, char **argv);
verb check_args(char **args);
static char* remote;
static char* local;
static int sock_id;



char* get_resp_str(char* str, size_t* size) {

	if (strstr(str, "OK") == str) {
    int i = 14;
		while (i>=0) {
			*size += str[(i+2) / 2 + 2] << (4 * i);
			i = i-2;
		}

		return str + 11;
	} else if (strstr(str, "ERROR") == str) {
		return NULL;
	} else {
		return (char*)(-1);
	}

}







int myconnect(char* host, char*port){
 int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET; /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* TCP */

    s = getaddrinfo(host, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	freeaddrinfo(result);
        exit(1);
    }

    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("connect");
	freeaddrinfo(result);
        exit(2);
    }else{
	freeaddrinfo(result);
	return sock_fd;
}

}


void client_get(){
  char* remote_name = remote;
  char* local_name = local;
	int local_fd = open(local_name, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	dprintf(sock_id, "GET %s\n", remote_name);
	shutdown(sock_id,SHUT_WR);
	char buffer_get_header[256];
	memset(buffer_get_header, 0, 256);
	int first_read = 0;
        while (1) {
            int errno_saved = errno;
            first_read = read(sock_id, buffer_get_header, 256);
            if (first_read >= 0) break;
            if (first_read == -1 && errno == EAGAIN) {
                errno = errno_saved;
                continue;
            } else {
                fprintf(stderr, "READ ERR\n");
                exit(1);
            }
            errno = errno_saved;
        }
				size_t size = 0;
			  size_t real_size = first_read - 11;
			  char* info = get_resp_str(buffer_get_header, &size);
				if (info && info != (char*)-1) {
            write(local_fd, info, first_read - 11);
            char tp[4096]; memset(tp, 0, 4096);
            while (1) {
                memset(tp, 0, 4096);
                int errno_saved = errno;
                int second_read = read(sock_id, tp, 4096);
                if (second_read == 0) break;
                if (second_read > 0) {
                    write(local_fd, tp, second_read);
                    real_size = second_read+real_size;
                } else {
                    if (first_read == -1 &&errno == EAGAIN  ) {
                        errno = errno_saved;
                        continue;
                    }
                }
                errno = errno_saved;
            }
            if (real_size > size) {
							  print_received_too_much_data();

            } else if (real_size < size) {
              print_too_little_data();
            }

        } else if (info == NULL) {
            //error
            char* first = strstr(buffer_get_header, "\n");
						char* second = first+1;
						char *erro = strstr(second, "\n");

            char fla = *erro;
            *erro = '\0';
            print_error_message(buffer_get_header + 6);
            *erro = fla;
        } else {
            print_invalid_response();

        }


}



void client_put(){
  char* remote_name = remote;
  char* local_name = local;
  FILE* loc_file = fopen(local_name, "r+");
  if (!loc_file)
		exit(-1);
  struct stat sb;
  stat(local_name, &sb);
  size_t size = sb.st_size;

  unsigned char size_arr[8];

	size_t tempsize = size;
	int i =0;
	while (i < 8) {
		size_arr[i] = 0xFF & tempsize;
		tempsize = tempsize >> 8;
		i = i+1;

	}



  dprintf(sock_id, "PUT %s\n", remote_name);

	int j =0;
	while(j < 8) {
		dprintf(sock_id, "%c", size_arr[i]);
		j++;
	}
  		char send_file[7777];
			memset(send_file, 0, 7777);
  		if (size < 7750) {
  			fread(send_file, 1, size, loc_file);
  			write(sock_id, send_file, size);
  		} else {
  			while ((size = fread(send_file, 1, 7757, loc_file))) {

  				write(sock_id, send_file, size);
  				memset(send_file, 0, 7777);
  			}
  		}
  		shutdown(sock_id, SHUT_WR);
  		int var_count = 0;
  		char buffer_put[1000]; memset(buffer_put, 0, 1000);
  		char* dis = buffer_put;

  		while ((var_count = read(sock_id, dis, 1)) > 0) {

  			dis += var_count;
  		}

			if (   strstr(buffer_put, "ERROR") == buffer_put) {

					char* first = strstr(buffer_put, "\n");
					char* second = first+1;
					char *erro = strstr(second, "\n");

					char fla = *erro;
					*erro = '\0';
					print_error_message(buffer_put + 6);
					*erro = fla;

			} else if (strstr(buffer_put, "OK") == buffer_put) {
				print_success();
  		} else {
  			print_invalid_response();
  		}


 }
static char large_buffer[100000];
void client_list(){
dprintf(sock_id, "LIST\n");
shutdown(sock_id, SHUT_WR);

       	int var_count = 0;
       	char* dis = large_buffer;
        while ((var_count = read(sock_id, dis, 1)) > 0) {
        dis = dis + var_count;
  		}

       	size_t size = 0;
       	char* first_time = get_resp_str(large_buffer, &size);
       	if (first_time == NULL) {
					char* first = strstr(large_buffer, "\n");
					char* second = first+1;
					char *erro = strstr(second, "\n");

					char fla = *erro;
					*erro = '\0';
					print_error_message(large_buffer + 6);
					*erro = fla;

       	} else if (first_time == (char*)-1) {
       		print_invalid_response();
       	} else {
       		write(1, large_buffer + 11, size);
       	}

      shutdown(sock_id, 0);

      exit(0);
 }


 void client_delete(){
	 char* remote_name = remote;

     	dprintf(sock_id, "DELETE %s\n", remote_name);
     	shutdown(sock_id, SHUT_WR);
     	char buffer_put[1000]; memset(buffer_put, 0, 1000);
		char* dis = buffer_put;
		int var_count = 0;
		while ((var_count = read(sock_id, dis, 1)) > 0) {
			dis += var_count;
		}
		if (   strstr(buffer_put, "ERROR") == buffer_put) {
			char* first = strstr(buffer_put, "\n");
			char* second = first+1;
			char *erro = strstr(second, "\n");

			char fla = *erro;
			*erro = '\0';
			print_error_message(buffer_put + 6);
			*erro = fla;
		} else if (strstr(buffer_put, "OK") == buffer_put) {
			print_success();
		} else {
			print_invalid_response();
		}
}







int main(int argc, char **argv) {
verb option = check_args(argv);
char** param = parse_args(argc,argv);
if (!param) return 0;

if (param[3]!=NULL)
	remote = param[3];

if (param[4]!=NULL)
	local = param[4];

sock_id = myconnect(param[0],param[1]);
if (sock_id==-1) return -1;

switch(option){
case PUT:
	client_put();
	break;


case GET:
	client_get();
	break;

case LIST:
	client_list();
	break;

case DELETE:
	client_delete();
	break;
default:
	printf("error");
}

free (param);
shutdown(sock_id,SHUT_RDWR);
close(sock_id);



}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}

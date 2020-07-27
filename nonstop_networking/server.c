
/**
* Networking Lab
* CS 241 - Fall 2018
*/
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "vector.h"
#include "common.h"
#include "format.h"
#include <sys/types.h>
#include <sys/stat.h>
static char * storagepath;
static vector * all_files;
static int server_socket;
static ssize_t ftemp_size;
static dictionary * addrinfo_dic;
static struct addrinfo * adr_info;
static char * filename;
static bool new_file_status;
static addrinfo_sockd * event_sock;
static int client_socket;



void defaultdestruct(void * socket_data)
{
    addrinfo_sockd * temp = (addrinfo_sockd*)socket_data;
    free(temp->verb);
    free(temp->fzt_char);
    free(temp);
}

ssize_t readsocket(int socket, char *buffer, size_t count, addrinfo_sockd *socket_data) {

    ssize_t bytesRead = 0;
    ssize_t bytesLeft = count;
    ssize_t result = 0;
    while (bytesLeft)
    {
      bytesRead = read(socket, buffer, bytesLeft);
      if (bytesRead == -1)
      {
        if (errno == EAGAIN || errno ==  EINTR || errno == EWOULDBLOCK )
        {
          if (!socket_data)
          {
            continue;
          }
          result = count - bytesLeft;
          return result;
        }
        errno = 0;
        if (socket_data)
        {
          socket_data->dided = 1;
        }
        return -1;
      }
      if (bytesRead == 0)
      {
        if (socket_data)
        {
          socket_data->dided = 1;
        }
        result = count-bytesLeft;
        return result;
      }
      buffer += bytesRead;
      bytesLeft -= bytesRead;

    }
    result = count;
    return result;
}

ssize_t writesocket(int socket, const char *buffer, size_t count, addrinfo_sockd *socket_data) {
    // Your Code Here
    ssize_t bytes_write = 0;
    ssize_t bytesLeft = count;
    ssize_t result;
    while (bytesLeft)
    {
      bytes_write = write(socket, buffer, bytesLeft);
      if (bytes_write == -1)
      {
         if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK )
         {
           if (!socket_data)
           {
             continue;
           }
           socket_data->buffer = buffer;
           socket_data->buffer_size = bytesLeft;
           result = count - bytesLeft;
           return result;
         }
         errno = 0;
         if (socket_data)
         {
           socket_data->dided = 1;
         }
         return -1;
      }
      if (bytes_write == 0)
      {
        if (socket_data)
        {
          socket_data->dided = 1;
        }
        result = count - bytesLeft;
        return result;
      }
      buffer += bytes_write;
      bytesLeft -= bytes_write;
    }
    result = count;
    return result;
}

ssize_t serve_write(FILE * haze_pointof_file, int socket, size_t file_size, addrinfo_sockd * soc)
{
    if (soc->buffer && soc)
    {
       ssize_t bytes_write = 0;
       ssize_t total_write = 0;

       while (soc->buffer_size!=0)
       {
         bytes_write = writesocket(socket, soc->buffer, soc->buffer_size, soc);
         if (bytes_write == -1)
         {
           free(soc->fzt_char);
           return -1;

         }
         else if (bytes_write == 0)
         {
           return 0;
         }
         else
         {
           soc->buffer_size -= bytes_write;
           total_write = total_write+ bytes_write;
         }
       }
       free(soc->fzt_char);
       soc->buffer = NULL;
       soc->buffer_size = 0;
       soc->fzt_char = NULL;
       return total_write;
    }
    char buffer[4096];
    memset(buffer, 0, 4096);
    int bytes_read = 0;
    int bytes_write = 0;
    ssize_t total_write = 0;
    while (file_size)
    {
      bytes_read = fread(buffer, 1, 4096, haze_pointof_file);
      if (bytes_read == 0)
      {
        return total_write;
      }
      file_size -= bytes_read;
      bytes_write = writesocket(socket, buffer, bytes_read, soc);
      if (bytes_write == 0)
      {
        soc->fzt_char = buffer;
        return total_write;
      }
      else if (bytes_write == -1)
      {
        return -1;
      }
      else
      {
        total_write += bytes_write;
        memset(buffer, 0, 4096);
      }
    }
    return total_write;
}

ssize_t serve_read(FILE *haze_pointof_file, int socket, ssize_t retval, addrinfo_sockd* soc)
{
    char buffer [2048];
    ssize_t total_read = 0;
    memset(buffer, 0 ,2048);
    if(retval<=0)
    {
      return 0;
    }
    while (retval>0)
    {
      int bytes_read = readsocket(socket, buffer, 2048, soc);
      if (bytes_read == -1||bytes_read == 0 )
      {
        return total_read;
      }
      fwrite(buffer, bytes_read, 1, haze_pointof_file);
      memset(buffer, 0, 2048);
      total_read += bytes_read;
      retval -= bytes_read;
    }
    return total_read;
}



void server_put()
{
  bool already_in = false;
  if (new_file_status)
  {
    for (size_t i = 0; i < vector_size(all_files); i++)
    {
      if (strcmp(vector_get(all_files, i), filename) == 0)
      {
        already_in = true;
        break;
      }
    }
    //push into all_files
    if(!already_in)
    {
        vector_push_back(all_files, filename);
    }
    //get filesize
    ssize_t file_size = 0;
    ssize_t read_size = readsocket(client_socket, (char*)&file_size, 8, NULL);
    if(read_size!=8)
    {
      perror("fail read size");
      exit(1);
    }
    event_sock->remaining_bits = file_size;
    event_sock->haze_pointof_file = fopen(filename, "w+");
  }

  event_sock->remaining_bits -= serve_read(event_sock->haze_pointof_file, client_socket, 2048, event_sock);

  if (event_sock->remaining_bits <= 0)
  {
    event_sock->dided = 1;
  }
  if (event_sock->dided)
  {
    fclose(event_sock->haze_pointof_file);
    if (event_sock->remaining_bits > 0)
    {
      print_too_little_data();
      dprintf(client_socket,"ERROR\n%s\n",err_bad_file_size);
    }
    else if (event_sock->remaining_bits < 0)
    {
      print_received_too_much_data();
      dprintf(client_socket,"ERROR\n%s\n",err_bad_file_size);
    }
    else
    {
      dprintf(client_socket,"OK\n");
    }
    event_sock->remaining_bits = -1;
  }
}



void server_delete()
      {
        FILE *haze_pointof_file = fopen(filename, "r");
        if (haze_pointof_file != NULL)
        {
          //write ok to client
          dprintf(client_socket,"OK\n");
        }
        else
        {
          //send "ERROR\n" to client
          dprintf(client_socket,"ERROR\n%s\n",err_no_such_file);
        }
        fclose(haze_pointof_file);
      size_t i=0;
       while(i < vector_size(all_files))
       {
         if (strcmp(vector_get(all_files, i), filename) == 0)
         {
           vector_erase(all_files, i);
           break;
         }
         i++;
       }
        unlink(filename);
        event_sock->remaining_bits = -1;
      }


void server_list()
        {
          //write ok to client
          dprintf(client_socket,"OK\n");
          char result_str[ftemp_size];
          memset(result_str, 0, ftemp_size);
          char * curr_pos = result_str;
          ssize_t size = 0;
          size_t i=0;
          while(i < vector_size(all_files))
          {
            char * curr_file_name = vector_get(all_files, i);
            strcpy(curr_pos, curr_file_name);
            size+= strlen(curr_file_name) + 1;
            curr_pos += strlen(curr_file_name);
            *curr_pos = '\n';
            curr_pos++;

            i++;
          }
          // result_str[ftemp_size-1] = 0;
          if (vector_size(all_files) != 0)
          {

            size--;
            writesocket(client_socket, (char*)&size, 8, NULL);
            writesocket(client_socket, result_str, size, NULL);
          }
          else
          {
              writesocket(client_socket, (char*)&size, 8, NULL);
          }
          // free(result_str);
          event_sock->remaining_bits = -1;
        }


void cleanup(int signum)
{

   printf("IM CLEANING@\n");
    vector * dick_vec = dictionary_keys(addrinfo_dic);
    //suhtdown all the socket
    for (size_t i = 0; i < vector_size(dick_vec); i++)
    {
      int curr_socket = (int)vector_get(dick_vec, i);
      shutdown(curr_socket, SHUT_RDWR);
      addrinfo_sockd * curr_event_sock = dictionary_get(addrinfo_dic, &curr_socket);
      //free(curr_event_sock);
      if (curr_event_sock->fzt_char)
      {
        free (curr_event_sock->fzt_char);
      }
    }
    vector_destroy(all_files);
    vector_destroy(dick_vec);

    dictionary_destroy(addrinfo_dic);

    chdir("./..");
    rmdir(storagepath);
    freeaddrinfo(adr_info);
    close(server_socket);

    exit(1);
}


verb getverb(char* lin){
	if (strstr(lin,"GET")) return GET;
	if (strstr(lin,"PUT")) return PUT;
 	if (strstr(lin,"DELETE")) return DELETE;
	if (strstr(lin,"LIST")) return LIST;

return V_UNKNOWN;

}
void server_get() {
   if (new_file_status)
        {
          FILE* haze_pointof_file = fopen(filename, "r");
          if (haze_pointof_file==NULL)//case where there's no such file
          {
            dprintf(client_socket,"ERROR\n%s\n",err_no_such_file);
            event_sock->remaining_bits = -1;
            return;
          }
          //get the sizes
        struct stat sb;
         int res = stat(filename,&sb);
         if (res<0)(perror("stat"));
         size_t file_size = (size_t) sb.st_size;

          event_sock->haze_pointof_file = haze_pointof_file;
          event_sock->remaining_bits = file_size;

          dprintf(client_socket,"OK\n");

          writesocket(client_socket, (char*)&file_size, sizeof(size_t), NULL);
        }
        ssize_t s =  serve_write(event_sock->haze_pointof_file, client_socket, 4096,event_sock);
        event_sock->remaining_bits -= s;
        if ( s == -1||event_sock->remaining_bits <= 0 )
        {
          fclose(event_sock->haze_pointof_file);
          event_sock->remaining_bits = -1;
        }
      }


void execute(struct epoll_event * e)
{
    filename = NULL;
    new_file_status = false;
    event_sock = NULL;
    client_socket = e->data.fd;
    char *copy = malloc(1000);

    if (!dictionary_contains(addrinfo_dic, &client_socket))
    {
      event_sock = calloc(1,sizeof(addrinfo_sockd));
      dictionary_set(addrinfo_dic, &client_socket, event_sock);

      char header[1000];
      memset(header, 0, 1000);
      int count = 0;
      char* curr = header;

      while (1)
      {
        if(1000<=count)//header too long
        {

          dprintf(client_socket,"ERROR\n%s\n",err_bad_request);
          print_invalid_response();
          event_sock->remaining_bits = -1;
          return;
        }
        if(readsocket(client_socket, curr, 1, NULL)==1)
        {
          if(*curr == '\n')
          {
            *curr = 0;
            break;
          }
          count+=1;
          curr+=1;

        }
      }
      char *end_of_verb = strstr(header, " ");
      if (!end_of_verb)
      {
        copy = strdup(header);
        event_sock->verb = strdup(header);

      }
      else
      {
        int verb_len = end_of_verb - header;
        event_sock->verb = strndup(header, verb_len);
        event_sock->verb[end_of_verb-header] = 0;
        filename = strdup(end_of_verb + 1);
        ftemp_size = ftemp_size+ strlen(filename) + 1;
      }
      new_file_status = true;
    }
    else
    {
      event_sock = dictionary_get(addrinfo_dic, &client_socket);


    }
    if (event_sock->remaining_bits == -1)//done process
    {
      dictionary_remove(addrinfo_dic, &client_socket);
      close(client_socket);
      return;
    }

    verb target = getverb(copy);


    char* option = event_sock->verb;
    if(strcmp(option,"PUT")==0){
        event_sock->type = PUT;

          //server_put();
        }
else if(strcmp(option,"GET")==0){
    event_sock->type = GET;
  //server_get();
      }

else if(strcmp(option,"DELETE")==0){
    event_sock->type = DELETE;
    //server_delete();
        }
else if(strcmp(option,"LIST")==0){
      event_sock->type = LIST;
      //server_list();
      }
else{
        printf("not valid command");
        char* error = "ERROR\n";
        writesocket(client_socket, error, 6, NULL);
        writesocket(client_socket, err_bad_request, strlen(err_bad_request), NULL);
        print_invalid_response();
        free(filename);
        exit(1);
    }
//
switch(target){
  case PUT:
	server_put();
  break;
  case GET:
	server_get();
  break;
  case DELETE:

	server_delete();
  break;
  case LIST:

	server_list();
    break;
  case V_UNKNOWN:
   printf("the back up plan");
}

    free(filename);
  }

int main(int argc, char **argv)
{
    // good luck!
    signal(SIGINT, cleanup);
    signal(SIGPIPE, sig_handler);

    char storage_dic[] = "XXXXXX";
    storagepath = mkdtemp(storage_dic);
    print_temp_directory(storagepath);
    chdir(storagepath);

    //create and initialize the data structure we need
    addrinfo_dic = dictionary_create(NULL, int_compare, int_copy_constructor, NULL, NULL, defaultdestruct);
    all_files = string_vector_create();
    ftemp_size = 0;

    //from wiki, build a simple server
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = sock_fd;
    event.events = EPOLLIN;// EPOLLIN==read, EPOLLOUT==write
    server_socket = sock_fd;
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    adr_info = result;


    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (s != 0)
    {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
      exit(1);
    }

    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0)
    {
      perror("bind()");
      exit(1);
    }

    if (listen(sock_fd, 10) != 0){
      perror("listen()");
      exit(1);
    }


    int epfd = epoll_create(1);
    if (epfd == -1)
    {
      perror("epoll()");
      exit(1);
    }




    epoll_ctl(epfd, EPOLL_CTL_ADD, sock_fd, &event);




    while (1)
    {
      struct epoll_event new_event;
      int num_ready = epoll_wait(epfd, &new_event, 1, -1);
      if (num_ready>0)
      {

        if (new_event.data.fd == sock_fd)
        {   printf("ready to write on %d\n", sock_fd);
            int new_fd = accept(sock_fd, NULL, NULL);
            int flags = fcntl(new_fd, F_GETFL, 0);
            fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
            struct epoll_event event;
            memset(&event, 0, sizeof(struct epoll_event));
            event.data.fd = new_fd;
            event.events = EPOLLIN;
            epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &event);

        }
        else
        {
          printf("ready to write on %d\n", sock_fd);
          execute(&new_event);
        }
      }
    }
}

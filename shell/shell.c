
/*
* Shell Lab
* CS 241 - Fall 2018
*/

#include "format.h"
#include "shell.h"
#include "vector.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>



static vector* history =  NULL;
static int exit_signal = 0;
static int exit_value =0;
static int fmode =0;
typedef struct process {
   char *command;
   char *status;
   pid_t pid;
} process;

 int execute(char* command);

int background(char* command){
 if(command[strlen(command)-1]=='&')
   return 1;

 return 0;
}

int execute_outside(char* command){
  vector_push_back(history,command);


  pid_t pid = fork();
  int back =0;
  if (pid<0){
   print_fork_failed();
   return -1;
  }
  if (pid==0){
   char* copy = malloc(200);
  strcpy(copy,command);
  if (copy[strlen(command)-1] == '&') {
            back =1;
             copy[strlen(command)-1] = '\0';
             if (strlen(command) > 1 && copy[strlen(command)-2] == ' ')
                 copy[strlen(command)-1] = '\0';
         }



   size_t num;

   char** argv = malloc(200);//= strsplit(copy," ", &num);
   size_t i = 0;
   char* p = strtok(copy," \n");
   while(p!=NULL){
     *(argv+i) = p;
     p = strtok(NULL," \n");
     i++;
   }

  num = i;


   if(num==0||argv==NULL){
     print_exec_failed(command);
         exit_value =1;
         free(copy);
         free(argv);

     return -1;
   }
   print_command_executed(getpid());
   execvp(argv[0],argv);

   print_exec_failed(command);
  exit_value =1;
  free(copy);
  free(argv);
   return -1;

  }


  if (command[strlen(command)-1]=='&')
   return 0;

  int ww;
  if (waitpid(pid,&ww,0)<0){
        exit_value =1;
   print_wait_failed();
   return -1;
  }

  return 0;

}


void cd_command(char* command){
   size_t len = strlen(command);
    if (len < 3) {
        print_no_directory("");
        return;
    }
	char* c=command;
     c+= 3;
    if (chdir(get_full_path(c))) {
        vector_push_back(history, command);
        print_no_directory(command);
    }
}



void showhistory() {
   // if (strcmp(command, "!history")!=0){
 size_t len = vector_size(history);
     const char *c;
     for (size_t i = 0; i < len; i++) {
         c = vector_get(history, i);
         print_history_line(i, c);
     }

}

int prefix(char* command){
   size_t len = vector_size(history);
    char* c=command;
    c++;
    size_t iter;
    char *comman;
    for (size_t i = 1; i <= len; i++) {
       iter = len - i;
       comman = vector_get(history, iter);
       if (strstr(comman,c)!=NULL) {
             print_command(comman);
             int stat =execute(comman);
             return stat;
      }
  }

  print_no_history_match();
  exit_value = 1;
  return 1;
}


int nth_command(char *command) {
   char* c =command;
    c++;
   size_t len = strlen(c);

   if (!len) {
       print_invalid_index();
       return -1;
   }
   for (size_t i = 0; i < len; i++) {
       if (!isdigit(c[i])) {
           print_invalid_index();
           exit_value = 0;
           return -1;
       }
   }
   size_t num = atoi(c);
   size_t length = vector_size(history);
   if (num < length) {
       char *comman = vector_get(history,num);
       print_command(comman);
        return(execute(comman));

   }
   else{
     print_invalid_index();
     exit_value =1;
     return -1;
   }
 return 0;
}

int logical_operator(char* command, char** two_commands){

  char* loc = NULL;
  size_t length = 0;
      if((loc= strstr(command,"&&"))){
        length = loc - command;
        loc--;
        length--;
        while(*loc){
          if(*loc == ' '){
            loc--;
            length--;
          }else{
            loc++;
            length++;
            break;
          }
        }

        memcpy(two_commands[0],command,length);
        while(*loc == ' ' || *loc=='&'){
          loc++;
        }
        length = strlen(command) - (loc - command);
        memcpy(two_commands[1],loc,length);
        return 1;
      }else if((loc= strstr(command,"||"))){
        length = loc - command;
        do{
          loc--;
          length--;
        }while(*loc == ' ');
        loc++;
        length++;
        memcpy(two_commands[0],command,length);
        while(*loc == ' ' || *loc=='|'){
          loc++;
        }
        length = strlen(command) - (loc - command);
        memcpy(two_commands[1],loc,length);
        return 2;
      }else if((loc= strstr(command,";"))){

        length = loc - command;
        do{
          loc--;
          length--;
        }while(*loc == ' ');
        loc++;
        length++;
        memcpy(two_commands[0],command,length);
        while(*loc == ' ' || *loc==';'){
          loc++;
        }
        length = strlen(command) - (loc - command);
        memcpy(two_commands[1],loc,length);
        return 3;
      }
      return 0;
}

void ct(int pid, char* command){
  if (!pid){
    print_no_process_found(pid);
  }
  int stat = kill(pid,SIGTERM);
  if (stat==-1){
    print_no_process_found(pid);
  }

}


void sp(int pid, char* command){
  if (!pid){
    print_no_process_found(pid);
  }
  int stat = kill(pid,SIGTSTP);
  if (stat==-1){
    print_no_process_found(pid);
  }
  if (stat==0){
    print_stopped_process(pid, command);
  }
}



void k_s(int pid,char* command){
  if (!pid){
        exit_value =1;
    print_no_process_found(pid);
  }
  //printf("goodbye my friends");
  int stat = kill(pid,SIGTERM);
  if (stat==-1){
    exit_value =1;
    print_no_process_found(pid);
  }
  if (stat==0){
    print_killed_process(pid, command);
  }
}



void writetohistory(char* file, vector* vector){
 FILE* fp = fopen(file,"a");

 for (size_t i=0; i<vector_size(vector); i++){
   fprintf(fp,"%s\n",vector_get(vector,i));
 }
}


int execute(char* command){
size_t len = strlen(command);
if (len==0)
return 1;
char* loc;
int exits =-1;


  if (len>4&&strstr(command,"stop")!=NULL){
vector_push_back(history,command);
    char* cin = command+5;
    int val = atoi(cin);
    sp(val,command);

  }

  else if (len>4 &&strstr(command,"kill")!=NULL){
    vector_push_back(history,command);
  char* cin = command+5;
  int val = atoi(cin);

  k_s(val,command);

  }


  else if (len>4 && strstr(command,"cont")!=NULL){
    vector_push_back(history,command);
      char* cin = command+5;
      int val = atoi(cin);
      ct(val,command);

  }



else if( (loc = strstr(command,"cd")) && *(loc+2)==' ')// cd
{      loc+=2;
    while(*loc == ' '){
      loc++;
    }
    exits =0;
    int change_result = chdir(loc);
    if(change_result==-1){
      print_no_directory(loc);
      return -1;
    }
    vector_push_back(history,command);
}

else if (strcmp(command,"!history")==0){
 showhistory();
  exits = 0;
}

else if (command[0] == '#'){
 exits =nth_command(command);

}

else if (command[0]=='!'&&strcmp(command,"!history")!=0){
 int stat =prefix(command);
 exits= stat;
}
else if (strlen(command)==4 && strcmp(command,"exit")==0){
 exit_signal =1;
}

else{     //external
exits = execute_outside(command);
}

return exits;
}

void sigcontrol(int sig){
 pid_t pid = getpgid(getpid());
 printf("%d\n",pid);
 // if (pid!=0){exit_si
 //   kill(getpid(),SIGINT);
 // }
 return;
 // switch
}


int shell(int argc, char *argv[]) {
   // TODO: This is the entry point for your shell.

   pid_t p = getpid();
   char* pa= getcwd(NULL,300);
   print_prompt(pa,p);


signal(SIGINT,sigcontrol);


char* ini_path = malloc(100);
size_t size = 100;
getcwd(ini_path,size);

history = string_vector_create();//vector_create(char_copy_constructor,char_destructor,char_default_constructor);
//pid_t pid;
//int status;
int shouldwrite =0;
int d;
//FILE* historyfile;
char* filepath;
 FILE* command;

if (!(argc == 1 || argc == 3 || argc == 5)) {
     print_usage();
   exit(1);
 }



     if ((d = getopt(argc,argv,"-h")!=-1)){   // need to load in history file and save
       shouldwrite =1;
       if (argc>2)
       filepath = argv[2];

}
   if (argc>2){
      if (strcmp( argv[1], "-f" ) == 0)  // file to be excuted
        fmode =1;
      }

      if (fmode){
        int logic =0;
         command  = fopen(argv[2],"r");
         if (command==NULL){
            print_script_file_error();
            exit(1);
         }
         ssize_t c;
         char* line =NULL;
         size_t len;
         while((c=getline(&line,&len,command))!=-1){
           exit_value =0;
           if (exit_signal==1){
             if (shouldwrite){
               chdir(ini_path);
               writetohistory(filepath,history);
             }
             exit(0);
           }
           pid_t pid = getpid();
           char* path= getcwd(NULL,300);
           print_prompt(path,pid);
           printf("%s\n",line);
           // free(path);
           char* exec = strdup(line);
           char* opt;
           if ((opt = strchr(exec,'\n'))!=NULL){
             *opt = '\0';
           }

           char** command2 = malloc(2*sizeof(char*));
           command2[0] = malloc(100);
           command2[1] = malloc(100);
             logic = logical_operator(exec,command2);
             switch(logic){
               case 0:
                 execute(exec);
                 break;
               case 1:
                 if (execute(command2[0])==0){
                   execute(command2[1]);
                 }
                 break;
               case 2:
                 if (execute(command2[0])==-1){
                   execute(command2[1]);
                 }
                 break;
               case 3:
                   execute(command2[0]);
                   execute(command2[1]);
                   break;

             break;
             }


         }
         fclose(command);
         exit(0);

     }


// red input without command text
char cwd[1024];
char input[1024];
char* last;
int logic =0;
pid_t pid = getpid();
     while(1){
        exit_value =0;
       if (exit_signal==1){
         if (shouldwrite){
           chdir(ini_path);
           writetohistory(filepath,history);
         }
         exit(0);
       }
       if (!getcwd(cwd,sizeof(cwd)))
         print_prompt(cwd,pid);
       if (!fgets(input,1024,stdin))
         exit(1);
       if (*input == EOF)
         exit(0);
       if ((last = strchr(input,'\n'))!=NULL){
         *last = '\0';
       }
 pid_t pid = getpid();
       char* path= getcwd(NULL,300);
       print_prompt(path,pid);
       printf("%s\n",input);


char** command2 = malloc(2*sizeof(char*));
command2[0] = malloc(100);
command2[1] = malloc(100);
  logic = logical_operator(input,command2);
  switch(logic){
    case 0:
      execute(input);
      break;
    case 1:
      if (execute(command2[0])==0){
        execute(command2[1]);
      }
      break;
    case 2:
      if (execute(command2[0])==-1){
        execute(command2[1]);
      }
      break;
    case 3:
        execute(command2[0]);
        execute(command2[1]);
        break;

  break;
  }

}


   //shouldwrite =0;


   return 0;

}

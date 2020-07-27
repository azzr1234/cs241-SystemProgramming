/**
* Password Cracker Lab
* CS 241 - Fall 2018
*/

#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include <crypt.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"

#include <pthread.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
typedef struct jobs{
	char name[20];
	char hash[40];
	char given[40]; // begin aaaa
	char final[40];
	char begin[40];
	int eofstatus;
	int findstatus;
	double cpu_time;
	int position;
	int hashcount;
	size_t threadcount;
	char* answer;
}jobs;

static jobs* job;
void* process(void* input);

typedef struct pool{
	pthread_t id;
	int number;
} pool;

int start(size_t thread_count) {
	job = malloc (sizeof(jobs));
	//job->eofstatus = 0;

	job->hashcount =0;
	job->cpu_time =0;
	job->answer = malloc(40);
	pthread_barrier_init(&barrier,NULL,thread_count+1);
	pthread_mutex_init(&m,NULL);
		job->findstatus =-1;
	double cpu_begin;
	double cpu_end;
	double thread_begin;
	double  thread_end;
job->threadcount = thread_count;
	pool ** wehappyfew = malloc(sizeof(pool*)*thread_count);


		for(int i = 0; i < (int)thread_count; i++){
			wehappyfew[i] = malloc(sizeof(pool));
			wehappyfew[i]->number = i+1;
		}

		for(int i = 0; i < (int)thread_count; i++){
			pthread_create(&(wehappyfew[i]->id), NULL, process, (void*)wehappyfew[i]);
		}

	char t_n[20];
	char t_h[40];
	char t_g[40];
//int eoof;
	while(1){
		job->hashcount = 0;
		job->cpu_time =0;
		cpu_begin = getTime();
		thread_begin = getThreadCPUTime();
		job->eofstatus  = scanf("%s %s %s",t_n,t_h,t_g );
		job->findstatus = 1;
		pthread_barrier_wait(&barrier);
		if (job->eofstatus==EOF) break;

		strcpy(job->name,t_n);
		strcpy(job->hash,t_h);
		strcpy(job->given,t_g);
		v2_print_start_user(t_n);

		int pre = getPrefixLength(t_g); // length of letters
		for (size_t i=pre; i<(strlen(t_g)-pre);i++){
			t_g[i] = 'z';

		}
		strcpy(job->final, t_g);
		for (size_t j=pre; j<(strlen(t_g)-pre);j++){
			t_g[j] = 'a';

		}
		strcpy(job->begin, t_g);
		job->position = pre;

		pthread_barrier_wait(&barrier);

		pthread_barrier_wait(&barrier);
		cpu_end = getTime();
		thread_end = getThreadCPUTime();

		job->cpu_time+= (thread_end-thread_begin);

		v2_print_summary(t_n,job->answer,job->hashcount,cpu_end-cpu_begin,job->cpu_time,job->findstatus);
	}





		for(int i = 0; i < (int)thread_count; i++){
			pthread_join(wehappyfew[i]->id,NULL);
		}
for (size_t i=0; i<thread_count; i++){
	free(wehappyfew[i]);

}
free(job->answer);
free(job);
		free(wehappyfew);
		pthread_mutex_destroy(&m);
		pthread_barrier_destroy(&barrier);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}



void* process(void* input){
  pool* in = input;


	char* temp = malloc(40);
	//size_t thread_count;
	//int thread_id;
	long start_index;
	long count;

  struct crypt_data cdata;
  cdata.initialized = 0;

int status;
int miss;
double timei;
double timef;
char* curhash;
int hash_num=0;
char* wtf = malloc(40);

    while (1){
    //  end = strcmp(current->known,lasttry);
		status =2;
		pthread_barrier_wait(&barrier);  // wait until the main has modified jobs

		if (job->eofstatus==EOF){
			break;
		}
		pthread_barrier_wait(&barrier);

		miss = strlen(job->given)- job->position;
		getSubrange(miss,job->threadcount,in->number,&start_index,&count);
		strcpy(temp, job->given);
		setStringPosition(temp+job->position,start_index);
		v2_print_thread_start(in->number, job->name, start_index, temp);

		hash_num = 0;
		timei = getThreadCPUTime();


		for (int i =0; i<count; i++){

pthread_mutex_lock(&m);
if (job->findstatus==0){
	status = 1;
	pthread_mutex_unlock(&m);
	break;
}
pthread_mutex_unlock(&m);



      curhash  = crypt_r(temp,"xx", &cdata);
      hash_num++;
			strcpy(wtf,curhash);
      if (!strcmp(wtf,job->hash)){
				strcpy(job->answer,temp);
				pthread_mutex_lock(&m);
				job ->findstatus =0;
				pthread_mutex_unlock(&m);
        status =0;
        break;
      }
			if (!strcmp(wtf,job->final)){
				status =2 ;
				break;
			}

      incrementString(temp+job->position);
    }

timef = getThreadCPUTime();

pthread_mutex_lock(&m);
if (job->findstatus==-1)  status =2;
job->cpu_time+=(timef-timei);
v2_print_thread_result(in->number, hash_num, status);

pthread_mutex_unlock(&m);
pthread_mutex_lock(&m);
	job->hashcount += hash_num;
		pthread_mutex_unlock(&m);

		pthread_barrier_wait(&barrier);

	}
    // timecost = getThreadCPUTime()-timecost;
    // if (result){
    //   in->numRecovered++;
    // }else{
    //   in->numFailed++;
    // }
    // v1_print_thread_result(in->id, current->name, current->known, hashcount, timecost,!result);
    // free(lasttry);
    // free(current->name);
    // free(current);
free(temp);
free(wtf);
		return NULL;
  }

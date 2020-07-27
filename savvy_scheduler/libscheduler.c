#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"
// partner: xinyey2
typedef struct _job_info {
    int id;

   int priority;
	double arrival_time;
	double burst_time;
	double start_time;
	double 	end_time;
	double remain_time;
  double last_exc;
    /* Add whatever other bookkeeping you need into this struct. */
} job_info;

priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;
//
int job_num=0;
double waiting_time=0;
double turnarround_time=0;
double response_time=0;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
	job_info* ainfo=(job_info*)(((job*)a)->metadata);
	job_info* binfo=(job_info*)(((job*)b)->metadata);
	if (ainfo->arrival_time < binfo->arrival_time) return -1;
	else if (ainfo->arrival_time > binfo->arrival_time) return 1;
	else return 0;

}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
	job_info* ainfo=(job_info*)(((job*)a)->metadata);
        job_info* binfo=(job_info*)(((job*)b)->metadata);
        if (ainfo->priority < binfo->priority) return -1;
        else if (ainfo->priority > binfo->priority) return 1;
        else return comparer_fcfs(a,b);

}

int comparer_psrtf(const void *a, const void *b) {
 	job_info* ainfo=(job_info*)(((job*)a)->metadata);
        job_info* binfo=(job_info*)(((job*)b)->metadata);
        if (ainfo->remain_time < binfo->remain_time) return -1;
        else if (ainfo->remain_time > binfo->remain_time) return 1;
        else return comparer_fcfs(a,b);

}

int comparer_rr(const void *a, const void *b) {
  job_info* ainfo=(job_info*)(((job*)a)->metadata);
  job_info* binfo=(job_info*)(((job*)b)->metadata);
  if (ainfo->last_exc<0){
    ainfo->last_exc = ainfo->arrival_time;
  }
  if (binfo->last_exc<0){
    binfo->last_exc = binfo->arrival_time;
  }
  if(ainfo->last_exc<binfo->last_exc)
    return -1;
  else
    return 1;
}

int comparer_sjf(const void *a, const void *b) {
        job_info* ainfo=(job_info*)(((job*)a)->metadata);
        job_info* binfo=(job_info*)(((job*)b)->metadata);
        if (ainfo->burst_time < binfo->burst_time) return -1;
        else if (ainfo->burst_time > binfo->burst_time) return 1;
        else return comparer_fcfs(a,b);

}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO complete me!
job_info* ret=(job_info*) malloc(sizeof(job_info));
ret->id = job_number;
ret->last_exc =-1;
  ret->priority = sched_data->priority;
  ret->arrival_time = time;
  ret->burst_time = sched_data->running_time;
 ret->start_time = -1;
 ret->end_time = -1;
 ret->remain_time = sched_data->running_time;
 newjob->metadata = ret;

  priqueue_offer(&pqueue, newjob);

}

job *scheduler_quantum_expired(job *job_evicted, double time) {


    job_info *data;

    // If there is no job currently running, job_evicted will be NULL
    if (!job_evicted) {
        return (job *)priqueue_peek(&pqueue);
    }

    // If the current scheme is not premptive
    if (pqueue_scheme == FCFS || pqueue_scheme == PRI || pqueue_scheme == SJF) {
        // job_evicted is not NULL, return job_evicted
        if (job_evicted) {
            data = (job_info *)job_evicted->metadata;
            data->last_exc = time;
            data->remain_time--;
            if (data->start_time < 0) data->start_time = time - 1;
            return job_evicted;
        }
    }

    // In all other cases, if job_evicted is not NULL,
    // place it back on the queue and return a pointer to the next job that should run.
    if (job_evicted) {
        data = (job_info *)job_evicted->metadata;
        data->last_exc = time;
        data->remain_time--;
        if (data->start_time < 0) data->start_time = time - 1;

        job *ptr = (job *)priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, ptr);
    }
    return (job *)priqueue_peek(&pqueue);




    // TODO complete me!
}


void scheduler_job_finished(job *job_done, double time) {
    // TODO complete me!
    job_info *data = job_done->metadata;
       waiting_time += ((time - data->arrival_time) - data->burst_time);
       turnarround_time += (time - data->arrival_time);
       response_time += (data->start_time - data->arrival_time);
       job_num++;

       free(data);
       data = NULL;
       priqueue_poll(&pqueue);

}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO complete me!

    return waiting_time/job_num;
}

double scheduler_average_turnaround_time() {
    // TODO complete me!
    return turnarround_time/job_num;
}

double scheduler_average_response_time() {
    // TODO complete me!
    return response_time/job_num;
}

void scheduler_show_queue() {
    // Implement this if you need it!i

}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}

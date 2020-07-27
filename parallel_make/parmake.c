/**
 * Parallel Make
 * CS 241 - Fall 2017
 */


#include "format.h"
#include "graph.h"
#include "set.h"
#include "queue.h"
#include "parmake.h"
#include "parser.h"
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

#define INITIAL 0
#define CYCLE 1
#define COMPLETE 2
#define VISITED 3

typedef struct {
    queue* q;
    graph* g;
} thread_pool;
int quit =0;
int queue_size;
pthread_mutex_t q_mutex;


void* parallel(void* i) ;
void DFS(graph* g, char* target);
void print_cycle(graph* g, char* target);
int Dependency_test(graph* g, char* rule_name);
void push_child(graph* g, char* rule_name, set* visited, vector* vec);
int test_file_access(graph* g, char* target);
// detects cycles and sets all children involved to status CYCLE
void DFS(graph* g, char* target) {
    rule_t* current_rule = graph_get_vertex_value(g, target);
    // traverse all my neighbors and uhh detect cycles
    vector* neighbors = graph_neighbors(g, target);
    for(size_t i = 0; i < vector_size(neighbors); i++) {

        char* rule_name = vector_get(neighbors, i);
        rule_t* next_rule = graph_get_vertex_value(g, rule_name);
        if(next_rule->state == CYCLE) {
            // we hit something that was a cycle
            current_rule->state = CYCLE;
            continue;
        }
        else if(next_rule->state == VISITED) {
            // we got a cycle
            current_rule->state = CYCLE;
            continue;
        }
        // set ourselves as seen and continue on
        next_rule->state = VISITED;
        // recurse
        DFS(g, rule_name);
        // next rule encountered a cycle
        if(next_rule->state == CYCLE) {
            current_rule->state = CYCLE;
            continue;
        }
        next_rule->state = INITIAL;
    }
    vector_destroy(neighbors);
}

// checks for a cycle and prints it out if it exists
void print_cycle(graph* g, char* target) {
    rule_t* current_rule = graph_get_vertex_value(g, target);
    DFS(g, target);
    if(current_rule->state == CYCLE) {
        print_cycle_failure(target);
        quit =1;
    }
}

int Dependency_test(graph* g, char* rule_name) {
    rule_t* curr_rule = graph_get_vertex_value(g, rule_name);
    return curr_rule->state;
}


void push_child(graph* g, char* rule_name, set* visited, vector* vec) {
    set_add(visited, rule_name);
    vector* dependencies = graph_neighbors(g, rule_name);
    for(size_t i = 0; i < vector_size(dependencies); i++) {
        char* dep_name = vector_get(dependencies, i);
        int dep_stat = Dependency_test(g, dep_name);
        // make sure the rule is not a cycle and hasn't been visited already
        if(dep_stat != CYCLE && !set_contains(visited, dep_name)) {
            // go into that rule and make sure all it's dependencies are filled
            push_child(g, dep_name, visited, vec);
        }
    }
    vector_destroy(dependencies);
    vector_push_back(vec, rule_name);
}

int test_file_access(graph* g, char* target) {
    vector* dependencies = graph_neighbors(g, target);
    if(access(target,F_OK)!= -1) {
        struct stat *stat_rule = malloc(sizeof(struct stat));
        stat(target, stat_rule);
        for(size_t i = 0; i < vector_size(dependencies); i++) {
            char* current_dep = vector_get(dependencies, i);
            if(access(current_dep,F_OK)!=-1) {
                struct stat* stat_dep = malloc(sizeof(struct stat));
                stat(current_dep, stat_dep);
                if(fabs(difftime(stat_rule->st_mtime, stat_dep->st_mtime)) > 1) {

                    free(stat_dep);
                    free(stat_rule);
                    vector_destroy(dependencies);
                    return 0;
                }
                free(stat_dep);
            }
        }
        free(stat_rule);
    }
    vector_destroy(dependencies);
    return 1;
}

void* parallel(void* i) {
    thread_pool* thread_pool = i;
    queue* q = thread_pool->q;
    graph* g = thread_pool->g;

    // try and get a rule off the task queue
    while(queue_size > 0&& quit!=1) {
        // get a rule off the queue
        pthread_mutex_lock(&q_mutex);
        queue_size--;
        pthread_mutex_unlock(&q_mutex);
        char* rule_name = queue_pull(q);

        int can_build = 1;
        int has_dependency = 0;
        rule_t* curr_rule = graph_get_vertex_value(g, rule_name);
        // check that all dependencies have complete as status
        vector* dependencies = graph_neighbors(g, rule_name);
        for(size_t i = 0; i < vector_size(dependencies); i++) {
            char* dependency = vector_get(dependencies, i);
            // if the rule is not completed we got a problem
            if(Dependency_test(g, dependency) != COMPLETE) {
                can_build = 0;
                if(Dependency_test(g, dependency) == CYCLE) {
                    can_build = 0;
                    has_dependency = 1;

                }
                break;
            }
            if(Dependency_test(g, dependency) == CYCLE) {
                can_build = 0;
                has_dependency = 1;
                break;
            }
        }

        // if we can't build then we gotta push back and rethink
        if(!can_build) {
            // if one of the dependencies CYCLEed then don't push back
            //printf("%d\n", has_dependency);
            // pthread_mutex_lock(&q_mutex);
            // if(has_dependency){quit =1;}
            // if (quit){exit(1);}
            // pthread_mutex_unlock(&q_mutex);
            if(!has_dependency) {
                queue_push(q, rule_name);
                pthread_mutex_lock(&q_mutex);
                //quit++;
                queue_size++;
                pthread_mutex_unlock(&q_mutex);
            }
        }
        // if we can build then lets build it
        else {
            // first check if we need to build it based on the file stuff
            int decenders = test_file_access(g, rule_name);
            if(decenders) {
                // run through all the commands
                for(size_t i = 0; i < vector_size(curr_rule->commands); i++) {
                    int sys_stat = system(vector_get(curr_rule->commands, i));
                    if(sys_stat != 0) {
                        curr_rule->state = CYCLE;
                        break;
                    }
                }
            }
            if(curr_rule->state != CYCLE) {
                curr_rule->state = COMPLETE;
            }
        }
        vector_destroy(dependencies);
    }
    // when nothing is left to eat in the queue
    return NULL;
}


int parmake(char *makefile, size_t num_threads, char **targets) {
    // make the graph
    graph* g = parser_parse_makefile(makefile, targets);
    // find all these dirty cycles
    vector* target = graph_neighbors(g, "");
    for(size_t i = 0; i < vector_size(target); i++) {
        char* current_rule = vector_get(target, i);
        print_cycle(g, current_rule);
    }
    // build the set and queue and do the top sort into a queue
    set* visited = string_set_create();
    vector* task_vec = string_vector_create();
    queue* q = queue_create(-1);
    pthread_mutex_init(&q_mutex, NULL);

    // do the tippy toppy top sort B)
    for(size_t i = 0; i < vector_size(target); i++) {
        char* current_rule = vector_get(target, i);
        push_child(g, current_rule, visited, task_vec);
    }
    // print out the vector

    size_t vec_size = vector_size(task_vec);

    // convert the task_vec to q
    for(size_t i = 0; i < vec_size; i++) {
        char* curr_rule = vector_get(task_vec, i);
        queue_push(q, curr_rule);
        queue_size++;
    }

    // start doing those dependencies things
    thread_pool* i_struct = malloc(sizeof(thread_pool));
    i_struct->g = g;
    i_struct->q = q;

    // spawn up some threads
    pthread_t thread_arr[num_threads];
    for(size_t i = 0; i < num_threads; i++) {
        pthread_create(thread_arr + i, NULL, parallel, i_struct);
    }
    // join up some threads
    for(size_t i = 0; i < num_threads; i++) {
        pthread_join(thread_arr[i], NULL);
    }

    // kill everything
    free(i_struct);
    set_destroy(visited);
    vector_destroy(task_vec);
    queue_destroy(q);
    pthread_mutex_destroy(&q_mutex);
    vector_destroy(target);
    graph_destroy(g);
    return 0;
}

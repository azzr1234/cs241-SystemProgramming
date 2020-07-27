
/**
* Parallel Make Lab
* CS 241 - Fall 2018
*/




#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "rule.h"
#include "dictionary.h"
#include "queue.h"
#include "set.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
// check my git log is right

pthread_mutex_t m;
pthread_cond_t cv;
int target_is_specified = 1;
graph * g;
queue * q;
set * s;
int run_rule(char * t);
int DFS(char * target, graph* g, dictionary* par, set* cycles_lists, set* visited,char* goal);
int dependencytest(char * target);
int rule_runner(char * target);
int execute(char * target) ;
void createQueue(size_t threads);
void* parallel(void *in);

int DFS(char * target, graph* g, dictionary* par, set* cycles_lists, set* visited,char* goal) {


      rule_t * rule = (rule_t *) graph_get_vertex_value(g, target);

      if (rule->state)
          return (!strcmp(rule->data, "iscycle"));

      rule->state = 1;
      char * invalid = "iscycle";
      rule->data = invalid;

  set_add(visited, target);
	vector* neighbors = graph_neighbors(g, target);
	size_t vec_size = vector_size(neighbors);
	for (size_t i = 0; i < vec_size; i++) {
		char* temp = vector_get(neighbors, i);
		if (set_contains(visited, temp)) {
			set_add(cycles_lists,target);
			vector_destroy(neighbors);
      if (!target_is_specified){
			print_cycle_failure(goal);
			// print_cycle_failure(temp);
    }
		   	return 1;
		}
		dictionary_set(par, temp, target);
		DFS(temp, g,par, cycles_lists, visited,goal);
		set_remove(visited, temp);
	}

  char *state = "nocycle";
  rule->data = state;
  queue_push(q, target);

  vector_destroy(neighbors);

  return 0;

}

int dependencytest(char * target) {
    vector * neighbors = graph_neighbors(g, target);
    size_t vec_size = vector_size(neighbors);
    size_t i = 0;

    while (i < vec_size) {
        char * temp = vector_get(neighbors, i);
        rule_t * r = (rule_t *) graph_get_vertex_value(g, temp);

        if (!strcmp(r->data, "iscycle")) {
            vector_destroy(neighbors);
            return 1;
        }

        i++;
    }

    vector_destroy(neighbors);
    return 0;
}

int rule_runner(char * target) {
    vector * neighbors = graph_neighbors(g, target);
    size_t i = 0;
    int flag = 0;
    size_t vec_size = vector_size(neighbors);

    while (i < vec_size) {
        char * temp = vector_get(neighbors, i);
        rule_t * r = (rule_t *) graph_get_vertex_value(g, temp);

        if (!strcmp(r->data, "nocycle")) {
            if (set_contains(s, temp) && !set_contains(s, temp)) {
                set_add(s, temp);

                if (run_rule(temp)) {
                    flag = 1;
                }
            }
            else {
                pthread_mutex_lock(&m);
                while (strcmp((char*)r->data, "nocycle") == 0) {
//
                    pthread_cond_wait(&cv, &m);
                }
                pthread_mutex_unlock(&m);
                flag = !strcmp(r->data, "iscycle");
            }
        }

        i++;
    }

    vector_destroy(neighbors);
    return flag;
}

int execute(char * target) {
    if (!(access(target, F_OK) != -1))
        return 1;

    struct stat curr_stat;
    stat(target, &curr_stat);



    vector * neighbors = graph_neighbors(g, target);

    size_t vec_size = vector_size(neighbors);
    int flag = 0;

    for (size_t i = 0; i< vec_size; i++) {
        if (!(access(vector_get(neighbors, i), F_OK) != -1)) {
            vector_destroy(neighbors);
            return 1;
        }

        struct stat temp;
        stat(vector_get(neighbors, i), &temp);


        if (difftime(temp.st_mtime, curr_stat.st_mtime) > 0)
            flag = 1;

    }

    vector_destroy(neighbors);
    return flag;
}

int run_rule(char * target) {
    rule_t * rule = (rule_t *) graph_get_vertex_value(g, target);

     pthread_mutex_lock(&m);
    if (rule->state && !strcmp(rule->data, "iscycle")) {
        // if (!target_is_specified){
        //     print_cycle_failure(target);
        pthread_cond_broadcast(&cv);
     pthread_mutex_unlock(&m);
        // }
        return 1;
    }

   pthread_mutex_unlock(&m);
    char * invalid = "iscycle";

    if (dependencytest(target)) {
       pthread_mutex_lock(&m);
        rule->data = invalid;
        // if (!target_is_specified){
        //     print_cycle_failure(target);
        // }
        pthread_cond_broadcast(&cv);
       pthread_mutex_unlock(&m);
          return 1;
    }


    if (rule_runner(target)) {
       pthread_mutex_lock(&m);
        rule->data = invalid;
        // if (!target_is_specified){
        //     print_cycle_failure(target);
        // }
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&m);

        return 1;
    }


    if (!execute(target)) {
         pthread_mutex_lock(&m);
        char * done = "done";
        rule->data = done;
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&m);
        return 0;
    }


    size_t i = 0;
    size_t vec_size = vector_size(rule->commands);
    vector * neighbors = graph_neighbors(g, target);

    while (i < vec_size) {
        if (system(vector_get(rule->commands, i))) {
            char * val = "iscycle";
            pthread_mutex_lock(&m);
            rule->data = val;
            pthread_cond_broadcast(&cv);
           pthread_mutex_unlock(&m);
            vector_destroy(neighbors);
            return 1;
        }
        i++;
    }

    pthread_mutex_lock(&m);
    char * done = "done";
    rule->data = done;
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&m);
    vector_destroy(neighbors);
    return 0;
}

void createQueue(size_t threads) {

    size_t i = 0;



    while (i < threads) {
        queue_push(q, NULL);
        i++;
    }

}

void* parallel(void* in){

    char * target = queue_pull(q);

    while (target != NULL) {

        if (!set_contains(s, target)) {
            set_add(s, target);
            run_rule(target);
        }
        target = queue_pull(q);
    }
    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    pthread_cond_init(&cv, NULL);
    pthread_mutex_init(&m, NULL);
if (*targets ==NULL){  // check if we have a specified target
  target_is_specified = 0;
}


dictionary* par = string_to_string_dictionary_create();
set* visited  = string_set_create();
set* cycles_lists  = string_set_create();




// // sentinel node  = (void*) "";


//     // visited = shallow_to_int_dictionary_create();
//     //
//     // graph* tensorflow = parser_parse_makefile(makefile, targets);
//     // void* sentinel = (void*) "";
//     // vector* goal_rules = graph_neighbors(tensorflow, sentinel);
//     // while(!vector_empty(goal_rules)){
//     //   void* key  =
//     }
//
//     return 0;
// }

    g = parser_parse_makefile(makefile, targets);
vector* command_list = graph_vertices(g); // clear
char* begin = vector_get(command_list, 0);
char* goal = vector_get(command_list,1);


    q = queue_create(-1);

    DFS(begin,g,par,cycles_lists,visited,goal);
    createQueue(num_threads);

    s = string_set_create();

//for (char** iterator = targets; *iterator!=NULL; iterator++){
//	if (set_contains(visited,*iterator)){
//		print_cycle_failure(*iterator);
//}
//}
if (*targets &&dependencytest(*targets)){
if (target_is_specified){

      print_cycle_failure(*targets);

}
}


    size_t start = 0;
    pthread_t ids[num_threads];

    while (start < num_threads) {
        pthread_create(&ids[start], NULL, parallel, NULL);
        start++;
    }

    start = 0;
    void * result;

    while (start < num_threads) {
        pthread_join(ids[start], &result);
        start++;
    }

    pthread_cond_destroy(&cv);
    pthread_mutex_destroy(&m);

    queue_destroy(q);
    set_destroy(s);
    graph_destroy(g);
    return 0;
}

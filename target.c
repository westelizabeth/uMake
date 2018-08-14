/* Elizabeth West
   CS347
*/
#include "arg_parse.h"
#include "target.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

/*  STRUCTS  */

/*
*  target_node
*  This struct can be thought of as an 'object', this type will be used in a linked list
*  target node contains a pointer to the next in the linked list,
*  it's name, and two pointers to the start of linked lists of string_nodes.
*  These other two linked lists will contain dependencys and rules for the target
*/
struct target_node {
  a_target_list next;
  char* targetname;
  str_node_list dependency;
  str_node_list rule;
};
typedef struct target_node a_target_node;

/*
*  string_node
*  This struct is used for creating two different linked lists.
*  It contains a pointer to the next string_node in the list and the string it contains
*/
struct string_node {
   str_node_list next;
  char* string;
};
typedef struct string_node a_string_node;


/*
* make_targ_and_dep
* This takes in a pointer to the head of the target list, and the current line
* Only lines with colons come in so it passes line to have the colon removed.
* The line gets parsed to assign the target name to a new target and create
* dependencys to add to the target.
* returns the target that is created so current_T can be updated in main
*
* If argcp is greater than 1 there are dependencys to add
*/
a_target_list make_targ_and_dep(char* line, a_target_list* headTarget){
  colon_to_space(line);
  int argcp;
  char** processed_target_line = arg_parse(line, &argcp);
  a_target_list target_temp = append_target(headTarget, processed_target_line[0]);

  if(argcp > 1){
    for(int i = 1; i < argcp; i ++){
      add_dependency(target_temp, processed_target_line[i]);
    }
  }
  return target_temp;
}


/*
* This simple function changes a colon into a space
*/
void colon_to_space(char* line){
  int c = 0;
  while(line[c] != '\0'){
    if(line[c] == ':'){
      line[c] = ' ';
    }
    c++;
  }
}

/*
* This simple function changes a equal sign into a space
*/
void eq_to_space(char* line){
  int c = 0;
  while(line[c] != '\0'){
    if(line[c] == '='){
      line[c] = ' ';
    }
    c++;
  }
}

/*
* Creates a new target node.
* Sets everything to null besides the targetname which is passed in as name
* returns the node
*/
a_target_list new_target(char* name){
  a_target_node* newNode = malloc(sizeof(a_target_node));
  newNode->next = NULL;
  newNode->targetname = strdup(name);
  newNode->dependency = NULL;
  newNode->rule = NULL;

  return newNode;
}

/*
* uses helper funtion to create a new target node, it passes along the target
* name to the helper function.
* Searches through the target node list and adds the new target to the end of the list
* Returns the new target node that was created with new_target
*/
a_target_list append_target(a_target_list* headTarget, char* targetName){
  a_target_list n = new_target(targetName);
  while(*headTarget != NULL) {
    headTarget = &((*headTarget)->next);
  }
  *headTarget = n;
  return n;
}

/*
* Creates new string node which points to null and contains the string that was passed in
*/
str_node_list new_string_node(char* string){
  a_string_node* newStrNode = malloc(sizeof(a_string_node));
  newStrNode->next = NULL;
  newStrNode->string = strdup(string);

  return newStrNode;
}

/*
* Given the node to which it's dependency list will be added to,
* this uses a helper funtion to create the dependency node it will add.
* Seaches through the end of the dependency list of the target, adds the string node to the end.
*/
void add_dependency(a_target_list temp_target, char* dep){
  str_node_list temp = new_string_node(dep);
  str_node_list *first = &temp_target->dependency;
  while(*first != NULL) {
    first = &((*first)->next);
  }
  *first = temp;
}

/*
* Given the node to which it's rule list will be added to,
* this uses a helper funtion to create the rule node it will add.
* Seaches through the end of the rule list of the target, adds the string node to the end.
*/
void add_rule(char* line, a_target_list current_target){
  str_node_list temp = new_string_node(line);
  str_node_list *first = &current_target->rule;
  while(*first != NULL) {
    first = &((*first)->next);
  }
  *first = temp;
}

/*
* This function is given the head of target list and a name of a potential target.
* It will search through the target list comparing the targetnames with the string
* and return the target if it is found
*/
a_target_list find_target(a_target_list head, char* theName){
  while(head != NULL) {
    if(strcmp(head->targetname,theName) == 0){
      return head;
    }
    head = head->next;
  }
  return NULL;
}

/*
* This method is passed in the method processline and a target
* It will send all the rules to process line to get run.
*/
void for_each_rule(process_the_line callProcess, a_target_list argtarg){
  a_string_node* first = argtarg->rule;

  while(first != NULL ) {
    callProcess(first->string);
    first = first->next;
  }
}

/*
* Given an argument which is a target, this method will create a pointer to the dependency list of that target
* if there are dependencies it will call execute with the dependencies, making it recurisve
*/
void for_each_dep(process_the_line callProcess,  a_target_list argtarg, a_target_list* headTarget){
  a_string_node* first = argtarg->dependency;
  while(first != NULL ) {
    execute(callProcess, headTarget, first->string);
    first = first->next;
  }
}

/*
* Gets st_mtime for target, and checks each dependency's st_mtime to determine if rules should be executed
* Returns execute if rules should be executed
*/
int check_mtime(a_target_list temp){
  struct stat buff;
  stat(temp->targetname, &buff);
  int target_mtime = buff.st_mtime;
  str_node_list dep_list = temp->dependency;
  struct stat buf;
  int execute = 1;
  if(!dep_list){
    execute = 0;
  }
  while(dep_list != NULL) {
    if( stat(dep_list->string, &buf) != 0){
      execute = 0;
    }
    else if(buf.st_mtime > target_mtime){
      execute = 0;
    }
    dep_list = dep_list->next;
  }
  return execute;
}

/*
* Given an argument it will find the target with the same name and recursively
* search for it's dependencies then execute the rules of those dependencies
*/
void execute ( process_the_line callProcess,  a_target_list* headTarget, char* targetname){
a_target_list argtarg = find_target(*headTarget, strdup(targetname));
  if(argtarg != NULL){
    for_each_dep(callProcess, argtarg, headTarget);
    if(check_mtime(argtarg) == 0){
      for_each_rule(callProcess, argtarg);
    }
  }
}

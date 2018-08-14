/* Elizabeth West
   CS347
*/

#ifndef _TARGET_
#define _TARGET_

/*  STRUCTS  */
struct target_node;
typedef struct target_node* a_target_list;

struct string_node;
typedef struct string_node* str_node_list;

typedef void(process_the_line)(char*);

typedef void(execute_rec)(process_the_line,a_target_list*, char*);


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
a_target_list make_targ_and_dep(char* line,a_target_list* headTarget);



/*
* This function changes a colon into a space
*/
void colon_to_space(char* line);


/*
* This function changes a equals sign into a space
*/
void eq_to_space(char* line);

/*
* Creates a new target node.
* Sets everything to null besides the targetname which is passed in as name
* returns the node
*/
a_target_list new_target(char* name);

/*
* uses helper funtion to create a new target node, it passes along the target
* name to the helper function.
* Searches through the target node list and adds the new target to the end of the list
* Returns the new target node that was created with new_target
*/
a_target_list append_target(a_target_list* headTarget, char* targetName);

/*
* Creates new string node which points to null and contains the string that was passed in
*/
str_node_list new_string_node(char* string);

/*
* Given the node to which it's dependency list will be added to,
* this uses a helper funtion to create the dependency node it will add.
* Seaches through the end of the dependency list of the target, adds the string node to the end.
*/
void add_dependency(a_target_list temp_target, char* dep);

/*
* Given the node to which it's rule list will be added to,
* this uses a helper funtion to create the rule node it will add.
* Seaches through the end of the rule list of the target, adds the string node to the end.
*/
void add_rule(char* line, a_target_list current_target);


/*
* This function is given the head of target list and a name of a potential target.
* It will search through the target list comparing the targetnames with the string
* and return the target if it is found
*/
a_target_list find_target(a_target_list head, char* theName);

/*
* This method is passed in the method processline and a target
* It will send all the rules to process line to get run.
*/
void for_each_rule(process_the_line callProcess, a_target_list argtarg);

/*
* Given an argument which is a target, this method will create a pointer to the dependency list of that target
* if there are dependencies it will call execute with the dependencies, making it recurisve
*/
void for_each_dep(process_the_line callProcess, a_target_list argtarg, a_target_list* headTarget);

/*
* Given an argument it will find the target with the same name and recursively
* search for it's dependencies then execute the rules of those dependencies
*/
void execute ( process_the_line callProcess,a_target_list* headTarget, char* targetname);

/*
* Gets st_mtime for target, and checks each dependency's st_mtime to determine if rules should be executed
* Returns execute if rules should be executed
*/
int check_mtime(a_target_list temp);

#endif

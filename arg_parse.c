/* Elizabeth West
   CS347
*/
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "arg_parse.h"



static int count_args(char* line);

#define STATE_OUT 0
#define STATE_IN 1

/*
* Arg Parse
* Process: Uses helper function to count number of words in variable line,
* this method adds null terminators to the end of words using a state machine and
* adds pointers to the beginning of words to an array.
* Returns: a new array of pointers that point to characters in line
*/
char **arg_parse(char *line, int *argcp){
  int numOfArg = count_args(line);
  *argcp = numOfArg;
  char** arguments = malloc (sizeof(char*) * (numOfArg + 1));
  int state = STATE_OUT;
  int k = 0;
  while(*line != '\0'){
    switch(state){
      case STATE_OUT:
        if(!(isspace(*line))){
          state = STATE_IN;
          arguments[k] = line;
          k++;
        }
      break;

      case STATE_IN:
        if(isspace(*line)){
          state = STATE_OUT;
          *line = '\0';
        }
      break;
    }
    line++;
  }
  arguments[k] = '\0';
  return arguments;
}

/*
* count_args
* a state machine to count the number of arguments in a char* passed in called line.
* Returns an int which represents number of words in the line
*/
static int count_args(char* line){
  int numOfTok = 0; //number of words counted in line
  int state = STATE_OUT; //initiate state to out
  while(*line != '\0'){
    switch(state){
      case STATE_OUT:
        if(!(isspace(*line))){
          state = STATE_IN;
          numOfTok++;
        }
      break;

      case STATE_IN:
        if(isspace(*line)){
          state = STATE_OUT;
        }
      break;
    }
    line++;
  }
  return numOfTok;
}

/* Elizabeth West
   CS347
*/
#ifndef ARG_PARSE_
#define ARG_PARSE_
/*
* Arg Parse
* Process: Uses helper function to count number of words in variable line,
* this method adds null terminators to the end of words using a state machine and
* adds pointers to the beginning of words to an array.
* Returns: a new array of pointers that point to characters in line
*/
char **arg_parse(char *line, int *argcp);
#endif

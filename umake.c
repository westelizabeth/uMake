/* CSCI 347 micro-make
   Elizabeth West
   CS347
*/

//st_mtime, files last status change timestamp

//st_mtime is a time_t. So, you can use difftime to find the difference between two times
//http://www.cplusplus.com/reference/ctime/difftime/


//umake
//target: dependecies
// (recipe) commands
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include "arg_parse.h"
#include "target.h"


//#define BUFFER_LEN 1024
/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
void processline(char* line);

/*
* This method will replace environtment variables found in a line that are indicated with ${} formatting
* It uses an additional 'buffer' to copy in the characters that will be switched out
* There are lots of int variables used to save places in the many char*
*/
int expand(char* orig, char* new, int newsize);


/*
* Removes comments in line
*/
void remove_comments(char* line);

/*
* adds environtment variables when a line with an equal sign is detected
* Replaces the equal sign with a space, parses the line to grab the individual variables
* Uses setenv to set environment variables.
*/
void env_assignment(char* line);


/*
* Checks for any redirecting, appending or truncating input/output to different stdout
*/
char** io_redirect(char** parsedLine);

/* Main entry point.
 * argc    A count of command-line arguments
 * argv    The command-line argument valus
 *
 * Micro-make (umake) reads from the uMakefile in the current working
 * directory.  The file is read one line at a time.  Lines with a leading tab
 * character ('\t') are interpreted as a command and passed to processline minus
 * the leading tab.
 */
int main(int argc, const char* argv[]) {
  FILE* makefile = fopen("./uMakefile", "r");
  if(makefile == NULL){
    fprintf(stderr, "%s", "uMakeFile not found, try again.\n");
    exit(1);
  }
  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  a_target_list headTarget = NULL;
  a_target_list current_T = NULL;
  const char colon = ':';
  const char eq = '=';
  const char pound = '#';

  while(-1 != linelen) {
    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }

      char *ispound = strchr(line, pound);
      if(ispound != NULL){
        remove_comments(line);
      }

      char *iseq = strchr(line, eq);
      char *iscolon = strchr(line, colon);
      if(iseq != NULL){
        env_assignment(line);
      }
      else if(iscolon != NULL){
        current_T = make_targ_and_dep(line, &headTarget);
      }
      else if(line[0] == '\t'){
        add_rule(line, current_T);
      }
    linelen = getline(&line, &bufsize, makefile);
  }

  for(int arg = 1; arg < argc; arg++){
    execute( processline, &headTarget, strdup(argv[arg]));
  }

  free(line);
  return EXIT_SUCCESS;
}


/* Process Line*/
void processline (char* line) {
  const int BUFFER_LEN = 1024;
  char buffer[BUFFER_LEN];
  if(!expand(line, buffer, BUFFER_LEN)){
    exit(BUFFER_LEN);
  }

  int argcp;
  char* templine = strdup(buffer);
  char** parsedLine = arg_parse(templine, &argcp);

//  parsedLine = io_redirect(parsedLine);

  if(argcp > 0 ){
    const pid_t cpid = fork();
    switch(cpid) {
      case -1: {
        perror("fork");
        break;
      }

      case 0: {
        parsedLine = io_redirect(parsedLine);
        execvp(parsedLine[0], parsedLine);
        perror("execvp");
        free(parsedLine);
        exit(EXIT_FAILURE);
        break;
      }

      default: {
        int   status;
        const pid_t pid = wait(&status);
        if(-1 == pid) {
          perror("wait");
        }
        else if (pid != cpid) {
          fprintf(stderr, "wait: expected process %d, but waited for process %d", cpid, pid);
        }
        break;
      }
    }
  }
  free(parsedLine);
  free(templine);
}

/*
* This method will replace environtment variables found in a line that are indicated with ${} formatting
* It uses an additional 'buffer' to duplicate the characters that will be switched out
* There are lots of int variables used to save places in the many char*
*/
int expand(char* orig, char* new, int newsize){
  const int FAILURE = 0;
  const int SUCCESS = 1;
  const char CLOSING_CURLY_BRACE = 0x7D;
  const char OPENING_CURLY_BRACE = 0x7B;
  const char MONEY = '$';

  int b = 0;
  int j = 0;
  int c = 0;
  for(int i = 0; orig[i] != '\0'; i++){
    if(orig[i] == MONEY && orig[i+1] == OPENING_CURLY_BRACE){
      b = i+2;
      for(c = b ;orig[c] != CLOSING_CURLY_BRACE ; c++){
        if( orig[c] == '\0'){
          printf("Syntax error: Closing bracket not found\n");
          return FAILURE;
        }
      }
      char* small_buffer = strndup( &orig[b], c-b);
      char* temp = getenv(small_buffer);
      if(temp != NULL){
        int p = strlen(temp);
        if((p + j ) > newsize){
          printf("Expand: buffer overflow\n");
          return FAILURE;
        }
        for(int a = 0; temp[a] != '\0'; a++){
          new[j] = temp[a];
          j++;
        }
      }
      i = c+1;
    }
    if(j  > newsize){
      printf("Expand: buffer overflow\n");
      return FAILURE;
    }
    new[j] = orig[i];
    j++;
  }
  new[j] = '\0';
  return SUCCESS;
}

/*
* when a pound symbol is found, it is replaced with a null terminator
*/
void remove_comments(char* line){
  while(*line != '\0'){
    if(*line == '#'){
      *line = '\0';
    }
    line++;
  }
}

/*
* Checks for any redirecting, appending or truncating input/output to different stdout
*/
char** io_redirect(char** parsedLine){
  int i = 0;
  while(parsedLine[i] != NULL){
    if(parsedLine[i][0] == '<'){
      parsedLine[i] = NULL;
      int fd =  open(parsedLine[i+1],O_RDONLY, 0777);
      dup2(fd, 0);
      if(fd != 1) {
        perror("open");
        exit(1);
      }
      close(fd);
    }
    else if(parsedLine[i][0] == '>' && parsedLine[0][i] == '>' ){
      parsedLine[i] = NULL;
      int fd1 =  open(parsedLine[i+1],O_WRONLY | O_APPEND | O_CREAT, 0777);
      dup2(fd1, 1);
     if(fd1 != 1) {
       perror("open");
       exit(1);
     }
      close(fd1);
    }
    else if(parsedLine[i][0] == '>'){
      parsedLine[i] = NULL;
      int fd2 =  open(parsedLine[i+1],O_WRONLY |  O_TRUNC | O_CREAT , 0777);
      dup2(fd2, 1);
      if(fd2 != 1) {
        perror("open");
        exit(1);
      }
      close(fd2);
    }
    i++;
  }
  return parsedLine;
}

/*
* adds environtment variables when a line with an equal sign is detected
* Replaces the equal sign with a space, parses the line to grab the individual variables
* Uses setenv to set environment variables.
*/
void env_assignment(char* line){
  int nonzero;
  eq_to_space(line);
  char** parsed_line = arg_parse(line, &nonzero);
  setenv(parsed_line[0], parsed_line[1], 1);
}

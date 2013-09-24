# include <stdio.h>
# include <string.h>
#include <unistd.h>
#include <stdlib.h>
# include <sys/wait.h>

int main (int argc, char *argv[]){
  while (1){
    printf("mysh>");
    char* input=malloc(512);
    if (NULL != fgets(input,512,stdin)){
      char* token = strtok(input, "  ");
      char** args=malloc(512 * sizeof(char*));
      char** s_args=args;
      while (token != NULL){
	*args = token;
	args++;
	token=strtok(NULL," ");
      }
      char* lp=*(args-1);
      char* nl_ptr=(char*)(lp+strlen(lp)-1);
      
      if (0  != strcmp(nl_ptr,"\n")){
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
      } else {
	strncpy(nl_ptr,"\0",1);
      }

      //handle build-in
      args=s_args;
      if (0 == strcmp(*args, "exit")){
	exit(0);
      } else if (0 == strcmp (*args, "cd")){
	//?????? how to handle more than 1 arguement is passed in? error or ignore?
	// do we need to handle ..?
	const char* path=*(args+1);
	if (0 == strcmp(path,"")){
	  path=getenv("HOME");
	} 
	if (0 != chdir(path)){
	  //sth wrong with reading in input
	  char error_message[30] = "An error has occurred\n";
	  write(STDERR_FILENO, error_message, strlen(error_message));
	}
      } else if (0 == strcmp (*args, "pwd")){
	char* c_wd=malloc(517); // ????? when passed in as getcwd(c_wd,sizeof(c_wd))), a NULL is returned?
	c_wd=getcwd(NULL,0);
	if (NULL == c_wd){
	  //sth wrong with reading in input
	  char error_message[30] = "An error has occurred\n";
	  write(STDERR_FILENO, error_message, strlen(error_message));
	} else {
	  printf("%s\n",c_wd);
	}
      } else if (0 == strcmp (*args, "wait")){
	//???????????? handle status of waiting for children process???
	(void) wait(NULL);
      }
    } else {
      //sth wrong with reading in input
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }
}

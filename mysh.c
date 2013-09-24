# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <fcntl.h>


void print_error(){
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

void call_bin(char ** input_ptr){
  char* bin_name_ptr;
  bin_name_ptr=*(input_ptr);
  //  char* argv_ptr=malloc(512);
  /* for (++input_ptr;NULL != input_ptr;input_ptr++){ */
  /*   strcat(argv_ptr,*input_ptr); */
  /* } */
  /* char* argv_ptr; */
  /* argv_ptr= (char *)(*(input_ptr)+1); */
  int rc=fork();
  if ( 0 ==rc){
    //    printf("children starting\n");
    //    printf("%s",bin_name_ptr);
    int result=execvp(bin_name_ptr,input_ptr);
    if (-1 == result){
      print_error();
      exit(1);
    }
  } else if ( 0 < rc) {
    //    printf("parent waiting");
    (void)wait(NULL);
  } else {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  //  return rc;
}

void check_redict(char* input){
  char* token = strtok(input, ">");
  char* out_file = malloc(512);
  int num=0;
  while (token != NULL){
    token=strtok(NULL," ");
    out_file = token;
    num++;
  }
  if (1 == num){
    close(STDOUT_FILENO);
    open(out_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  } else {
    //more than one ">"
    print_error();
  }
}

void close_redirect(){
  (void)close(1);
  open("console", O_RDWR);
    //  fdopen("CON", "w");
}

/*delete the new line character at the end of string */
void del_NL(char* ori){
  //get rid of the final \n mark
  char* lp;
  lp=ori;
  char* nl_ptr=(char*)(lp+strlen(lp)-1);
  if (0  != strcmp(nl_ptr,"\n")){
    print_error();
  } else {
    strncpy(nl_ptr,"\0",1);
  }
  //  return nl_ptr;
}


int main (int argc, char *argv[]){
  while (1){
    printf("mysh>");
    char* input=malloc(512);
    if (NULL != fgets(input,512,stdin)){
      //      check_redict(input);
      char* token = strtok(input, " ");
      char** args=malloc(512 * sizeof(char*));
      char** s_args=args;
      while (token != NULL){
	*args = token;
	args++;
	token=strtok(NULL," ");
      }

      del_NL(*(args-1));
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
      } else if (0 == strcmp (*args,"")){ // if user just enter /n
        continue;

      } else {
        //hanlde non-build in. Calling program binaries
              (void)call_bin(args);
      }

    } else {
      //sth wrong with reading in input
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }

    //    close_redirect();
  }
}



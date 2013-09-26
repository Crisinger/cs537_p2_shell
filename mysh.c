# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <fcntl.h>


void del_NL(char* input);

void print_error(){
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

/* check whether we are starting a new process*/
int check_ps(char ** input_ptr, int arg_num){
  char * tmp = *(input_ptr + arg_num -1);
  if (0 == strcmp(tmp, "&")){
    //?????????
    //the following statement doesn't chagne *(input_ptr + argnum -1) to '\0'
    //    *tmp = '\0';
    *(input_ptr + arg_num -1)='\0';
    //    strncpy(tmp,"\0",1);
    return 1;
  } else
    return 0;
}

void call_bin(char ** input_ptr, int arg_num){
  char* bin_name_ptr;
  bin_name_ptr=*(input_ptr);
  int separate_ps= check_ps(input_ptr,arg_num);
  int rc=fork();
  if ( 0 ==rc){
    //child execute
    int result=execvp(bin_name_ptr,input_ptr);
    if (-1 == result){
      print_error();
      exit(1);
    }
  } else if ( 0 < rc) {
    //parent execute
    if (1 != separate_ps)
      (void)wait(NULL);
  } else {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
}


int check_redict(char* input){
  char* token = strtok(input, ">");
  if (NULL != token){
    input=token;
  }
  char out_file[513]={0};
  int num=0;
  while ((token=strtok(NULL," ")) != NULL){
    /* if (0==num){ */
    /*   del_NL(token); */
    /* } */
    strcpy(out_file,token);
    //    out_file = token;
    num++;
  }
  if (1 == num){
    //redirect. Do we overwrite or append?
    freopen(out_file, "a+", stdout);
    return 1;
    //    close(STDOUT_FILENO);
    //    int fd=open(out_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  } else if ( 1 < num ){
    //more than one ">"
    print_error();
  }
  return 0;
}

void close_redirect(int redirect){
  //  (void)close(1);
  //forcefullly redirect everything onto the screen
  if (1== redirect){
    freopen ("/dev/tty", "a+", stdout);
  }
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
    *nl_ptr='\0';
    //strncpy(nl_ptr,"\0",1);
  }
  //  return nl_ptr;
}


int main (int argc, char *argv[]){
  while (1){
    fprintf(stdout,"mysh>");
    char input[513];
    if (NULL != fgets(input,512,stdin)){
      del_NL(input);
      int redirect=check_redict(input);
      char* token = strtok(input, " ");
      char** s_args=(char **)calloc(512,sizeof(char*));
      char** args=s_args;
      int arg_num=0;
      while (token != NULL){
	*args = token;
	args++;
        arg_num++;
	token=strtok(NULL," ");
      }

      //handle build-in
      args=s_args;
      //if user just enter "\n"
      if (0 == arg_num){
        continue;
      }
      else if (0 == strcmp(*args, "exit")){
	exit(0);
      } else if (0 == strcmp (*args, "cd")){
	//?????? how to handle more than 1 arguement is passed in? error or ignore?
        const char* path=NULL;
	if (1 == arg_num){
	  path=getenv("HOME");
	} else {
          path=*(args+1);
        }
        if (0 != chdir(path)){
          //sth wrong with reading in input
          char error_message[30] = "An error has occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
        }
      } else if (0 == strcmp (*args, "pwd")){
	char* c_wd=(char *)malloc(517); // ????? when passed in as getcwd(c_wd,sizeof(c_wd))), a NULL is returned?
	c_wd=getcwd(NULL,0);
	if (NULL == c_wd){
	  //sth wrong with reading in input
	  char error_message[30] = "An error has occurred\n";
	  write(STDERR_FILENO, error_message, strlen(error_message));
	} else {
	  fprintf(stdout,"%s\n",c_wd);
	}
        free(c_wd);
      } else if (0 == strcmp (*args, "wait")){
	//???????????? handle status of waiting for children process???
	(void) wait(NULL);
      } else {
        //hanlde non-build in. Calling program binaries
        (void)call_bin(args,arg_num);
      }
      free(s_args);
      close_redirect(redirect);
    } else {
      //sth wrong with reading in input
      char error_message[30] = "An error has occurred\n";
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
  }
}



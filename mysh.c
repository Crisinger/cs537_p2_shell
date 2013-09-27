#define _GNU_SOURCE
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
int check_ps(char ** input_ptr, int* arg_num_ptr){
  if ( 1 <= *arg_num_ptr){
    char * tmp = *(input_ptr + *arg_num_ptr -1);
    if (0 == strcmp(tmp, "&")){
      //?????????
      //the following statement doesn't chagne *(input_ptr + argnum -1) to '\0'
      //    *tmp = '\0';
      *(input_ptr + *arg_num_ptr -1)='\0';
      (*arg_num_ptr)--;
      return 1;
    } else
      return 0;
  } else {
    return 0;
  }
}

/*create a new process and call the binary*/
void call_bin(char ** input_ptr, int b_ps){
  char* bin_name_ptr;
  bin_name_ptr=*(input_ptr);
  //  int separate_ps= check_ps(input_ptr,arg_num);
  int rc=fork();
  //  printf("created child process id: %d\n",rc);
  if ( 0 ==rc){
    //child execute
    int result=execvp(bin_name_ptr,input_ptr);
    if (-1 == result){
      print_error();
      exit(1);
    }
  } else if ( 0 < rc) {
    //parent execute
    if (1 != b_ps){
      //use waitpid to wait for the particular process just created
      (void)waitpid(rc,NULL,0);
      //printf("waited children pid: %d\n",c_pid);
    }
  } else {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
}

/*check whether there is redict simple in the input.
  if do, then strip off the file name redirected into
  and redirect stdout, return 1
*/
int check_redict(char** input, int* arg_num_ptr){
  int offset=0;
  int red_s_num=0;
  char out_file[513]={0};
  int find_outfile=0;
  //record the offset to get rid of 
  int red_s_offset=-1;
  while (NULL != *(input+offset)){
    if ((0==find_outfile) && (1 == red_s_num)){
      strcpy(out_file,*(input+offset));
      find_outfile=1;
      red_s_offset=offset-1;
    }
    if (0==strcmp(*(input+offset),">")){
      red_s_num++;
    }
    offset++;
  }
  if (1 == red_s_num){
    //redirect. Do we overwrite or append?
    freopen(out_file, "w", stdout);
    *(input+red_s_offset)='\0';
    *arg_num_ptr = *arg_num_ptr-2;
    return 1;
    //    close(STDOUT_FILENO);
    //    int fd=open(out_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  } else if ( 1 < red_s_num ){
    //more than one ">"
    print_error();
  }
  return 0;
}

void close_redirect(int redirect){
  //forcefullly redirect everything onto the console screen
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
    char input[513]={0};
    if (NULL != fgets(input,513,stdin)){
      del_NL(input);

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

      const int b_ps=check_ps(s_args,&arg_num);
      const int redirect=check_redict(s_args,&arg_num);

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
      } else if (0 == strcmp(*args+strlen(*args)-3,".py")){ // fun feature
        int offset=arg_num-1;
        char extraSpace[strlen(*(args+offset))];
        //allocate space for "python" string
        *(args + arg_num) = &extraSpace[0];
        for (;offset>=0;offset--){
          strcpy(*(args+offset+1),*(args+offset));
        }
        //add "python in"
        strcpy(*args,"python");
        (void)call_bin(args,b_ps);
      } else {
        //hanlde non-build in. Calling program binaries
        (void)call_bin(args,b_ps);
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



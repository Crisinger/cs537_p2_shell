#define _GNU_SOURCE
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <errno.h>
# include <sys/types.h>




void print_error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

/* check whether we are starting a new process*/
int check_ps(char *input){
    char* cptr;
    if ((cptr= strstr(input, "&")) != NULL){
	*cptr = '\0';
	return 1;
    } else
	return 0;
} 


/*create a new process and call the binary*/
void call_bin(char ** input_ptr, int b_ps,int redirect,char** outputFile){
    char* bin_name_ptr;
    bin_name_ptr=*(input_ptr);
    //  int separate_ps= check_ps(input_ptr,arg_num);
    int rc=fork();
    //  printf("created child process id: %d\n",rc);
  

    if ( 0 ==rc){
	//child execute
	if(redirect == 1){
	    if(NULL == freopen(*outputFile, "w", stdout)){
		print_error();
		return;
	    }
	    outputFile='\0';
	} 
	int result=execvp(bin_name_ptr,input_ptr);
	if (-1 == result){
	    print_error();
	    exit(0);
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


/*check whether there is redict symbol in the input.
  if do, then strip off the file name redirected into
  and redirect stdout, return 1. If no redirect symbol, return 0
  if the input is "", then return -1;
*/
int check_redict(char * line_ptr, char** outputFile){
    char * token = strtok(line_ptr,">");

    int arg_num=0;
    while (NULL != token ){
	arg_num++;
	*outputFile=token;
	//    strcpy(outputFile,token);
	token=strtok(NULL,">");
    }
    
    

    if (1 == arg_num){ // no such token
	return 0;
    } else if (2 == arg_num){
	char* whitespaceTest = strtok(*outputFile," ");
	int whitespace_arg = 0;
	while(NULL != whitespaceTest){
	    whitespace_arg++;
	    whitespaceTest=strtok(NULL," ");
	}
    
	if(whitespace_arg != 1){
	    print_error();
	    return -1;
	}
	return 1;
    } else if (0 == arg_num){
	return -1;
    } else {
	print_error();
	return -1;
    }
}


void close_redirect(int redirect){
    //forcefullly redirect everything onto the console screen
    if (1== redirect){
	freopen ("/dev/tty", "a+", stdout);
    }
}

/*delete the new line character at the end of string */
int del_NL(char* ori){
    //get rid of the final \n mark
    char* lp;
    lp=ori;
    char* nl_ptr=(char*)(lp+strlen(lp)-1);
    if (0  != strcmp(nl_ptr,"\n")){
	return -1;
    } else {
	*nl_ptr='\0';
	return 0;
    }
    //  return nl_ptr;
}

void check_python(char **input){
    char *temp = (char *)malloc(strlen(*input));
    strcpy(temp,*input);
    char *token = strtok(*input, " ");
    if(token != NULL){
	if (0 == strcmp(token+strlen(token)-3,".py")){
	    *input = (char *)malloc(strlen(temp)+7);
	    strcpy(*input,"python ");
	    strcpy((*input+7),temp);
	}
	else{
	    strcpy(*input,temp);
	}
		
	    
	
    }
    free(temp);

}


int handler (char* input){
    int oversize = del_NL(input);
    //if the input is oversized
    if( oversize < 0){
	return -1;
    }
    check_python(&input);
    char *outputFile = "/no/such/file";
    const int b_ps=check_ps(input);
    //check redirect
    const int redirect=check_redict(input,&outputFile);
    if (-1 == redirect){ // if only "\n" is entered
	return 0;
    }
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
    if (0 == strcmp(*args, "exit")){
	if(arg_num != 1){
	    print_error();
	    return 0;
	}
	exit(0);
    } 
    else if (0 == strcmp (*args, "cd")){
	//?????? how to handle more than 1 arguement is passed in? error or ignore?
        const char* path=NULL;
	if (1 == arg_num){
	    path=getenv("HOME");
	} 
	else if (2 == arg_num){
	    path=*(args+1);
        }
	else{
	    print_error();
	    return 0;
	}

        if (0 != chdir(path)){
	    //sth wrong with reading in input
	    print_error();
	    return 0;
        }
    } 
    else if (0 == strcmp (*args, "pwd")){
	if(arg_num != 1){
	    print_error();
	    return 0;
	}
	char* c_wd=(char *)malloc(517); // ????? when passed in as getcwd(c_wd,sizeof(c_wd))), a NULL is returned?
	c_wd=getcwd(NULL,0);
	if (NULL == c_wd){
	    //sth wrong with reading in input
	    print_error();
	    return 0;
	} 
	else {
	    write(STDOUT_FILENO,c_wd,strlen(c_wd));
	    char *newline = "\n";
	    write(STDOUT_FILENO,newline,strlen(newline));
	}
        free(c_wd);
    } 
    else if (0 == strcmp (*args, "wait")){
	//int pid;
	if(arg_num != 1){
	    print_error();
	    return 0;
	}
	while (waitpid(-1, NULL, 0)) {
	    if (errno == ECHILD) {
		break;
	    }
	}
	
    } 
    else {
        //hanlde non-build in. Calling program binaries
        (void)call_bin(args,b_ps,redirect,&outputFile);
    }
    free(s_args);
    return 0;
}



int main (int argc, char *argv[]){
    int batch_mode = 0;
    FILE* fd;
    char input[515]={0};
    int oversize;
   
    if(argc > 2)
	print_error();
    if(argc == 2){
	batch_mode = 1;
        //strcpy(filename,argv[1]);
    }
    
    if(batch_mode == 1){
	fd = fopen(argv[1],"r");
	if (fd == NULL){
	    if(ferror(fd) != 0)
		print_error();
	    exit(0);
	}
    
    }
    
    while (1){
	char* rc;
	if(batch_mode == 1){
	    rc = fgets(input,514,fd);
	    if(rc  == NULL){
		exit(0);
	    }
	    if(0 != strcmp(input,"\n")){
		write(STDOUT_FILENO,input,strlen(input));
	    }
	    oversize = handler(input);
	}
	else{
	    write(STDOUT_FILENO,"mysh> ",strlen("mysh> "));
	    if (NULL != fgets(input,514,stdin)){
		oversize = handler(input);
	    } else {
		exit(0);
	    }
	}
    }

    if(batch_mode == 1)
	fclose(fd);

    return 0;
	
    
}



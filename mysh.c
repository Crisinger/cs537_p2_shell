#define _GNU_SOURCE
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <errno.h>
# include <sys/types.h>



/**
 * This function is used to print out an error message and send it to the stderr
 */
void print_error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

/**
 * This function is used to check if there is new process, which will be used in 
 * background job situation. if there is a new process, return 1, else return 0
 */
int check_ps(char *input){
    char* cptr;
    if ((cptr= strstr(input, "&")) != NULL){  //check if there is any background symbol
	*cptr = '\0';
	return 1;
    } else
	return 0;
} 


/**
 * This function is used to create a new process and call binary. In this function
 * redirection and background jobs should also work in this function.
 */
void call_bin(char ** input_ptr, int b_ps,int redirect,char** outputFile){
    char* bin_name_ptr;
    bin_name_ptr=*(input_ptr);//record the binary function name
    int rc=fork();//create a new child process
   
    if ( 0 ==rc){
	//child execute
	if(redirect == 1){//check if redirection is enabled
	    if(NULL == freopen(*outputFile, "w", stdout)){//if output file has problem, print error
		print_error();
		return;
	    }
	    outputFile='\0';
	} 
	//call execvp function and run the binary function
	int result=execvp(bin_name_ptr,input_ptr);
	if (-1 == result){ //if the result is -1,print the error message
	    print_error();
	    exit(0);
	}
    } else if ( 0 < rc) {
	//parent execute
	if (1 != b_ps){
	    //use waitpid to wait for the particular process just created
	    (void)waitpid(rc,NULL,0);
	}
    } else {//if create child failed, print error message
	print_error();
    }
}


/**
 * check whether there is redict symbol in the input.
 * if do, then strip off the file name redirected into
 * and redirect stdout, return 1. If no redirect symbol, return 0
 * if the input is "", then return -1;
 */
int check_redict(char * line_ptr, char** outputFile){
    char * token = strtok(line_ptr,">");

    int arg_num=0;
    while (NULL != token ){//if there is such a token
	arg_num++;
	*outputFile=token;
	token=strtok(NULL,">");//keep checking the ">" token
    }
    
    

    if (1 == arg_num){ // no such token
	return 0;
    } else if (2 == arg_num){//if there is only one ">" symbol
	//check second argument to see if there is only one file name
	char* whitespaceTest = strtok(*outputFile," ");
	int whitespace_arg = 0;
	while(NULL != whitespaceTest){//keep checking whitespace
	    whitespace_arg++;
	    whitespaceTest=strtok(NULL," ");
	}
    
	if(whitespace_arg != 1){//if there is more than one output file name
	    print_error();
	    return -1;
	}
	return 1;
    } else if (0 == arg_num){//if input is ""
	return -1;
    } else {
	print_error();
	return -1;
    }
}



/**
 * This function is used to delete the new line character at the end of string
 * Also it can check if the input is oversized. If it cannot detect an newline
 * char at the end of 512 byte input, it will return -1, else it will return 0
 */
int del_NL(char* ori){
    //get rid of the final \n mark
    char* lp;
    lp=ori;
    char* nl_ptr=(char*)(lp+strlen(lp)-1);
    if (0  != strcmp(nl_ptr,"\n")){//if there is no newline character at the end of input
	print_error();
	return -1;
    } else {//eliminate the newline character at the end of input
	*nl_ptr='\0';
	return 0;
    }
}

/**
 * This function is used to check if there is a pythone file is passed at the input
 * if there is a python file was passed, add a python argument before the whole input
 */
void check_python(char **input){
    char *temp = (char *)malloc(strlen(*input));//deckare a space used to store the original input
    strcpy(temp,*input);
    char *token = strtok(*input, " ");//split the input get the first argument
    if(token != NULL){
	if (0 == strcmp(token+strlen(token)-3,".py")){//check if the first argument is a python file
	    *input = (char *)malloc(strlen(temp)+7);//if it is, reallocate input
	    strcpy(*input,"python ");//add the python argument before the orignial input
	    strcpy((*input+7),temp);
	}
	else{
	    strcpy(*input,temp);//if there is no python file, restore the original input
	}
		
	    
	
    }
    free(temp);//free the temp memory

}


/**
 * This function is used to handle all the input situation. It can split the whole input and
 * determine which function should be executed.
 */
void handler (char* input){
    check_python(&input);//first check if it is a python file
    char *outputFile = "/no/such/file";
    const int b_ps=check_ps(input);//check if it is a backgroud job
    //check redirect
    const int redirect=check_redict(input,&outputFile); //check if it is a redirection job
    if (-1 == redirect){ // if only "\n" is entered
	return;
    }
    char* token = strtok(input, " "); //split the input into several arguments
    char** s_args=(char **)calloc(512,sizeof(char*));//s_args hold the start point of array of arguments
    char** args=s_args;
    int arg_num=0;//arg_num records the number of arguments
    while (token != NULL){
	*args = token;
	args++;
        arg_num++;
	token=strtok(NULL," ");
    }
    
    

    //handle build-in
    args=s_args;//restore the pointer
    if (0 == strcmp(*args, "exit")){//if an exit function was enter
	if(arg_num != 1){//if there is more than one argument, an error should be output
	    print_error();
	    return;
	}
	exit(0);
    } 
    else if (0 == strcmp (*args, "cd")){//if a cd function was enter
        const char* path=NULL;
	if (1 == arg_num){//if no path was enter, go to home direction
	    path=getenv("HOME");
	} 
	else if (2 == arg_num){//if there is a path passed
	    path=*(args+1);
        }
	else{//if there is more than one path entered, print error
	    print_error();
	    return;
	}

        if (0 != chdir(path)){//change path
	    //sth wrong with reading in input
	    print_error();
	    return;
        }
    } 
    else if (0 == strcmp (*args, "pwd")){//if a pwd function was called
	if(arg_num != 1){//if there is more than one argument, an error was printed
	    print_error();
	    return;
	}
	char* c_wd=(char *)malloc(517);//allocate a space to store the path
	c_wd=getcwd(NULL,0);//get the path
	if (NULL == c_wd){
	    //sth wrong with reading in input
	    print_error();
	    return;
	} 
	else {//if every thing is correct, print out the current path
	    write(STDOUT_FILENO,c_wd,strlen(c_wd));
	    char *newline = "\n";
	    write(STDOUT_FILENO,newline,strlen(newline));
	}
        free(c_wd);//free the temp space
    } 
    else if (0 == strcmp (*args, "wait")){//if a wait function was entered
	if(arg_num != 1){//if there is more than one argument, print error
	    print_error();
	    return;
	}
	while (waitpid(-1, NULL, 0)) {//wait until all children process is done
	    if (errno == ECHILD) {
		break;
	    }
	}
	
    } 
    else {
        //hanlde non-build in. Calling program binaries
        (void)call_bin(args,b_ps,redirect,&outputFile);
    }
    free(s_args);//free the space
}

/**
 * This is main function of this program, in this function, it can extract the input,
 * check the oversize input,determine if the program is in the batch mode.Then it will
 * call handler function and handle all inputs.
 */

int main (int argc, char *argv[]){
    int batch_mode = 0;//record if program is in batch mode
    FILE* fd;
    char input[515]={0};//space used to record input
    int oversize;//record if the input is a oversize input
   
    if(argc > 2){//if there are more than two argument, an error printed
	print_error();
	exit(1);
    }
    if(argc == 2){//if there is 2 argument, the shell is in batch mode
	batch_mode = 1;
    }
    
    if(batch_mode == 1){//if shell is in batch mode
	if ((fd = fopen(argv[1],"r")) == NULL){//open the batch file
	    print_error();//if batch file cannot open, print error and exit
	    exit(1);
	}
    
    }
    
    while (1){//main loop, keep roll until an exit command or eof 
	char* rc;
	int validInput = 0;//record if a input is valid
	if(batch_mode == 1){//if shell is in batch mode
	    rc = fgets(input,514,fd);
	    if(rc  == NULL){ //if eof was read
		exit(0); 
	    } 
	    if(0 != strcmp(input,"\n")){//if only newline character was enter, dont print it out in batch mode
		write(STDOUT_FILENO,input,strlen(input));
	    }
	    oversize = del_NL(input);//delete the newline character at the end of input and check if input is oversized
	    if(oversize < 0){//if input is oversized
		validInput = -1;//set it as an invalid input
	    }
	    while(oversize < 0){//keep reading input and print it out if input is oversized
		rc = fgets(input,514,fd);
		oversize = del_NL(input);//keep check until there is a new line character
		write(STDOUT_FILENO,input,strlen(input));
	    }
	    if(validInput < 0){//if input is oversized, it need an extra newline character
		char* newLine = "\n";
		write(STDOUT_FILENO,newLine,strlen(newLine));
	    }
	    if(validInput == 0){//if input is valid, handle it
	    	handler(input);
	    }
	     
	    	    
	}
	else{//if shell is in interactive mode
	    write(STDOUT_FILENO,"mysh> ",strlen("mysh> "));//print out the promote
	    rc = fgets(input,514,stdin);//get input
	    if(rc  == NULL){//if eof get from stream, exit the shell
		exit(0); 
	    } 
	    oversize = del_NL(input);  //check if the input is oversize
	    if(oversize < 0){  //if input is oversized
		validInput = -1;  //set it as an invalid input
	    }
	    if(validInput == 0){ //if input is valid, handle it.
	    	handler(input);
	    } 
	}
    }

    if(batch_mode == 1) //if shell is in batch mode, close the file descriptor
	fclose(fd);

    return 0;
	
    
}



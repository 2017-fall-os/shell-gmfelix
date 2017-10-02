#ifndef shell_h
#define shell_h
#include "mytoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

//Checks for the reserved word 'exit' to finish execution 
int commandCheck(char * buffer){
  if(buffer[0] && buffer[0] == 'c'){
    if(buffer[1] && buffer[1] == 'd'){
      return 2;
    }
  }
  if(buffer[0] && buffer[0] == 'e'){
    if(buffer[1] && buffer[1] == 'x'){
      if(buffer[2] && buffer[2] == 'i'){
	if(buffer[3] && buffer[3] == 't'){
	  return 0;
	}
      }
    }
  }
  return 1;
}

//Concatenates the given command to the end of the path
char* concatenatePath(char* path, char* command){
  int currPos = 0;
  int pathSize = countCharacters(path, ' ');
  int commandSize = countCharacters(command, ' ');
  char* commandPath = (char*) malloc ((pathSize+commandSize+2) * sizeof(char)); //mem allocation, +2 for the extra slash and \0

  //Begin copying first string
  for(int i = 0; path[i] != '\0'; i++){
    commandPath[currPos] = path[i];
    currPos++;
  }
  //end

  //append the extra '/'
  commandPath[currPos] = '/';
  currPos++;

  //begin copying second string
  for(int i = 0; command[i] != '\0'; i++){
    commandPath[currPos] = command[i];
    currPos++;
  }
  //end
  return commandPath;
}

//Finds the index where PATH is located in the envp array
int getPathIndex(char**envp){
  for(int i = 0; *envp[i] != '\0'; i++){
    if(envp[i][0] == 'P'){
      if(envp[i][1] == 'A'){
	if(envp[i][2] == 'T'){
	  if(envp[i][3] == 'H'){
	    if(envp[i][4] == '='){
	      return i;
	    }
	  }
	}
      }
    }
  }
  return -1; 
}

int childExecution(char**paths, char**resultingVector, char** envp){
  int executionStatus;
  int processStatus;
  while(*paths){   //Check the paths to see if the command is found
    char * commandPath = concatenatePath(*paths, resultingVector[0]);   //Concatenate the path to the command
    FILE * file;
    file = fopen(commandPath, "r");   //Try to open the file
    if(file){    //If the file exists, execute the command
      executionStatus = 1;  //If ExecStatus = 1 means that it was able to execute, 0 if not
      if(fork() == 0){
	execve(commandPath, &resultingVector[0], envp);
	exit(1);
      }else{
	wait(&processStatus);
	if(WIFSIGNALED(processStatus)){  //Check if the process ended abnormally
	  printf("Program terminated abnormally with code: %d", WTERMSIG(processStatus));
	}
	break;
      }
    }
    *paths++;
  }
  return executionStatus;
}

//Method in charge of piping, only accepts '|'
int pipeExecution(char**pipeSeparation, char**paths, char**envp){
  int fd[2];
  pid_t pid;
  char buffer [1024];
  int executionStatus;
  pipe(fd);
  pid = fork();
  if(pid == 0){ //Child 1
    close(1); //stdout closed
    dup(fd[1]); //Replaced stdout with pipe write
    close(fd[0]); //Closed pipe read
    char**resultingVector = mytoc(pipeSeparation[0], ' ');
    executionStatus = childExecution(paths, resultingVector, envp); //Process writes to stdout, which is piped to the child
    close(fd[1]);   //Clean up the pipe connections
    exit(1);
  }
  pid = fork();
  if(pid == 0){ //Child 2 
    int stdin_copy = dup(0);
    int stdout_copy = dup(1);   //Store stdin and stdout for later restoration in order for process to function
    close(0); //stdin closed
    dup(fd[0]); //Replaced stdin for pipe read
    close(fd[0]);
    close(fd[1]);
    int fileDescriptor = open("output-file", O_CREAT | O_EXCL | O_WRONLY, 0644); //Create a temporary file to feed to process
    close(1); 
    dup(fileDescriptor); //Feed the temporary file descriptor to sort
    close(fileDescriptor);
    char** resultingVector = mytoc(pipeSeparation[1], ' '); //Get the command and parameters
    dup(stdin_copy);
    dup(stdout_copy); //Restore stdout and stdin
    executionStatus = childExecution(paths, resultingVector , envp);
    remove("output-file");
    exit(1);
  }
  
  close(fd[0]);
  close(fd[1]);//Close pipe in parent
  wait(0); //Wait for the first child to terminate
  wait(0); //Wait for the second child to terminate
  return executionStatus;
}

//Method in charge of executing various commands in a string
int subCommandExecution(char**subCommandSeparation, char** paths, char** envp){
  int i = 0;
  int executionStatus = 1;
  while(subCommandSeparation[i] && executionStatus != 0){ //While there is another command and the previous command didn't failt
    char ** resultingVector = mytoc(subCommandSeparation[i], ' '); //Get the parameters for the ith command
    executionStatus = childExecution(paths, resultingVector, envp); //Execute the ith command
    i++;
  }
  return executionStatus;
}


void changeDirectory(char* directory){
  int completion = chdir(directory);
  char buffer[1024];
  if(completion == 0){
    printf("Successfully changed directory to: %s \n", getcwd(buffer, sizeof(buffer)));
  }else{
      printf("An error occurred while changing to directory: %s \n", getcwd(buffer, sizeof(buffer)));
  }
  return;
}

int main(int argc, char **argv, char**envp){
  
  int status = 1;   //Status of the loop  1 = execution || Status 0 = exit
  int processStatus;    //Detects whether the child process terminated abruptly
  while(status == 1){    
    char buffer[1024];
    pid_t pid;
    write(1, "$ ", 2);
    int bytesRead = read(0, buffer, sizeof(buffer));    //Read input from user
    buffer[sizeof(char) * bytesRead-1] = '\0';
    status = commandCheck(buffer);      //Check for commands
    if(status == 2 ){ //Status == 2  | cd has been read, changing directory
      char* directory = mytoc(buffer, ' ')[1];
      changeDirectory(directory);
      status = 1;
    }else if(status == 1){
      int executionStatus = 0;     //Determines whether the child process was created or not, error reporting purposes
      char ** pipeSeparation = mytoc(buffer, '|');
      char ** subCommandSeparation = mytoc(buffer, '&');
      if(subCommandSeparation[1]){
	int PathIndex = getPathIndex(envp);
	char ** paths = mytoc(mytoc(envp[PathIndex], '=')[1], ':'); //Tokenize the path entry, first to get rid of the PATH, then to get the actual paths
	subCommandExecution(subCommandSeparation, paths, envp);
      }else if(pipeSeparation[1]){  //If the array grew in size, there is a pipe, switch to pipe execution
	int PathIndex = getPathIndex(envp);    //Get the index for the path entry in envp
	char ** paths = mytoc(mytoc(envp[PathIndex], '=')[1], ':');   //Tokenize the path entry, first to get rid of the PATH, then to get the actual paths
	pipeExecution(pipeSeparation, paths, envp);
      }else{
	char ** resultingVector =  mytoc(buffer, ' ');    //Pass user input to the method and generate a vector
	FILE * file;
	file = fopen(resultingVector[0], "r");   //Tries to open the file to determine if it exists
	if(file){   //If file exists fork and create a child process
	  pid = fork();  
	  status = commandCheck(buffer);      //Check for commands
	  if(pid == 0 && status == 1 ){       //Child
	    execve(resultingVector[0], &resultingVector[0], envp);   //Execute the process
	    exit(2);  
	  }else{      //parent
	    executionStatus = 1;
	    pid = wait(&processStatus);
	  }
	}else if(status == 1){
	  int PathIndex = getPathIndex(envp);    //Get the index for the path entry in envp
	  char ** paths = mytoc(mytoc(envp[PathIndex], '=')[1], ':');   //Tokenize the path entry, first to get rid of the PATH, then to get the actual paths
	  executionStatus = childExecution(paths, resultingVector, envp);
	}
      }
      if(executionStatus == 0){
	printf("Command not found \n");
      }
    }else if(status == 2){ //Changing directory has been handled, return to normal execution
      status = 1;
    }else{
      return 0;
    }
    for(int i = 0; i<bytesRead; i++){     //Clean the buffer for next user input
      buffer[i] = '\0';
    }
  }
}

#endif //shell_h

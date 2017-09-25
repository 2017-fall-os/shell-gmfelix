#ifndef shell_h
#define shell_h
#include "mytoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//Checks for the reserved word 'exit' to finish execution 
int commandCheck(char * buffer){
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
    if(status == 1){
      int executionStatus = 0;     //Determines whether the child process was created or not, error reporting purposes
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
	  pid = wait(&processStatus);
	}
      }else if(status == 1){
        int PathIndex = getPathIndex(envp);    //Get the index for the path entry in envp
	char ** paths = mytoc(mytoc(envp[PathIndex], '=')[1], ':');   //Tokenize the path entry, first to get rid of the PATH, then to get the actual paths
	while(*paths){   //Check the paths to see if the command is found
	  char * commandPath = concatenatePath(*paths, resultingVector[0]);   //Concatenate the path to the command
	  FILE * file;
	  file = fopen(commandPath, "r");   //Try to open the file
	  if(file){    //If the file exists, execute the command
	    executionStatus = 1;  //If ExecStatus = 1 means that it was able to execute, 0 if not
	    pid = fork();
	    if(pid == 0){
	      execve(commandPath, &resultingVector[0], envp);
	      return 0;
	    }else{
	      wait(&processStatus);
	      if(WIFSIGNALED(processStatus)){  //CHeck if the process ended abnormally
		printf("Program terminated abnormally with code: %d", WTERMSIG(processStatus));
	      }
	      break;
	    }
	  }
	  *paths++;
	}
      }
      if(executionStatus == 0){
	printf("Command not found \n");
      }
    }else{
      return 0;
    }
    for(int i = 0; i<bytesRead; i++){     //Clean the buffer for next user input
      buffer[i] = '\0';
    }
  }
}

#endif //shell_h

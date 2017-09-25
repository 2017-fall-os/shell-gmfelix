#ifndef mytoc_h
#define mytoc_h

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytoc.h"

//Method used to count the characters, given the pointer to the starting letter and
//the delimiter, this method will count the number of characters until the delimiter has been found
//or the string has ended.
unsigned int countCharacters(char *str, char delim){
  unsigned int counter = 1;        //Counter starts at 1, since the pointer is already pointing to a letter
  char *tempPointer = str;
  while(*tempPointer && *tempPointer != delim){    //Loop while the pointer is not null and the pointer is not pointing to a delimiter
    counter++;    //A character has been found, increment the counter and keep counting
    *tempPointer++;
  }
  return counter;    //Return the final result
}

//Given a pointer to the start of a string and a delimiter, counts the number of words in the string
int countWords(char *str, char delim){
  int counter = 0;
  unsigned short readingWord = 0;
  char *tempPointer = str;
  while(*tempPointer){
    if(*tempPointer == delim){    //If the pointer is a pointing to a delimiter, we are not yet reading a word
      readingWord = 0;
    }else{
      if(readingWord == 0){    //If the pointer is not a delmiter, and we were not reading a word, the word counter increases
	counter++;
	readingWord = 1;
      }
    }                         //Otherwise, just keep reading the word
    *tempPointer++;
  }
  return counter;
}

//Given the pointer to the start of a word, return the pointer to the string with a terminal character at the end
char * copyString(char * tempPointer, int wordLength){
  char * word = (char*) malloc((wordLength+1) * (sizeof (char)));    //Mem allocation
  int counter = 0;
  char * pointer;
  while(wordLength != 1){    //While the length is not 1, 1 is used instead of 0 to adjust for the terminal character
    word[counter] = (*tempPointer);    //Dereference the value of the temporary pointer and assign it to the final result
    wordLength--;
    counter++;
    *tempPointer++;
  }
  word[counter] = '\0'; 
  pointer = word;    //Return the pointer to the beginning of the array
  return pointer;
}

//Given an array of pointers, prints the vector of strings contained in the array
void printVector(char ** resultingVector){
  write(1, "\n", 1);
  while(*resultingVector){
    char * word = *resultingVector;   //Get the pointer to a word
    while(*word){    //Cycle through the word
      write(1, word, 1);
      *word++;
    }
    write(1, "\n", 1);
    *resultingVector++;    //Increment the counter in order to get another word
  }
  write(1, "\n", 1);
}

//Given a string and a delimiter, generates an array of pointers containing the tokenized string
char ** mytoc(char *str, char delim){
  int wordCount = countWords(str, delim);    //Count the number of words in the string
  char ** resultingVector =  (char**) malloc((wordCount+1) * sizeof(char *));   //Mem allocation, +1 to compensate for '\0'
  char ** tempVectorPointer = resultingVector;
  int i = 0;
  while(*str){
    if(*str != delim){   //If the current character is not a delimiter, then we must copy this word to the vector
      int wordLength = countCharacters(str, delim);    //Get the size of the word
      char * word = (char*) malloc((wordLength+1) * sizeof(char));    //Mem allocation for the word, +1 to compensate for '\0'
      word = copyString(str, wordLength);    //Get a pointer to the new word
      resultingVector[i] = word;             //Assign it to our resulting vector
      str = (str + (wordLength-1));          //Skip the word that we just copied and continue with the next
      i++;
    }else{
    *str++;
    }
  }
  resultingVector[i] = '\0';                 //Final terminal symbol before returning
  return resultingVector;
}


#endif // mytoc_h

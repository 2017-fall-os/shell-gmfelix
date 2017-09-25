#ifndef mytoc_h
#define mytoc_h

#include <stdio.h>

char ** mytoc(char *str, char delim);

unsigned int countWords(char *str, char delim);

unsigned int countCharacters(char *str, char delim);
void printVector(char ** resultingVector);
#endif //mytoc_h

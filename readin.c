#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
#include <ctype.h>
#include "opcode.h"

// Main array of processor, acting as memory and stack as well
int* byteArray;
char buffer[2048];
int byteArrayCount = 0x100;

void removeSpaces(char* source){
	char* result = source;
	char* current = source;
	while(*current != 0)
	{
		*result = *current++;
		if(*result != ' ')
			result++;
	}
	*result = 0;
}

int copyBytes(int start, int end){
	for(int i = start; i<end; i++){
		int currentByte = buffer[i] & 0xff;
		*(byteArray+byteArrayCount) = currentByte;
		byteArrayCount++;
	}
}

void readBinary(){
	byteArray = (int*) malloc(sizeof(int)*0xFFFF);
	FILE *inputFile;
	char line[20];

	inputFile = fopen("conway.exc", "rb");

	if (inputFile == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	bool found = false;
	while(fgets(line, 20, inputFile)){
		if(!strcmp(line, "ENDHEADER\n")){
			found = true;
			break;
		}
	}

	if(!found){
		fclose(inputFile);
		inputFile = fopen("sieve.exc", "rb");

	}

	int byteRead = fread(buffer,1,2048,inputFile);
	copyBytes(0, byteRead);

	while(byteRead == 2048){
		byteRead = fread(buffer,1,2048,inputFile);
		copyBytes(0, byteRead);
	}

	for(int i = 0x100; i < byteArrayCount + 1; i= i + 4){
		printf("0x%04x %02x %02x %02x %02x\n",i, byteArray[i], byteArray[i+1],byteArray[i+2],byteArray[i+3]);
	}
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
#include <ctype.h>
#include "opcode.h"

// Main array of processor, acting as memory and stack as well
int* byteArray;

//Bytes read will be stored temporarily in buffer, then converted 
// to hex and stored in byte array
char buffer[2048];

int byteArrayCount = 0x100;

// Remove all whitespaces from strings, used for formatting purposes
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

// Copy bytes from buffer to main byte array
int copyBytes(int end){
	for(int i = 0; i<end; i++){
		int currentByte = buffer[i] & 0xff;
		*(byteArray+byteArrayCount) = currentByte;
		byteArrayCount++;
	}
}

void readBinary(FILE* inputFile){
	byteArray = (int*) malloc(sizeof(int)*0xFFFF);
	char line[20];

	bool found = false;
	while(fgets(line, 20, inputFile)){
		if(!strcmp(line, "ENDHEADER\n")){	// If ENDHEADER TAG found, keep pointer at a certain address
			found = true;
			break;
		}
	}

	// If ENDHEADER not found, assume file is all binary
	// Restart the file and start reading

	if(!found){		
		// fclose(inputFile);
		fseek(inputFile, 0, SEEK_SET);
	}

	int byteRead = fread(buffer,1,2048,inputFile);
	copyBytes(byteRead);

	//Check if there are still more to read
	while(byteRead == 2048){
		byteRead = fread(buffer,1,2048,inputFile);
		copyBytes(byteRead);
	}

	for(int i = 0x100; i < byteArrayCount + 1; i= i + 4){
		// printf("0x%04x %02x %02x %02x %02x\n",i, byteArray[i], byteArray[i+1],byteArray[i+2],byteArray[i+3]);
	}
}

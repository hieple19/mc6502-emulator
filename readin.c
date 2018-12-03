#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include "opcode.h"

int* byteArray;
char buffer[2048];
int byteArrayIndex = 0x100;

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

int byteCount(){
	return byteArrayIndex - 0x100;
}
int checkForHeader(){
	for(int i=0; i<2048; i++){
		if(	buffer[i] == 'E'
			&& buffer[i+1] == 'N'
			&& buffer[i+2] == 'D'
			&& buffer[i+3] == 'H'
			&& buffer[i+4] == 'E'
			&& buffer[i+5] == 'A'
			&& buffer[i+6] == 'D'
			&& buffer[i+7] == 'E'
			&& buffer[i+8] == 'R'
			&& buffer[i+9] == 10
			){
				return i+10;
		}
	}
}

int copyBytes(int start, int end){
	for(int i = start; i<end; i++){
		int currentByte = buffer[i] & 0xff;
		*(byteArray+byteArrayIndex) = currentByte;
		byteArrayIndex++;
	}
}

	
void readBinary(){
	byteArray = (int*) malloc(sizeof(int)*0xFFFF);
	FILE *inputFile;

	inputFile = fopen("conway.exc", "rb");

	if (inputFile == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	int byteRead = fread(buffer,1,2048,inputFile);

	int index = checkForHeader();
	// printf("%d\n", byteRead );
	copyBytes(index, byteRead);

	while(byteRead == 2048){
		byteRead = fread(buffer,1,2048,inputFile);
		copyBytes(0, byteRead);
	}

	printf("%d\n", byteCount());
	for(int i = 0x100; i < 0x100 + byteCount(); i= i + 4){
		// printf("%02x %02x %02x %02x\n",byteArray[i], byteArray[i+1],byteArray[i+2],byteArray[i+3]);
	}
 //   	fseek( inputFile, 0, SEEK_SET );


	// while(1){
	// 	if(feof(inputFile)){
	// 		break;
	// 	}
	// 	char hex[8];
	// 	fread(buffer,1,1,inputFile);
	// 	// printf("%02x\n", currentByte );
	// 	*(byteArray + count ) =  currentByte;
	// 	count++;
	// }
}

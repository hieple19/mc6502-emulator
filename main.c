#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <stdbool.h>
#include "opcode.h"
#include "readin.h"
#include "machine.h"

void instructions(){
	printf("e 		Step one step\n" );
	printf("p 		Print current data\n");
	printf("c $xxxx 	Continue to an address (Min: 0x0, Max: 0xFFFF)\n");
	printf("e $xxxx 	Examine an address (Min: 0x0, Max: 0xFFFF)\n");
	printf("x 		Exit program\n");
}

// Examine byte at a certain location
// Address in format $xxxx
void examineAddress(char* address){
	if(*address != '$' || address == 0 ){
		printf("Error with address \n");
	}
	else{
		*address++;	// Remove $
		removeSpaces(address);
		address[strlen(address)-1] = 0;
		int hexCode = (int)strtol(address,NULL,16);
		if(hexCode >= 0x0 && hexCode <= 0xFFFF){
			printf("Byte at $%04x: %02x\n", hexCode, byteArray[hexCode]);
		}
		else{
			printf("Address out of range \n");
		}
	}
}

// Continue to a certain address
// Address in format $xxxx
void continueCommand(char* address){
	if(*address != '$' || address == 0 ){
		printf("Error with address \n");
	}
	else{
		removeSpaces(address);
		*address++;
		int hexCode = (int)strtol(address,NULL,16);
		if(hexCode >= 0x100 && hexCode <= byteArrayCount){
			continueToAddress(hexCode);		// Continue to addresss only if within program range of bytes
		}
		else{
			printf("Address out of program range \n");
		}
	}
}

int main(int argc, char* argv[]){
	FILE *inputFile = fopen(argv[1], "rb");

	if (inputFile == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	processOpCodes();

	readBinary(inputFile);
	char buffer[15];
	instructions();
	while(pc < byteArrayCount + 1){
		printf("\nEnter new instruction: ");
		fgets(buffer, 15, stdin );
		printf("\n");
		char* command = strtok(buffer, " ");
		removeSpaces(command);
		command[1] = 0;

		if(!strcmp(command, "s")){
			updatePC();
		}
		else if(!strcmp(command, "x")){
			break;
		}
		else if(!strcmp(command, "p")){
			printData();
		}
		else if(!strcmp(command, "e")){
			char* address = strtok(0," ");
			if(address != 0){
				examineAddress(address);
			}
			else{
				printf("Invalid Address\n");
			}
		}

		else if(!strcmp(command, "c")){
			char* address = strtok(0," ");
			if(address != 0){
				continueCommand(address);
			}
			else{
				printf("Invalid Address\n");
			}
		}
		else{
			printf("Invalid Instruction\n");
		}
	}
	fclose(inputFile);
	return 0;
}
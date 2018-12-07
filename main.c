#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <stdbool.h>
#include "opcode.h"
#include "readin.h"
#include "machine.h"

int main(){
	processOpCodes();
	initializeFunctions();
	readBinary();
	int i = 0;
	char buffer[15];
	while(pc < byteArrayCount + 1){
		printf("\nEnter new instruction: ");
		fgets(buffer, 15, stdin );
		printf("\n");
		char* command = strtok(buffer, " ");
		removeSpaces(command);
		command[1] = 0;
		// int hexCode = (int)strtol(strtok(0," "),NULL,0);	// Copy hex Code to opCode[] array of

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
			if(*address != '$' || address == 0 ){
				printf("Error with address \n");
			}
			else{
				*address++;
				removeSpaces(address);
				address[strlen(address)-1] = 0;
				printf("String %s\n", address);

				int hexCode = (int)strtol(address,NULL,16);
				printf("hex is %x \n", hexCode);
				if(hexCode >= 0 && hexCode <= 0xFFFF){
					printf("Byte at $%02x: %02x\n", hexCode, byteArray[hexCode]);
				}
				else{
					printf("Error with address \n");
				}
			}
		}

		else if(!strcmp(command, "c")){
			char* address = strtok(0," ");
			if(*address != '$' || address == 0 ){
				printf("Error with address \n");
			}
			else{
				removeSpaces(address);
				*address++;
				printf("%s\n", address);
				int hexCode = (int)strtol(address,NULL,16);
				printf("%x\n", hexCode);
				if(hexCode >= 0x100 && hexCode <= byteArrayCount){
				continueToAddress(hexCode);
				}

				else{
					printf("Error with address \n");
				}
			}
		}
		else{
			printf("Invalid Instruction\n");
		}
		
		if(i == 120){
			break;
		}
		i++;
	}
	return 0;
}
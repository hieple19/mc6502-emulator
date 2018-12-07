#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include "opcode.h"

// Code list contains hex code
// Each OpCode object has a name i.e. ADC
OpCode codeList[256];
char* codeNames[56];

char* modes[12] = {"Imm", "ZP", "ZPX", "ZPY","Ind", "IndX", "IndY", "Acc", "Abs", "AbsX", "AbsY", "Bran"};

// Process BRK, special case as OpCode is 0 same as NULL
void processBreak(){
	codeNames[12] = "BRK";
	codeList[0].hexCode = 0;
	
	strcpy(codeList[0].name,"BRK");
	strcpy(codeList[0].mode, "Acc");
	codeList[0].noBytes = 1;
}
void processOneOpCode(char* line, int index){
	char name[3];
	codeNames[index] = (char*) malloc(sizeof(char)*3);

	strcpy(name,strtok(line," "));		// Name of hexCode. Various hex codes can have same name
	strcpy(codeNames[index],name);		// Store name into code names array

	for(int i =0; i<12; i++){
		int hexCode = (int)strtol(strtok(0," "),NULL,0);	// Copy hex Code to opCode[] array of
		if(hexCode != 0){
			codeList[hexCode].hexCode = hexCode;

			strcpy(codeList[hexCode].name,name);
			strcpy(codeList[hexCode].mode, modes[i]);		// Copy name and mode to list hex op codes

			switch(i){
				case 8:
				case 9:
				case 10:
				case 4:
				codeList[hexCode].noBytes = 3;			// Set length of each op codes
				break;

				case 7:
				codeList[hexCode].noBytes = 1;
				break;

				default:
				codeList[hexCode].noBytes = 2;
			}
		}
	}
} 

// Read and process 56 instructions
void processOpCodes(){
	char line[70];
	FILE *fptr;

	fptr = fopen("opcode.txt", "r");
	if (fptr == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	fgets(line, sizeof(line), fptr);

	int index = 0;
	processBreak();
	while(fgets(line, sizeof(line), fptr)){
		processOneOpCode(line, index);
		index++;
	}
}
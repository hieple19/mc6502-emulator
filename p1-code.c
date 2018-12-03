#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h>
#include <ctype.h>

#include "readin.h"
#include "check.h"
#include "opcode.h"
#include "format.h"
#include "symbol.h"
#include "process.h"


OpCode* codeList[56];			// Code list stores 56 opcodes 

// OpCode struct has an array of ints which holds the hex op code for each addresing mode
// If op code does not support op code, hex op code is NULL 

void processOneOpCode(char* line, OpCode** opCodeList, int index){
	*(opCodeList + index) = (OpCode*)malloc(sizeof(OpCode));		// Initialize array[index] to hold op code

	OpCode* opCode = *(opCodeList + index);
	strcpy(opCode->name, strtok(line," "));						// Copy name from file to OpCode struct

	for(int i =0; i<12; i++){
		opCode->code[i] = (int)strtol(strtok(0," "),NULL,0);	// Copy hex Code to opCode[] array of
	}
} 

// Read OpCodes info
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

	int i = 0;
	while(fgets(line, sizeof(line), fptr)){
		processOneOpCode(line, codeList, i);
		i++;
	}
}

// Given string opCode, find corresponding line in the file
int searchOpIndex(char* opCode){
	for(int i = 1; i<57; i++){
		if(!strcmp(opCode, codeList[i]->name)){
			return i;
		}
	}
	return 0;
}

// Given string opCode and operand lists, find corresponding hex op code
int opCodeHex(char* opCode, StringList* operandList){
	int opCodeIndex = searchOpIndex(opCode);
	OpCode* opCodeInfo = codeList[opCodeIndex];			// Access corresponding op code

	int operandType = checkType(opCode,operandList);	// Return index in the code[] array of each opcode

	int res = opCodeInfo->code[operandType];
	return res;
}




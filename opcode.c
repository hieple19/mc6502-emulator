#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include "opcode.h"

OpCode codeList[256];
char* codeNames[56];
// Read OpCodes info
char* modes[12] = {"Imm", "ZP", "ZPX", "ZPY","Ind", "IndX", "IndY", "Acc", "Abs", "AbsX", "AbsY", "Bran"};
void processOneOpCode(char* line, int index){
	char name[3];
	codeNames[index] = (char*) malloc(sizeof(char)*3);
	strcpy(name,strtok(line," "));
	strcpy(codeNames[index],name);

	for(int i =0; i<12; i++){
		int hexCode = (int)strtol(strtok(0," "),NULL,0);	// Copy hex Code to opCode[] array of
		if(hexCode != 0){
			codeList[hexCode].hexCode = hexCode;

			strcpy(codeList[hexCode].name,name);
			strcpy(codeList[hexCode].mode, modes[i]);
			switch(i){
				case 8:
				case 9:
				case 10:
				case 4:
				codeList[hexCode].noBytes = 3;
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
	while(fgets(line, sizeof(line), fptr)){
		processOneOpCode(line, index);
		index++;
		
	}
}

int lmain(){
	processOpCodes();
	for(int i = 0; i<56; i++){
		printf("%s\n", codeNames[i]);
	}
	for (int i = 0; i < 256; ++i)
	{
		if(codeList[i].noBytes != 0){
		printf("%d ", i );
		printf("Name %s, ", codeList[i].name);
		printf(", Mode %s, ", codeList[i].mode);
		printf(", Hex 0x%x, ", codeList[i].hexCode);
		printf(", Length %d", codeList[i].noBytes);
		printf("\n");

	}
	}
}
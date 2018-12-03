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
	char command[1];
	while(pc - 0x100 < byteCount() && !endFile){
		scanf("%s", command);
		if(!strcmp(command, "c")){
			updatePC();
		}
		// updatePC();
		if(i == 15){
			// break;
		}
		i++;
		printf("\n");
		printf("i is %d\n",i);
		// printf("0xf0 is 0x%x\n", byteArray[0xf0] );
		// printf("0xf1 is 0x%x\n", byteArray[0xf1] );
		printf("\n");
	}

	// printf("0xf0 is 0x%02x\n", byteArray[0xf0] );
	// printf("0xf1 is 0x%02x\n", byteArray[0xf1] );

	return 0;
}
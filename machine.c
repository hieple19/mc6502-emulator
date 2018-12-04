#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#include "readin.h"
#include "opcode.h"

int pc = 0x0100;
// int pc = 0;
int stack[256];
int acc = 0;
int* acc_ptr = &acc;
int X = 0;
int* x_ptr = &X;
int Y = 0;
int* y_ptr = &Y;
int SP = 0xFF;
bool endFile = false;

int CF = 0;
int VF = 0;
int ZF = 0;
int NF = 0;
int IF = 0;
int DF = 0;

int findPositionStack(){
	return SP & 0xFF;
}

int translate8bitInts(int normal){
	int bit7 = (-1) * ((normal & 0x80) >> 7) * pow(2,7);
	int bit6 = ((normal & 0x40) >> 6) * pow(2,6);
	int bit5 = ((normal & 0x20) >> 5) * pow(2,5);
	int bit4 = ((normal & 0x10) >> 4) * pow(2,4);
	int bit3 = ((normal & 0x08) >> 3) * pow(2,3);
	int bit2 = ((normal & 0x04) >> 2) * pow(2,2);
	int bit1 = ((normal & 0x02) >> 1) * pow(2,1);
	int bit0 = (normal & 0x01) * pow(2,0);
	
	return bit7 + bit6 + bit5 + bit4 + bit3 + bit2 + bit1 + bit0;
}

int checkCF(int no){
	if(no > 0xFF){
		no = no & 0xFF;
		CF = 1;
		return no;
	}
	CF = 0;
	return no;
}

// bool checkCF(int actual, int carry){
// 	if(actual != carry){
// 		CF = 1;
// 		return true;
// 	}
// 	return false;
// }

bool checkZF(int reg){
	if(reg == 0){
		ZF = 1;
		return true;
	}
	ZF = 0;
	return false;
}

bool checkNF(int reg){
	if ((reg & 0x80) == 0x80){
		NF = 1;
		return true;
	}
	NF = 0;
	return false;
}

int getValue(int address){
	return byteArray[address];
}

void AND(int* operands,char* type){
	if(!strcmp(type,"Imm")){
		acc = acc & operands[0];
	}
	else if(!strcmp(type,"ZP")){
		acc = acc & getValue(operands[0]);

	} else if(!strcmp(type,"ZPX")){
		int lookup = byteArray[operands[0]] + X;
		acc = acc & getValue(lookup);

	} else if(!strcmp(type,"Abs")){
		acc = acc & getValue(operands[0] + (operands[1]<<8));

	} else if(!strcmp(type,"AbsX")){
		int lookup = byteArray[operands[0] + (operands[1]<<8)] + Y;
		acc = acc & getValue(lookup);

	} else if(!strcmp(type,"AbsY")){
		int lookup = byteArray[operands[0] + (operands[1]<<8)] + Y;
		acc = acc & getValue(lookup);

	} else if(!strcmp(type,"IndX")){
		int lookupIndex = operands[0] + X;
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8;

		int address = byte1 + byte2;

		acc = acc & byteArray[address];
	} else if(!strcmp(type,"IndY")){
		int lookupIndex = operands[0];
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8; 
		int address = byte1 + byte2 + Y;
		acc = acc & byteArray[address];
	}
	checkZF(acc);
	checkNF(acc);
}

void loadRegister(int* operands,char* type, int* reg){
	if(!strcmp(type,"Imm")){
		*reg = operands[0];
	}
	else if(!strcmp(type,"ZP")){
		*reg = getValue(operands[0]);

	} else if(!strcmp(type,"ZPX")){
		int lookup = byteArray[operands[0]] + X;
		*reg = getValue(lookup);

	} else if(!strcmp(type,"Abs")){
		*reg = getValue(operands[0] + (operands[1]<<8));

	} else if(!strcmp(type,"AbsX")){
		int lookup = byteArray[operands[0] + (operands[1]<<8)] + X;
		*reg = getValue(lookup);

	} else if(!strcmp(type,"AbsY")){
		int lookup = byteArray[operands[0] + (operands[1]<<8)] + Y;
		*reg = getValue(lookup);

	} else if(!strcmp(type,"IndX")){
		int lookupIndex = operands[0] + X;
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8;

		int address = byte1 + byte2;
		*reg = byteArray[address];

	} else if(!strcmp(type,"IndY")){
		int lookupIndex = operands[0];
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8; 
		int address = byte1 + byte2 + Y;
		*reg = byteArray[address];
	}
	checkZF(*reg);
	checkNF(*reg);
}
void LDA(int* operands,char* type){
	loadRegister(operands, type, acc_ptr);
}

void LDX(int* operands,char* type){
	loadRegister(operands,type,x_ptr);
}

void LDY(int* operands,char* type){
	loadRegister(operands,type,y_ptr);
}
void storeRegister(int *operands, char* type, int* reg){
	if(!strcmp(type,"ZP")){
		byteArray[operands[0]] = *reg;

	} else if(!strcmp(type,"ZPX")){
		int storeAt = operands[0] + X;
		byteArray[storeAt] = *reg;

	} else if(!strcmp(type,"AbsX")){
		// printf("%04x\n", operands[0] );
		// printf("%04x\n", operands[1]<<8);
		// printf("%x\n", X);
		int storeAt = operands[0] + (operands[1]<<8) + X;
		printf("store at 0x%04x\n", storeAt );
		byteArray[storeAt] = *reg;

	} else if(!strcmp(type,"Abs")){
		int storeAt = operands[0] + operands[1]<<8;
		byteArray[storeAt] = *reg;

	} else if(!strcmp(type,"AbsY")){
		int storeAt = operands[0] + operands[1]<<8 + Y;
		byteArray[storeAt] = *reg;

	} else if(!strcmp(type,"IndX")){
		int lookupIndex = operands[0] + X;
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8;

		int address = byte1 + byte2;

		byteArray[address] = *reg;

	} else if(!strcmp(type,"IndY")){
		int lookupIndex = operands[0];
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8; 
		int address = byte1 + byte2 + Y;
		
		byteArray[address] = *reg;
	}
}
void STA(int* operands,char* type){
	storeRegister(operands, type,acc_ptr);
}

void STX(int* operands,char* type){
	storeRegister(operands, type, x_ptr);
}

void STY(int* operands,char* type){
	storeRegister(operands, type, y_ptr);
}

void INC(int* operands, char* type){
	int value;
	if(!strcmp(type,"ZP")){
		byteArray[operands[0]]++;
		value = getValue(operands[0]);

	} else if(!strcmp(type,"ZPX")){
		int lookup = byteArray[operands[0]] + X;
		byteArray[lookup]++;
		value = getValue(lookup) + 1;

	} else if(!strcmp(type,"Abs")){
		int lookup = byteArray[operands[0] + operands[1]<<8];
		byteArray[lookup]++;
		value = getValue(lookup) + 1;
	} else if(!strcmp(type,"AbsX")){
		int lookup = byteArray[operands[0] + operands[1]<<8] + X;
		byteArray[lookup]++;
		value = getValue(lookup) + 1;
	}
}

void INX(int* operands, char* type){
	X++;
	checkNF(X);
	checkZF(X);
}

void INY(int* operands, char* type){
	Y++;
	checkNF(X);
	checkZF(X);
}

void JSR(int* operands, char* type){
	int pos = findPositionStack();
	SP--;
	stack[pos] = pc + 2; 
	pc = operands[0] + operands[1]<<8;
}

void RTS(int* operands, char* type){
	int pos = (SP + 1) & 0xFF;
	SP++;
	printf("pos %x\n", pos );
	printf("pos %x\n", stack[pos] );

	pc = stack[pos] + 1;
	printf("pc %x\n", pc);
	stack[pos] = 0;
	return;
}

void ADC(int* operands, char* type){
	int sum;
	if(!strcmp(type,"Imm")){
		sum = operands[0] + CF + acc;
	}
	
	else if(!strcmp(type,"ZP")){
		sum = getValue(operands[0]) + acc + CF;

	} else if(!strcmp(type,"ZPX")){
		int lookup = byteArray[operands[0]] + X;
		sum = getValue(lookup) + acc + CF;

	} else if(!strcmp(type,"Abs")){
		sum = getValue(operands[0] + (operands[1]<<8)) + acc + CF;

	} else if(!strcmp(type,"AbsX")){
		int lookup = byteArray[operands[0] + (operands[1]<<8)] + X;
		sum = getValue(lookup) + acc + CF;

	} else if(!strcmp(type,"AbsY")){
		int lookup = byteArray[operands[0] + (operands[1]<<8)] + Y;
		sum = getValue(lookup) + acc + CF;

	} else if(!strcmp(type,"IndX")){
		int lookupIndex = operands[0] + X;
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8;

		int address = byte1 + byte2;
		sum = byteArray[address] + acc + CF;

	} else if(!strcmp(type,"IndY")){
		int lookupIndex = operands[0];
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8; 
		int address = byte1 + byte2 + Y;
		sum = byteArray[address] + acc + CF;
	}
	sum = checkCF(sum);
	acc = sum;
	checkZF(acc);
	checkNF(acc);
}

void CPX(int* operands, char* type){
	int compare;
	if(!strcmp(type,"Imm")){
		compare = operands[0];
	} else if(!strcmp(type,"ZP")){
		compare = getValue(operands[0]);
	} else if(!strcmp(type,"Abs")){
		compare = getValue(operands[0] + operands[1]<<8);
	}

	if(X>=compare){
		CF = 1;
	}
	if(X == compare){
		CF =1;
	}
	checkNF(X-compare);
}

void CPY(int* operands, char* type){
	int compare;
	if(!strcmp(type,"Imm")){
		compare = operands[0];
	} else if(!strcmp(type,"ZP")){
		compare = getValue(operands[0]);
	} else if(!strcmp(type,"Abs")){
		compare = getValue(operands[0] + operands[1]<<8);
	}

	if(Y>=compare){
		CF = 1;
	}
	if(Y == compare){
		CF =1;
	}
	checkNF(Y-compare);
}


void CMP(int* operands, char* type){

	int compare;
	if(!strcmp(type,"Imm")){
		compare = operands[0];
	}
	
	else if(!strcmp(type,"ZP")){
		compare = getValue(operands[0]);

	} else if(!strcmp(type,"ZPX")){
		int lookup = byteArray[operands[0]] + X;
		compare = getValue(lookup);

	} else if(!strcmp(type,"Abs")){
		compare = getValue(operands[0] + operands[1]<<8);

	} else if(!strcmp(type,"AbsX")){
		int lookup = byteArray[operands[0] + operands[1]<<8] + X;
		compare = getValue(lookup);

	} else if(!strcmp(type,"AbsY")){
		int lookup = byteArray[operands[0] + operands[1]<<8] + Y;
		compare = getValue(lookup);

	} else if(!strcmp(type,"IndX")){
		int lookupIndex = operands[0] + X;
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8;

		int address = byte1 + byte2;
		compare = byteArray[address];

	} else if(!strcmp(type,"IndY")){
		int lookupIndex = operands[0];
		int byte1 = byteArray[lookupIndex];
		int byte2 = (byteArray[lookupIndex+1])<<8; 
		int address = byte1 + byte2 + Y;
		compare = byteArray[address];
	}

	if(acc>=compare){
		CF = 1;
	}
	if(acc == compare){
		CF =1;
	}
	checkNF(acc - compare);
}

void BRK(int* operands, char* type){}

void BMI(int* operands, char* type){
	// translate8bitInts
	if(NF == 1){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}

void BPL(int* operands, char* type){
	// translate8bitInts
	if(NF == 0){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}


void BEQ(int* operands, char* type){
	// translate8bitInts
	if(ZF == 1){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}

void BVS(int* operands, char* type){
	// translate8bitInts
	if(VF == 1){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}

void BVC(int* operands, char* type){
	// translate8bitInts
	if(VF == 0){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}

void BNE(int* operands, char* type){
	// translate8bitInts
	if(ZF == 0){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}

void CLC(int* operands, char* type){
	CF = 0;
}

void CLV(int* operands, char* type){
	VF = 0;
}

void CLI(int* operands, char* type){
	IF = 0;
}

void CLD(int* operands, char* type){
	DF = 0;
}



void JMP(int* operands, char* type){
	if(!strcmp(type,"Ind")){
		int target = operands[0] + operands[1]<<8;

		int lsb = byteArray[target];
		int msb = byteArray[target+1]<<4;

		pc = byteArray[lsb + msb];
	}
	else if(!strcmp(type,"Abs")){
		int target = operands[0] + operands[1]<<8;
		pc = target;
	}
}
void TYA(int* operands, char* type){
	acc = Y;
	checkZF(acc);
	checkNF(acc);
}

void TAY(int* operands, char* type){
	Y = acc;
	checkZF(Y);
	checkNF(Y);
}



void (*ftable[56])(int* operands,char*mode);

int findIndex(char* code){
	for (int i = 0; i < 56; ++i)
	{
		if(!strcmp(codeNames[i],code)){
			return i;
		}
	}
	return -1;
}

void initializeFunctions() {
	ftable[0] = ADC;
	ftable[1] = AND;
	ftable[4] = BPL;
	ftable[5] = BMI;
	ftable[14] = CPX;
	ftable[27] = JSR;
	ftable[28] = LDA;
	ftable[29] = LDX;
	ftable[30] = LDY;
	ftable[37] = INX;
	ftable[38] = INY;
	ftable[39] = TYA;
	ftable[45] = RTS;
	ftable[47] = STA;
	ftable[54] = STX;
}

void processLine(int opCode, int noBytes){
	int* operands = (int*)malloc(sizeof(int)*(noBytes-1));

	for(int i = 1; i<noBytes; i++){
		printf("In here %x\n", byteArray[pc+i] );
		*(operands+i-1) = byteArray[pc+i];
	}
	// printf("%x\n", operands[0]);
	// printf("%d\n", noBytes);
	char* name = codeList[opCode].name;
	char* mode = codeList[opCode].mode;
	if(findIndex(name) == -1){
		endFile = true;
		return;
	}
	// printf("%s\n", name );
	// printf("%s\n", mode);
	printf("Ftable function is %d\n", findIndex(name));
	ftable[findIndex(name)](operands, mode);

}
void printData(){
	printf("A = $%02x \n", acc);
	// printf("ptr %02x\n", *acc_ptr);
	printf("X = $%02x \n", X);
	printf("Y = $%02x \n", Y);
	printf("NF = %d\n", NF);
	printf("ZF = %d\n", ZF);
	printf("CF = %d\n", CF);
	printf("SP = %02x\n", SP);
	printf("PC = 0x%04x\n", pc+(0x600 - 0x100));
}

// RUN PROGRAM AND UPDATE 
void updatePC(){
	int opCodeCurr = byteArray[pc];
	// printf("%s\n", );
	int noBytes = codeList[opCodeCurr].noBytes;
	int print = (0x600 - 0x100)+pc;
	// int print = pc;
	printf("0x%04x ", print);
	for(int i = 0; i<noBytes; i++){
		printf("%02x ", byteArray[pc+i]);
	}
	printf("\n");
	int current = pc;
	processLine(opCodeCurr, noBytes);
	
	if(current == pc){
		pc = pc + noBytes;
	}
	printData();

}
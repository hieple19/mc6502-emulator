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
int acc = 0;
int* acc_ptr = &acc;
int X = 0;
int* x_ptr = &X;
int Y = 0;
int* y_ptr = &Y;
int SP = 0xFFFF;
bool endFile = false;

int CF = 0;
int *cf_ptr = &CF;

int ZF = 0;
int *zf_ptr = &ZF;

int IF = 0;
int DF = 0;
int BF = 0;
int VF = 0;

int *vf_ptr = &VF;

int NF = 0;
int *nf_ptr = &NF;

int checkCF(int no){
	if(no > 0xFF){
		no = no & 0xFF;
		CF = 1;
		return no;
	}
	CF = 0;
	return no;
}

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

int add8bits(int a, int b){
	int sum = a + b + CF;
	int signA = a & 0x80;
	int signB = b & 0x80;
	if(sum > 0xFF){
		sum = sum & 0xFF;
		CF = 1;
	}
	int signSum = sum & 0x80;
	if(signA == signB){
		if(signSum != signA){
			VF = 1;
		}
	}
	return sum;
}

int subtract8bits(int a, int b){
	int diff = a - b - (1 - CF);
	int signA = a & 0x80;
	int signB = b & 0x80;
	if(diff > 0xFF){
		diff = diff & 0xFF;
		CF = 0;
	}
	int signDiff = diff & 0x80;
	if(signA == signB){
		if(signDiff != signA){
			VF = 1;
		}
	}
	checkZF(diff);
	checkNF(diff);
	return diff;
}

int getAddress(int* operands, int number){
	if(number == 1){
		return byteArray[operands[0]];
	}
	else{
		return byteArray[operands[0] + (operands[1]<<8)];
	}
}

int addressAbs(int* operands, int reg){
	return (operands[0] + (operands[1]<<8) + reg) & 0xFFFF;
}

int addressZP(int* operands, int reg){
	return (operands[0] + reg) & 0xFF;
}

int addressIndX(int* operands){
	int lookupIndex = (operands[0] + X) & 0xFF;
	int lowByte = byteArray[lookupIndex];
	int highByte = (byteArray[lookupIndex+1])<<8;

	return lowByte + highByte;
}

int addressIndY(int* operands){
	int lookupIndex = operands[0];
	int byte1 = byteArray[lookupIndex];
	int byte2 = (byteArray[lookupIndex+1])<<8; 
	int address = (byte1 + byte2 + Y) & 0xFFFF;

	return address;
}


int arrayIndex(int* operands, char* type){
	int storeAt;
	if(!strcmp(type,"ZP")){
		storeAt = addressZP(operands, 0);

	} else if(!strcmp(type,"ZPX")){
		// storeAt = (operands[0] + X) & 0xFF;
		storeAt = addressZP(operands, X);

	} else if(!strcmp(type,"ZPY")){
		storeAt = addressZP(operands, Y);

	} else if(!strcmp(type,"Abs")){
		storeAt = addressAbs(operands, 0);

	} else if(!strcmp(type,"AbsX")){

		storeAt = addressAbs(operands, X);

	}else if(!strcmp(type,"AbsY")){

		storeAt = addressAbs(operands,Y);

	} else if(!strcmp(type,"IndX")){

		storeAt = addressIndX(operands);

	} else if(!strcmp(type,"IndY")){

		storeAt = addressIndY(operands);
	}
	printf("Array index is 0x%04x\n", storeAt );
	return storeAt;
}


void ADC(int* operands, char* type){
	if(!strcmp(type,"Imm")){
		acc = add8bits(operands[0], acc);
	}
	else{
		int storeAt = arrayIndex(operands, type);
		int sum = add8bits(byteArray[storeAt], acc);	
		acc = sum;
	}
	acc = checkCF(acc);
	checkZF(acc);
	checkNF(acc);
}

void AND(int* operands,char* type){
	if(!strcmp(type,"Imm")){
		acc = acc & operands[0];
	}
	else{
		int storeAt = arrayIndex(operands, type);
		acc = acc & byteArray[storeAt];
	}
	checkZF(acc);
	checkNF(acc);
}

void ASL(int* operands,char* type){
	int bit7;
	if(!strcmp(type,"Acc")){
		bit7 = acc & 0x80;
		acc = (acc << 1) & 0xFF;
		checkNF(acc);
		checkZF(acc);
	}
	else{
		int storeAt = arrayIndex(operands, type);
		bit7 = byteArray[storeAt] & 0x80;
		byteArray[storeAt] = (byteArray[storeAt]<<1) & 0xFF;
		checkNF(byteArray[storeAt]);
		checkZF(byteArray[storeAt]);
	}

	CF = bit7;
}



void branching(int *operands, char* type, int flag){
	if(flag == 1){
		int offset = translate8bitInts(operands[0]);
		printf("offset %d\n", offset);
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
		pc = pc + 2 + offset;
		printf("pc is 0x%x\n", pc + (0x0600 - 0x100));
	}
}

void BCC(int* operands, char* type){
	branching(operands, type, !CF);
}

void BCS(int* operands, char* type){
	branching(operands, type, CF);
}

void BEQ(int* operands, char* type){
	branching(operands, type, ZF);
}

void BIT(int* operands, char* type){
	int storeAt = arrayIndex(operands, type);
	int value = byteArray[storeAt];
	CF = ((acc & value) != 0);
	NF = (value & 0x80) >> 7;
	VF = (value & 0x40) >> 6;
}

void BMI(int* operands, char* type){
	branching(operands, type, NF);
}

void BNE(int* operands, char* type){
	branching(operands, type, !ZF);
}

void BPL(int* operands, char* type){
	branching(operands, type, !NF);
}

void BVC(int* operands, char* type){
	branching(operands, type, !VF);
}

void BVS(int* operands, char* type){
	branching(operands, type, VF);
}

void CLC(int* operands, char* type){
	CF = 0;
}

void CLD(int* operands, char* type){
	DF = 0;
}

void CLI(int* operands, char* type){
	IF = 0;
}

void CLV(int* operands, char* type){
	VF = 0;
}

void compare(int* operands, char* type, int* reg){
	int compare;
	if(!strcmp(type,"Imm")){
		compare = operands[0];
	} else{
		int storeAt = arrayIndex(operands, type);
		compare = byteArray[storeAt];
	}
	if(*reg >= compare){
		CF = 1;
	}
	else {
		CF = 0;
	}
	if(*reg == compare){
		ZF = 1;
	} else{
		ZF = 0;
	}
	checkNF(*reg - compare);
}

void CMP(int* operands, char* type){
	compare(operands, type, acc_ptr);
}

void CPY(int* operands, char* type){
	compare(operands, type, y_ptr);
}

void CPX(int* operands, char* type){
	compare(operands, type, x_ptr);
}


void DEC(int* operands, char* type){
	int storeAt = arrayIndex(operands, type);
	byteArray[storeAt]--;
	checkZF(byteArray[storeAt]);
	checkNF(byteArray[storeAt]);
}

void DEX(int* operands, char* type){
	X--;
	checkNF(X);
	checkZF(X);
}
void DEY(int* operands, char* type){
	Y--;
	checkNF(Y);
	checkZF(Y);
}


void EOR(int* operands,char* type){
	if(!strcmp(type,"Imm")){
		acc = acc ^ operands[0];
	}
	else{
		int storeAt = arrayIndex(operands, type);
		acc = acc ^ byteArray[storeAt];
	}
	checkZF(acc);
	checkNF(acc);
}

void INC(int* operands, char* type){
	int storeAt = arrayIndex(operands, type);
	byteArray[storeAt]++;
	checkZF(byteArray[storeAt]);
	checkNF(byteArray[storeAt]);
}

void INX(int* operands, char* type){
	X++;
	checkNF(X);
	checkZF(X);
}

void INY(int* operands, char* type){
	Y++;
	checkNF(Y);
	checkZF(Y);
}

void JMP(int* operands, char* type){
	// int storeAt = arrayIndex(operands, type);
	if(!strcmp(type,"Ind")){
		int target = operands[0] + (operands[1]<<8);
		printf("Target 0x%x\n", target);
		int lsb = byteArray[target];
		int msb = byteArray[target+1]<<8;

		pc = lsb + msb;
	}
	else if(!strcmp(type,"Abs")){
		int target = operands[0] + (operands[1]<<8);
		pc = target;
	}
}


void JSR(int* operands, char* type){
	printf("pc+2 is %x\n", pc+2);
	byteArray[SP - 1] = (pc + 2) & 0xFF;
	byteArray[SP] = ((pc+2) & 0xFF00) >> 8;
	printf("HSB %x\n", byteArray[SP]);
	printf("LSB %x\n", byteArray[SP-1]);
	SP = (SP - 2) & 0xFFFF;
	pc = operands[0] + (operands[1]<<8) + 0x100;
}


void loadRegister(int* operands,char* type, int* reg){
	if(!strcmp(type,"Imm")){
		*reg = operands[0];
	}
	else{
		int storeAt = arrayIndex(operands, type);
		*reg = byteArray[storeAt];
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

void LSR(int* operands,char* type){
	int bit0;
	if(!strcmp(type,"Acc")){
		bit0 = acc & 0x01;
		acc = (acc >> 1) & 0x7F;
		checkNF(acc);
		checkZF(acc);
	}
	else{
		int storeAt = arrayIndex(operands, type);
		bit0 = byteArray[storeAt] & 0x01;
		byteArray[storeAt] = (byteArray[storeAt]>>1) & 0x7f;
		checkNF(byteArray[storeAt]);
		checkZF(byteArray[storeAt]);
	}

	CF = bit0;
}

void NOP(int* operands, char* type){}

void ORA(int* operands,char* type){
	if(!strcmp(type,"Imm")){
		acc = acc | operands[0];
	}
	else{
		int storeAt = arrayIndex(operands, type);
		acc = acc | byteArray[storeAt];
	}
	checkZF(acc);
	checkNF(acc);
}

void PHA(int* operands, char* type){
	byteArray[SP] = acc;
	SP = (SP - 1) & 0xFFFF;
}

void PHP(int* operands, char* type){
	int res = (NF<<7)+(VF<<6)+(BF<<4)+(DF<<3)+(IF<<2)+(ZF<<1)+(CF);
	byteArray[SP] = res;
	SP = (SP - 1) & 0xFFFF;
}

void PLA(int* operands, char* type){
	acc = byteArray[(SP+1) & 0xFFFF];
	SP = (SP + 1) & 0xFFFF;
	checkNF(acc);
	checkZF(acc);
}

void PLP(int* operands, char* type){
	int res = byteArray[(SP+1) & 0xFFFF];
	NF = (res>>7) & 0x1;
	VF = (res>>6) & 0x1;
	BF = (res>>4) & 0x1;
	DF = (res>>3) & 0x1;
	IF = (res>>2) & 0x1;
	ZF = (res>>1) & 0x1;
	CF = res & 0x1;

	SP = (SP + 1) & 0xFFFF;
}

void BRK(int* operands, char* type){
	BF = 0;
	byteArray[SP] = pc;
	SP--;
	PHP(operands, type);
	pc = 0xFFFE;
}

void ROL(int* operands,char* type){
	int bit7;
	if(!strcmp(type,"Acc")){
		bit7 = acc & 0x80;
		acc = (acc << 1) & 0xFF;
		acc = acc + CF;
		checkNF(acc);
		checkZF(acc);
	}
	else{
		int storeAt = arrayIndex(operands, type);
		bit7 = byteArray[storeAt] & 0x80;
		byteArray[storeAt] = ((byteArray[storeAt]<<1) & 0xFF) + CF;
		checkNF(byteArray[storeAt]);
		checkZF(byteArray[storeAt]);
	}

	CF = bit7;
}


void ROR(int* operands,char* type){
	int bit0;
	if(!strcmp(type,"Acc")){
		bit0 = acc & 0x01;
		acc = (acc >> 1) & 0x7F;
		acc = acc + (CF<<7);
		checkNF(acc);
		checkZF(acc);
	}
	else{
		int storeAt = arrayIndex(operands, type);
		bit0 = byteArray[storeAt] & 0x01;
		byteArray[storeAt] = (byteArray[storeAt]>>1) & 0x7f;
		byteArray[storeAt] = byteArray[storeAt] + (CF<<7);
		checkNF(byteArray[storeAt]);
		checkZF(byteArray[storeAt]);
	}

	CF = bit0;
}

void RTI(int* operands, char* type){
	PLP(operands, type);
	SP++;
	pc = byteArray[SP];
	SP++;
}

void RTS(int* operands, char* type){
	pc = (byteArray[SP+2] << 8) + byteArray[SP+1] + 1;
	byteArray[SP+1] = 0;
	byteArray[SP+2] = 0;
	SP = (SP + 2) & 0xFFFF;
}

void SBC(int* operands, char* type){
	if(!strcmp(type,"Imm")){
		acc = subtract8bits(acc,operands[0]);
	}
	else{
		int storeAt = arrayIndex(operands, type);
		int diff = subtract8bits(acc,byteArray[storeAt]);	
		acc = diff;
	}
	acc = checkCF(acc);
	checkZF(acc);
	checkNF(acc);
}

void SEC(int* operands, char* type){
	CF = 1;
}

void SED(int* operands, char* type){
	DF = 1;
}

void SEI(int* operands, char* type){
	DF = 1;
}
void storeRegister(int *operands, char* type, int* reg){
	int storeAt = arrayIndex(operands, type);
	byteArray[storeAt] = *reg;
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

void TYA(int* operands, char* type){
	acc = Y;
	checkZF(acc);
	checkNF(acc);
}

void TAX(int* operands, char* type){
	X = acc;
	checkZF(X);
	checkNF(X);
}

void TAY(int* operands, char* type){
	Y = acc;
	checkZF(Y);
	checkNF(Y);
}

void TSX(int* operands, char* type){
	X = SP;
	checkZF(X);
	checkNF(X);
}

void TXA(int* operands, char* type){
	acc = X;
	checkZF(acc);
	checkNF(acc);
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
	ftable[10] = BNE;
	ftable[13] = CMP;
	ftable[14] = CPX;
	ftable[15] = CPY;
	ftable[25] = INC;
	ftable[26] = JMP;
	ftable[27] = JSR;
	ftable[28] = LDA;
	ftable[29] = LDX;
	ftable[30] = LDY;
	ftable[34] = TAX;
	ftable[35] = TXA;
	ftable[36] = DEX;
	ftable[37] = INX;
	ftable[38] = TAY;
	ftable[39] = TYA;
	ftable[41] = INY;
	ftable[45] = RTS;
	ftable[47] = STA;
	ftable[50] = PHA;
	ftable[51] = PLA;
	ftable[54] = STX;
	ftable[55] = STY;
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
		// endFile = true;
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
	printf("PC = 0x%04x\n", pc);
}

// RUN PROGRAM AND UPDATE 
void updatePC(){
	int opCodeCurr = byteArray[pc];
	// printf("%s\n", );
	int noBytes = codeList[opCodeCurr].noBytes;
	// int print = pc;
	printf("0x%04x ", pc);
	// printf("\n %d\n", noBytes );
	for(int i = 0; i<noBytes; i++){
		printf("%02x ", byteArray[pc+i]);
	}
	printf("\n");
	int current = pc;
	processLine(opCodeCurr, noBytes);
	
	if(current == pc){
		printf("Run\n");
		printf("No bytes %d\n", noBytes);
		pc = pc + noBytes;
	}
	printData();
}

void continueToAddress(int address){
	while(pc < address){
		updatePC();
	}
}
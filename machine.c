#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#include "readin.h"
#include "opcode.h"

// Variables for program counter, flags and registers (pointers included for some)
int pc = 0x0100;

int acc = 0;
int* acc_ptr = &acc;
int X = 0;
int* x_ptr = &X;
int Y = 0;
int* y_ptr = &Y;
int SP = 0xFFFF;

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

/* 	Function checks if carry flag should be set
	Return number after wrapping is done
*/
int checkCF(int no){
	if(no > 0xFF){
		no = no & 0xFF;
		CF = 1;
		return no;
	}
	CF = 0;
	return no;
}

// Function checks if zero flag should be set
bool checkZF(int reg){
	if(reg == 0){
		ZF = 1;
		return true;
	}
	ZF = 0;
	return false;
}

// Function checks if negative flag should be set
bool checkNF(int reg){
	if ((reg & 0x80) == 0x80){
		NF = 1;
		return true;
	}
	NF = 0;
	return false;
}

// Given an integer, function will read it as if it is a 8bit 2s complement integer
// Used for branching operation
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

// Given two integers, function will add the two integers along with carry
// Then check for carry and overflow
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

// Given two integers, function will subtract the two integers along with carry
// Then check for carry and overflow
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

// int getAddress(int* operands, int number){
// 	if(number == 1){
// 		return byteArray[operands[0]];
// 	}
// 	else{
// 		return byteArray[operands[0] + (operands[1]<<8)];
// 	}
// }

// Given Absolute mode, Operands and Registers, function will return corresponding address
int addressAbs(int* operands, int reg){
	return (operands[0] + (operands[1]<<8) + reg) & 0xFFFF;
}

// Given Zero Page mode, Operands and Registers, function will return corresponding address
int addressZP(int* operands, int reg){
	return (operands[0] + reg) & 0xFF;
}

// Given Indirect X mode and Operands, function will return corresponding address
int addressIndX(int* operands){
	int lookupIndex = (operands[0] + X) & 0xFF;
	int lowByte = byteArray[lookupIndex];
	int highByte = (byteArray[lookupIndex+1])<<8;

	return lowByte + highByte;
}

// Given Indirect Y mode and Operands, function will return corresponding address
int addressIndY(int* operands){
	int lookupIndex = operands[0];
	int lowByte = byteArray[lookupIndex];
	int highByte = (byteArray[lookupIndex+1])<<8; 
	int address = (lowByte + highByte + Y) & 0xFFFF;

	return address;
}

// Function, given a type, will use above functions to return an address (array index of main array)
int arrayIndex(int* operands, char* type){
	int storeAt;
	if(!strcmp(type,"ZP")){
		storeAt = addressZP(operands, 0);
	} else if(!strcmp(type,"ZPX")){
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

// Adds the contents of a memory location to accumulator along with carry
// Set Zero, Overflow and Negative accordingly
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

// Perform AND on accumulator with contents of memory location
// Set Z, V, N 
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

// Shift all bits of accumulator to left
// Bit 0 is 0 and bit 7 is new carry
void ASL(int* operands,char* type){
	int bit7;
	if(!strcmp(type,"Acc")){
		bit7 = acc & 0x80;
		acc = (acc << 1) & 0xFE;
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

// Helper function for branching operations
void branching(int *operands, char* type, int flag){
	if(flag == 1){
		int offset = translate8bitInts(operands[0]);	//Convert to 8 bit 2s complements
		pc = pc + 2 + offset;		// Update pc
	}
}

// Branch if carry clear
void BCC(int* operands, char* type){
	branching(operands, type, !CF);
}

// Branch if carry set
void BCS(int* operands, char* type){
	branching(operands, type, CF);
}

// Branch if zero set
void BEQ(int* operands, char* type){
	branching(operands, type, ZF);
}

// Test if one or more bits are set in a memory location
void BIT(int* operands, char* type){
	int storeAt = arrayIndex(operands, type);
	int value = byteArray[storeAt];
	ZF = ((acc & value) != 0);
	NF = (value & 0x80) >> 7;		// NF - bit 7
	VF = (value & 0x40) >> 6;		// VD = bit 6
}

// Branch if Negative Flag set
void BMI(int* operands, char* type){
	branching(operands, type, NF);
}

// Branch if zero flag is clear
void BNE(int* operands, char* type){
	branching(operands, type, !ZF);
}

// Branch if positive
void BPL(int* operands, char* type){
	branching(operands, type, !NF);
}

// Branch if overflow flag clear
void BVC(int* operands, char* type){
	branching(operands, type, !VF);
}

// Branch if overflow flag set
void BVS(int* operands, char* type){
	branching(operands, type, VF);
}

// Clear Carry Flag
void CLC(int* operands, char* type){
	CF = 0;
}

// Clear Decimal Flag
void CLD(int* operands, char* type){
	DF = 0;
}

// Clear Inrterrupt Flag
void CLI(int* operands, char* type){
	IF = 0;
}

// Clear Overflow
void CLV(int* operands, char* type){
	VF = 0;
}

// Helper method for Compare Instructions
void compare(int* operands, char* type, int* reg){
	int compare;
	// Get compare value
	if(!strcmp(type,"Imm")){
		compare = operands[0];
	} else{
		int storeAt = arrayIndex(operands, type);
		compare = byteArray[storeAt];
	}

	// SET CF if *reg >= compare
	if(*reg >= compare){
		CF = 1;
	}
	else {
		CF = 0;
	}

	// SET ZF if *reg == compare
	if(*reg == compare){
		ZF = 1;
	} else{
		ZF = 0;
	}
	checkNF(*reg - compare);
}

// Compare acc with another value
void CMP(int* operands, char* type){
	compare(operands, type, acc_ptr);
}

// Compare Y with another value
void CPY(int* operands, char* type){
	compare(operands, type, y_ptr);
}

// Compare X with another value
void CPX(int* operands, char* type){
	compare(operands, type, x_ptr);
}

// Subtract one from a value at a memory location
void DEC(int* operands, char* type){
	int storeAt = arrayIndex(operands, type);
	byteArray[storeAt]--;
	checkZF(byteArray[storeAt]);
	checkNF(byteArray[storeAt]);
}

// Subtract one from X
void DEX(int* operands, char* type){
	X--;
	checkNF(X);
	checkZF(X);
}

// Subtract from Y
void DEY(int* operands, char* type){
	Y--;
	checkNF(Y);
	checkZF(Y);
}

// Perform XOR on accumulator using a memory content
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

// Add one to value held at a memory location
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

// Set PC to specified address
void JMP(int* operands, char* type){
	if(!strcmp(type,"Ind")){
		int target = operands[0] + (operands[1]<<8); // For Indirect, get a target
		printf("Target 0x%x\n", target);
		int lsb = byteArray[target];	
		int msb = byteArray[target+1]<<8;			// Get low and high bytes

		pc = lsb + msb;
	}
	else if(!strcmp(type,"Abs")){
		int target = operands[0] + (operands[1]<<8);
		pc = target;
	}
}

// Function pushes return point on stack (address of next instruction minus 1)
// Sets PC to memory address
void JSR(int* operands, char* type){
	printf("pc+2 is %x\n", pc+2);
	byteArray[SP - 1] = (pc + 2) & 0xFF;
	byteArray[SP] = ((pc+2) & 0xFF00) >> 8;
	printf("HSB %x\n", byteArray[SP]);
	printf("LSB %x\n", byteArray[SP-1]);
	SP = (SP - 2) & 0xFFFF;
	pc = operands[0] + (operands[1]<<8) + 0x100;
}

// Load a byte of memory into a register
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

// Shift specified memory/register one place to the right
// Bit 0 is shifted to CF. Bit 7 set to 0.
void LSR(int* operands,char* type){
	int bit0;
	if(!strcmp(type,"Acc")){
		bit0 = acc & 0x01;
		acc = (acc >> 1) & 0x7F; 	// Set bit 7 to 0
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

// No operation
void NOP(int* operands, char* type){}

// Perform OR on accumulator
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

// Push a copy of accumulator on stack
void PHA(int* operands, char* type){
	byteArray[SP] = acc;
	SP = (SP - 1) & 0xFFFF;
}

// Push status flags to stack
void PHP(int* operands, char* type){
	int res = (NF<<7)+(VF<<6)+(BF<<4)+(DF<<3)+(IF<<2)+(ZF<<1)+(CF);
	byteArray[SP] = res;
	SP = (SP - 1) & 0xFFFF;
}

// Pull 8 bit value to accumulator
void PLA(int* operands, char* type){
	SP = (SP + 1) & 0xFFFF;
	acc = byteArray[SP];
	byteArray[SP] = 0;
	checkNF(acc);
	checkZF(acc);
}

// Pull 8 bit to processor flags
void PLP(int* operands, char* type){
	SP = (SP+1) & 0xFFFF;
	int res = byteArray[SP];
	NF = (res>>7) & 0x1;
	VF = (res>>6) & 0x1;
	BF = (res>>4) & 0x1;
	DF = (res>>3) & 0x1;
	IF = (res>>2) & 0x1;
	ZF = (res>>1) & 0x1;
	CF = res & 0x1;

	byteArray[SP] = 0;
}

// Call an interrupt request. PC and processor status pushed to stack
// IRQ interrupt at FFFE/F load to PC
// BF = 1;
void BRK(int* operands, char* type){
	BF = 1;
	byteArray[SP] = pc;
	SP--;
	PHP(operands, type);
	pc = 0xFFFE;
}

// Move bits in A or Memory one place to the left
// Bit 0 is current CF. Old bit 7 is new CF
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

// Move bits in A or Memory one place to the right
// Bit 0 is current CF. Old bit 7 is new CF
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

// Pull processor flags from stack followed by PC

void RTI(int* operands, char* type){
	PLP(operands, type);
	SP++;
	pc = byteArray[SP];
	SP++;
}

// Return from subroutine to calling routine. 
// Pulls PC from stack
void RTS(int* operands, char* type){
	pc = (byteArray[SP+2] << 8) + byteArray[SP+1] + 1;
	byteArray[SP+1] = 0;
	byteArray[SP+2] = 0;
	SP = (SP + 2) & 0xFFFF;
}

// Subtracts acc by contents of memory location with the not of carry bit.
// If over flow occurs, carry bit is clear.
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

// Set Carry Flag
void SEC(int* operands, char* type){
	CF = 1;
}

// Set Decimal Flag
void SED(int* operands, char* type){
	DF = 1;
}

// Set Interrupt Flag
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

// Copy acc into X
void TAX(int* operands, char* type){
	X = acc;
	checkZF(X);
	checkNF(X);
}

// Copy acc to Y
void TAY(int* operands, char* type){
	Y = acc;
	checkZF(Y);
	checkNF(Y);
}


// Copy SP to X
void TSX(int* operands, char* type){
	X = SP;
	checkZF(X);
	checkNF(X);
}

// Copy X to Acc
void TXA(int* operands, char* type){
	acc = X;
	checkZF(acc);
	checkNF(acc);
}

// Copy X to SP
void TXS(int* operands, char* type){
	SP = X;
	checkZF(X);
	checkNF(X);
}

// Copy Y to Accumulator
void TYA(int* operands, char* type){
	acc = Y;
	checkZF(acc);
	checkNF(acc);
}

// Given name of opcode, return index of corresponding functions in function table
int findIndex(char* code){
	for (int i = 0; i < 56; ++i)
	{
		if(!strcmp(codeNames[i],code)){
			return i;
		}
	}
	return -1;
}

// Build a function table of 56 instructions
void (*ftable[56])(int* operands,char*mode) = {ADC, AND, ASL, BIT, BPL, BMI, BVC, BVS, BCC, BCS, BNE, 
BEQ, BRK, CMP, CPX, CPY, DEC, EOR, CLC, SEC, CLI, SEI, CLV, CLD, SED, INC, JMP, JSR, LDA, LDX, LDY, 
LSR, NOP, ORA, TAX, TXA, DEX, INX, TAY, TYA, DEY, INY, ROR, ROL, RTI, RTS, SBC, STA, TXS, TSX, PHA, PLA, PHP, PLP, STX, STY};

void initializeFunctions() {
// 	ftable = {ADC, AND, ASL, BIT, BPL, BMI, BVC, BVS, BCC, BCS, BNE, 
// BEQ, BRK, CMP, CPX, CPY, DEC, EOR, CLC, SEC, CLI, SEI, CLV, CLD, SED, INC, JMP, JSR, LDA, LDX, LDY, 
// LSR, NOP, ORA, TAX, TXA, DEX, INX, TAY, TYA, DEY, INY, ROR, ROL, RTI, RTS, SBC, STA, TXS, TSX, PHA, PLA, PHP, PLP, STX, STY}
	// ftable[0] = ADC;
	// ftable[1] = AND;
	// ftable[4] = BPL;
	// ftable[5] = BMI;
	// ftable[10] = BNE;
	// ftable[13] = CMP;
	// ftable[14] = CPX;
	// ftable[15] = CPY;
	// ftable[25] = INC;
	// ftable[26] = JMP;
	// ftable[27] = JSR;
	// ftable[28] = LDA;
	// ftable[29] = LDX;
	// ftable[30] = LDY;
	// ftable[34] = TAX;
	// ftable[35] = TXA;
	// ftable[36] = DEX;
	// ftable[37] = INX;
	// ftable[38] = TAY;
	// ftable[39] = TYA;
	// ftable[41] = INY;
	// ftable[45] = RTS;
	// ftable[47] = STA;
	// ftable[50] = PHA;
	// ftable[51] = PLA;
	// ftable[54] = STX;
	// ftable[55] = STY;
}

void processLine(int opCode, int noBytes){
	int* operands = (int*)malloc(sizeof(int)*(noBytes-1));

	for(int i = 1; i<noBytes; i++){
		*(operands+i-1) = byteArray[pc+i];	// Get operands using length of instructions
	}

	char* name = codeList[opCode].name;
	char* mode = codeList[opCode].mode;

	if(findIndex(name) == -1){		// If op code is invalid exit
		printf("Error reading opcode - Opcode invalid\n");
		exit(-1);
	}

	// printf("Ftable function is %d\n", findIndex(name));

	// Given operand code, run function
	ftable[findIndex(name)](operands, mode);
}

// Print Data
void printData(){
	printf("A = $%02x \n", acc);
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
	int noBytes = codeList[opCodeCurr].noBytes;

	// Print current instruction line
	printf("0x%04x ", pc);
	for(int i = 0; i<noBytes; i++){
		printf("%02x ", byteArray[pc+i]);
	}
	printf("\n");
	int current = pc;
	processLine(opCodeCurr, noBytes);
	
	// Update pc for operations that does not change PC
	if(current == pc){
		pc = pc + noBytes;
	}
	// Else keep pc the same
	printData();
}

// Continue to an address. 
// Address can be an operand
// Conitinue to nearest byte ( -4 to + 3) range;
void continueToAddress(int address){

	while(pc != address){
		if(pc>address && pc < address + 3){
			break;
		}
		if(pc < address && pc > address -4){
			break;
		}
		updatePC();
	}
}
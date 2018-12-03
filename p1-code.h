typedef struct OpCode{
	char name[3];
	int code[12];
} OpCode;  

extern OpCode* codeList[56];
extern void processOneOpCode(char* line, OpCode** opCodeList, int index);
extern int opCodeHex(char* opCode, StringList* operandList);
extern void processOpCodes();
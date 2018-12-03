typedef struct OpCode{
	char name[4];
	char mode[5];
	int hexCode;
	int noBytes;
} OpCode;  

extern OpCode codeList[256];
extern char* codeNames[56];
extern void processOpCodes();
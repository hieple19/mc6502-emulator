#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>


void removeSpaces(char* source){
	char* result = source;
	char* current = source;
	while(*current != 0)
	{
		*result = *current++;
		if(*result != ' ')
			result++;
	}
	*result = 0;
}
int main(){
	FILE *inputFile;
	char file[20000];

	inputFile = fopen("fib.exc", "r");

	if (inputFile == NULL)
	{
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	char line[20];
	while(fgets(line, sizeof(line),inputFile)){
		// printf("%s \n", line);

		if(!strcmp(line,"ENDHEADER\n")){
			break;
		}
	}

	fgets(line, sizeof(line),inputFile);
	// fgets(line, sizeof(line),inputFile);
	// printf("%s\n", line );

	// printf("%s\n", binaryLine);
    char *input = 0;
    size_t cur_len = 0;
    while (fgets(line, sizeof(line), inputFile) != 0)
    {
        size_t buf_len = strlen(line);
        char *extra = realloc(input, buf_len + cur_len + 1);
        if (extra == 0)
            break;
        input = extra;
        strcpy(input + cur_len, line);
        cur_len += buf_len;
        // printf("Line is %s\n", buffer);
    }
    printf("%s [%d]", input, (int)strlen(input));
    free(input);
    return 0;

	/*char* binaryLine;
	*/
}
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void parseInput(instruction* instr_ptr);
void addToken(instruction* instr_ptr, char* tok);
void addNull(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void expandVariables(instruction* instr_ptr);

int checkCommand(instruction* instr_ptr);
void fat32exit();
void fat32info();
void fat32size(instruction* instr_ptr);
void fat32ls(instruction* instr_ptr);
void fat32cd(instruction* instr_ptr);
void fat32create(instruction* instr_ptr);
void fat32mkdir(instruction* instr_ptr);
void fat32mv(instruction* instr_ptr);
void fat32open(instruction* instr_ptr);
void fat32close(instruction* instr_ptr);
void fat32read(instruction* instr_ptr);
void fat32write(instruction* instr_ptr);
void fat32rm(instruction* instr_ptr);
void fat32cp(instruction* instr_ptr);

int littleToBigEndian(uint8_t *address, int bytes);

//Array of all bytes read from the image
uint8_t *BYTE_ARRAY;

//String to contain the current working directory
char *fat32PWD;

int main (int argc, char* argv[]) {
    //Variables
    FILE *input;
    int offset;
    uint8_t inputByte;

	BYTE_ARRAY = malloc(100000000); //100 MB
	fat32PWD = malloc(100);

    instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

    //Check arguments
    if(argc != 2) {
        printf("Invalid number of arguments\n");
        return 1;
    }

    //Open file passed in through command line
    //"rb" opens file for reading as binary
    if((input = fopen(argv[1], "rb")) == NULL) {
        printf("File could not be found\n");
        return 1;
    }

    //Loop through entire file putting each byte in the BYTE_ARRAY
    offset = 0;
    while(!feof(input)) {
        inputByte = fgetc(input);

        BYTE_ARRAY[offset] = inputByte;

        ++offset;
    };

    //Close file
    fclose(input);

	while(1) {
		printf("%s/%s> ", basename(argv[1]), fat32PWD);	//PROMPT

		//Parse instruction
		parseInput(&instr);
			
		//Check for command
		if(!checkCommand(&instr)) {
			printf("Invalid command.\n");
		}

		//Clear instruction
		clearInstruction(&instr);
	}

    return 0;
}

void parseInput(instruction* instr_ptr) {

	char* token = NULL;
	char* temp = NULL;

	// loop reads character sequences separated by whitespace
	do {
		//scans for next token and allocates token var to size of scanned token
		scanf("%ms", &token);
		temp = malloc(strlen(token)+1);

		int i;
		int start = 0;
		for (i = 0; i < strlen(token); i++) {
			//pull out special characters and make them into a separate token in the instruction
			if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&') {
				if (i-start > 0) {
					memcpy(temp, token + start, i - start);
					temp[i-start] = '\0';
					addToken(instr_ptr, temp);
				}

				char specialChar[2];
				specialChar[0] = token[i];
				specialChar[1] = '\0';
				addToken(instr_ptr,specialChar);
				start = i + 1;
			}
		}

		if (start < strlen(token)) {
			memcpy(temp, token + start, strlen(token) - start);
			temp[i-start] = '\0';
			addToken(instr_ptr, temp);
		}

		//free and reset variables
		free(token);
		free(temp);
		token = NULL;
		temp = NULL;

	} while ('\n' != getchar());    //until end of line is reached

	addNull(instr_ptr);
}

void addToken(instruction* instr_ptr, char* tok){
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr){
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;

	//instr_ptr->numTokens++; <-----Why increment the number of tokens for a null token
}

void clearInstruction(instruction* instr_ptr){
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}

int checkCommand(instruction* instr_ptr) {
    char* first = instr_ptr->tokens[0];
    if(strcmp(first, "exit")==0) {
        printf("Exiting now!\n");
        fat32exit();
        return 1;
    }
    else if(strcmp(first, "info")==0) {
        fat32info();
        return 2;
    }
    else if(strcmp(first, "size")==0) {
        fat32size(instr_ptr);
        return 3;
    }
    else if(strcmp(first, "ls")==0) {
        fat32ls(instr_ptr);
        return 4;
    }
    else if(strcmp(first, "cd")==0) {
        fat32cd(instr_ptr);
        return 5;
    }
    else if(strcmp(first, "create")==0) {
        fat32create(instr_ptr);
        return 6;
    }
    else if(strcmp(first, "mkdir")==0) {
        fat32mkdir(instr_ptr);
        return 7;
    }
    else if(strcmp(first, "mv")==0) {
        fat32mv(instr_ptr);
        return 8;
    }
    else if(strcmp(first, "open")==0) {
        fat32open(instr_ptr);
        return 9;
    }
    else if(strcmp(first, "close")==0) {
        fat32close(instr_ptr);
        return 10;
    }
    else if(strcmp(first, "read")==0) {
        fat32read(instr_ptr);
        return 11;
    }
    else if(strcmp(first, "write")==0) {
        fat32write(instr_ptr);
        return 12;
    }
    else if(strcmp(first, "rm")==0) {
        fat32rm(instr_ptr);
        return 13;
    }
    else if(strcmp(first, "cp")==0) {
        fat32cp(instr_ptr);
        return 14;
    }
    else {
        return 0;
    }
}

void fat32exit() {
	free(BYTE_ARRAY);
	exit(1);
}

void fat32info() {

    uint16_t BPB_BytsPerSec = littleToBigEndian(&BYTE_ARRAY[11], 2);
    uint8_t BPB_SecPerClus = BYTE_ARRAY[13];
    uint16_t BPB_RsvdSecCnt = littleToBigEndian(&BYTE_ARRAY[14], 2);
    uint8_t BPB_NumFATs = BYTE_ARRAY[16];
    uint32_t BPB_TotSec32 = littleToBigEndian(&BYTE_ARRAY[32], 4);
    uint32_t BPB_FATSz32 = littleToBigEndian(&BYTE_ARRAY[36], 4);
    uint32_t PB_RootClus = littleToBigEndian(&BYTE_ARRAY[44], 4);

    printf("Bytes per sector: %d\n", BPB_BytsPerSec);
    printf("Sectors per cluster: %d\n", BPB_SecPerClus);
    printf("Reserved sector count: %d\n", BPB_RsvdSecCnt);
    printf("Number of FATs: %d\n", BPB_NumFATs);
    printf("Total Sectors: %d\n", BPB_TotSec32);
    printf("FAT size: %d\n", BPB_FATSz32);
    printf("Root Cluster: %d\n", PB_RootClus);

}

void fat32size(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 2) {
		printf("size : Invalid number of arguments\n");
		return;
	}
    printf("Size of %s: %d\n", instr_ptr->tokens[1], 6969);
    return;
}

void fat32ls(instruction* instr_ptr) {
    if(instr_ptr->numTokens > 2) {
		printf("ls : Invalid number of arguments\n");
		return;
	}
    printf("Files in directory\n");
}

void fat32cd(instruction* instr_ptr) {		
	if(instr_ptr->numTokens != 2) {
		printf("cd : Invalid number of arguments\n");
		return;
	}
    printf("Changing directory to %s\n", instr_ptr->tokens[1]);
}

void fat32create(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 2) {
		printf("create : Invalid number of arguments\n");
		return;
	}
    printf("Created file %s\n", instr_ptr->tokens[1]);
}

void fat32mkdir(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 2) {
		printf("mkdir : Invalid number of arguments\n");
		return;
	}
    printf("Created directory %s\n", instr_ptr->tokens[1]);
}

void fat32mv(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 3) {
		printf("mv : Invalid number of arguments\n");
		return;
	}
    printf("Moved %s to %s\n", instr_ptr->tokens[1], instr_ptr->tokens[2]);
}

void fat32open(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 3) {
		printf("open : Invalid number of arguments\n");
		return;
	}
    printf("Opened file %s in mode %s\n", instr_ptr->tokens[1], instr_ptr->tokens[2]);
}

void fat32close(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 2) {
		printf("close : Invalid number of arguments\n");
		return;
	}
    printf("Closed file %s\n", instr_ptr->tokens[1]);
}

void fat32read(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 4) {
		printf("read : Invalid number of arguments\n");
		return;
	}
    printf("Reading file %s\n", instr_ptr->tokens[1]);
}

void fat32write(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 5) {
		printf("write : Invalid number of arguments\n");
		return;
	}
    printf("Writing file %s\n", instr_ptr->tokens[1]);
}

void fat32rm(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 2) {
		printf("rm : Invalid number of arguments\n");
		return;
	}
    printf("Removed file %s\n", instr_ptr->tokens[1]);
}

void fat32cp(instruction* instr_ptr) {
    if(instr_ptr->numTokens != 3) {
		printf("cp : Invalid number of arguments\n");
		return;
	}
    printf("Copying %s to %s\n", instr_ptr->tokens[1], instr_ptr->tokens[2]);
}

int littleToBigEndian(uint8_t *address, int bytes) {
    if(bytes == 2) return address[0] | address[1] << 8;
    if(bytes == 4) return address[0] | address[1] << 8 | address[2] << 16 | address[3] << 24;
}
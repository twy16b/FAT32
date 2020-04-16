#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ATTR_READ_ONLY 1        //dirEntry attribute definitions
#define ATTR_HIDDEN 2
#define ATTR_SYSTEM 4
#define ATTR_VOLUME_ID 8
#define ATTR_DIRECTORY 16
#define ATTR_ARCHIVE 32
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

struct DIRENTRY {

    unsigned char DIR_Name[11];
    unsigned char DIR_Attr;
    unsigned char DIR_NTRes;
    unsigned char DIR_CrtTimeTenth;
    unsigned char DIR_CrtTime[2];
    unsigned char DIR_CrtDate[2];
    unsigned char DIR_LstAccDate[2];
    unsigned char DIR_FstClusHI[2];
    unsigned char DIR_WrtTime[2];
    unsigned char DIR_WrtDate[2];
    unsigned char DIR_FstClusLO[2];
    unsigned char DIR_FileSize[4];

}__attribute__((packed));

typedef struct
{

    uint16_t BPB_BytsPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint32_t BPB_TotSec32;
    uint32_t BPB_FATSz32;
    uint32_t BPB_RootClus;

} varStruct;

struct DIRENTRY {
    unsigned char DIR_name[11]; //0-10
    unsigned char DIR_Attributes; //11
    unsigned char DIR_NTRes; //12
    unsigned char DIR_CrtTimeTenth; //13
    unsigned char DIR_CrtTime[2]; //14-15
    unsigned char DIR_CrtDate[2]; //16-17
    unsigned char DIR_LstAccDate[2]; //18-19
    unsigned char DIR_FstClusHI[2]; //20-21
    unsigned char DIR_WrtTime[2]; //22-23
    unsigned char DIR_WrtDate[2]; //24-25
    unsigned char DIR_FstClusLO[2]; //26-27
    unsigned char DIR_FileSize[4]; //28-31
}   __attribute__((packed));

void parseInput(instruction* instr_ptr);
void addToken(instruction* instr_ptr, char* tok);
void addNull(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void expandVariables(instruction* instr_ptr);

void fat32info(varStruct fat32vars);
<<<<<<< Updated upstream
void fat32size(instruction* instr_ptr);
void fat32ls(instruction* instr_ptr, struct varStruct * bs, unsigned long clusterNum);
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
=======
void fat32size();
void fat32ls();
void fat32cd();
void fat32create();
void fat32mkdir();
void fat32mv();
void fat32open();
void fat32close();
void fat32read();
void fat32write();
void fat32rm();
void fat32cp();

int littleToBigEndian(uint8_t *address);
>>>>>>> Stashed changes
void fat32initVars(FILE *image, varStruct *fat32vars);
int getFirstDataSector();
int getSectorOffset(int clusterNum);
int getByteOffset(int clusterNum);
void populateDirEntry(struct varStruct * bs, struct DIRENTRY entry, unsigned long byteNum);

int main (int argc, char* argv[]) {
    FILE *input;
    varStruct fat32vars;
    instruction instr;
    char *first;

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

    //Initialize fat32 vars
    fat32initVars(input, &fat32vars);
    
    //Initialize instruction
    instr.tokens = NULL;
	instr.numTokens = 0;

	while(1) {
		printf("%s/> ", basename(argv[1]));	//PROMPT

		//Parse instruction
		parseInput(&instr);
			
		//Check for command
		first = instr.tokens[0];
        if(strcmp(first, "exit")==0) {
            printf("Exiting now!\n");
            break;
        }

        else if(strcmp(first, "info")==0) {
            if(instr.numTokens == 1) {            
                fat32info(fat32vars);
            }
            else printf("info : Invalid number of arguments\n");
        }

        else if(strcmp(first, "size")==0) {
            if(instr.numTokens == 2) {
                fat32size();
            }
            else printf("size : Invalid number of arguments\n");
        }

        else if(strcmp(first, "ls")==0) {
            if(instr.numTokens < 3) {
                fat32ls();
            }
            else printf("ls : Invalid number of arguments\n");
        }

        else if(strcmp(first, "cd")==0) {
            if(instr.numTokens < 3) {
                fat32cd();
	        }
            else printf("cd : Invalid number of arguments\n");
        }

        else if(strcmp(first, "create")==0) {
            if(instr.numTokens == 2) {
                fat32create();
            }
            else printf("create : Invalid number of arguments\n");
        }

        else if(strcmp(first, "mkdir")==0) {
            if(instr.numTokens == 2) {
                fat32mkdir();
            }
            else printf("mkdir : Invalid number of arguments\n");
        }

        else if(strcmp(first, "mv")==0) {
            if(instr.numTokens == 3) {
                fat32mv();
            }
            else printf("mv : Invalid number of arguments\n");
        }

        else if(strcmp(first, "open")==0) {
            if(instr.numTokens == 2) {
                fat32open();
            }
            else printf("open : Invalid number of arguments\n");
        }

        else if(strcmp(first, "close")==0) {
            if(instr.numTokens == 2) {
                fat32close();
            }
            else printf("close : Invalid number of arguments\n");
        }

        else if(strcmp(first, "read")==0) {
            if(instr.numTokens == 2) {
                fat32read();
            }
            else printf("read : Invalid number of arguments\n");
        }

        else if(strcmp(first, "write")==0) {
            if(instr.numTokens == 2) {
                fat32write();
            }
            else printf("write : Invalid number of arguments\n");
        }

        else if(strcmp(first, "rm")==0) {
            if(instr.numTokens == 2) {
                fat32read();
            }
            else printf("read : Invalid number of arguments\n");
        }

        else if(strcmp(first, "cp")==0) {
            if(instr.numTokens == 3) {
                fat32cp();
            }
            else printf("cp : Invalid number of arguments\n");
        }
        
        else {
            printf("Unknown command\n");
        }

		//Clear instruction
		clearInstruction(&instr);
	}

    //Close file
    fclose(input);

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
}

void clearInstruction(instruction* instr_ptr){
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}

void fat32info(varStruct fat32vars) {

    printf("Bytes per sector: %d\n", fat32vars.BPB_BytsPerSec);
    printf("Sectors per cluster: %d\n", fat32vars.BPB_SecPerClus);
    printf("Reserved sector count: %d\n", fat32vars.BPB_RsvdSecCnt);
    printf("Number of FATs: %d\n", fat32vars.BPB_NumFATs);
    printf("Total Sectors: %d\n", fat32vars.BPB_TotSec32);
    printf("FAT size: %d\n", fat32vars.BPB_FATSz32);
    printf("Root Cluster: %d\n", fat32vars.BPB_RootClus);

}

void fat32size() {

}

<<<<<<< Updated upstream
void fat32ls(instruction* instr_ptr, struct varStruct * bs, unsigned long clusterNum) {
    if(instr_ptr->numTokens > 2) {
		printf("ls : Invalid number of arguments\n");
		return;
	}
    else {
        struct DIRENTRY dir;
        unsigned long rootLoc = getByteOffset(bs.BPB_RootClus);
        unsigned long dirLoc = rootLoc + 32;
        unsigned long increment = 64;
        if (instr_ptr->numTokens == 1){
            //read current directory
        }
        else{
            unsigned char* directoryName = instr_ptr->tokens[1];
            do {
                populateDirEntry(bs, &dir, dirLoc);
                if (!strcmp(dir.DIR_name, directoryName)){
                    //enter directory and print all files/subdirectories
                    dirLoc = getByteOffset(littleToBigEndian(dir.DIR_FstClusLO, 4));
                    populateDirEntry(bs, &dir, dirLoc);
                    while (dir.DIR_name[0] != 0x0){
                        printf("%s\t", dir.DIR_name);
                        dirLoc += increment;
                        populateDirEntry(bs, &dir, dirLoc);
                    }
                    printf("\n");
                    return;
                }
                else{
                    dirLoc += increment;
                }
            }
            while (dir.DIR_name[0] != 0x0);
            printf("Directory '%s' not found\n", directoryName);
        }
    }
    
    printf("Files in directory\n");
=======
void fat32ls() {

>>>>>>> Stashed changes
}

void fat32cd() {		

}

void fat32create() {

}

void fat32mkdir() {

}

void fat32mv() {

}

void fat32open() {

}

void fat32close() {

}

void fat32read() {

}

void fat32write() {

}

void fat32rm() {

}

void fat32cp() {

}

//This function will segfault if you pass a pointer with less than 4 bytes of memory allocated
int littleToBigEndian(uint8_t *address) {
    return address[0] | address[1] << 8 | address[2] << 16 | address[3] << 24;
}

int getFirstDataSector(){
    return bs.BPB_RsvdSecCnt + (b.BPB_NumFATs * b.BPB_FATSz32);
}

int getSectorOffset(int clusterNum){
    return ((clusterNum - 2) * bs.BPB_SecPerClus) + getFirstDataSector();
}

int getByteOffset(int clusterNum){                              //use this function to find which byte the specified cluster starts at
    return getSectorOffset(clusterNum) * bs.BPB_BytsPerSec;
}

void fat32initVars(FILE *image, varStruct *fat32vars) {
    uint8_t *temp;

    temp = malloc(4);
    fseek(image, 11, SEEK_SET);
    fread(temp, 1, 2, image);
    fat32vars->BPB_BytsPerSec = littleToBigEndian(temp);
    free(temp);

    fseek(image, 13, SEEK_SET);
    fat32vars->BPB_SecPerClus = fgetc(image);

    temp = malloc(4);
    fseek(image, 14, SEEK_SET);
    fread(temp, 1, 2, image);
    fat32vars->BPB_RsvdSecCnt = littleToBigEndian(temp);
    free(temp);

    fseek(image, 16, SEEK_SET);
    fat32vars->BPB_NumFATs = fgetc(image);

    temp = malloc(4);
    fseek(image, 32, SEEK_SET);
    fread(temp, 1, 4, image);
    fat32vars->BPB_TotSec32 = littleToBigEndian(temp);
    free(temp);

    temp = malloc(4);
    fseek(image, 36, SEEK_SET);
    fread(temp, 1, 4, image);
    fat32vars->BPB_FATSz32 = littleToBigEndian(temp);
    free(temp);

    temp = malloc(4);
    fseek(image, 44, SEEK_SET);
    fread(temp, 1, 4, image);
    fat32vars->BPB_RootClus = littleToBigEndian(temp);
    free(temp);

}

void populateDirEntry(struct varStruct * bs, struct DIRENTRY entry, unsigned long byteNum){
    unsigned char * temp;
    temp = malloc(12);

    //open file
    int filedesc = open("fat32.img", O_RDWR);       //open() takes the filename and read/write permissions and returns a file descriptor (int)
    if (filedesc < 0){
        printf("Error opening file...");
    }

    //get dir name
    lseek(filedesc, byteNum, SEEK_SET);             //lseek() takes the filedescriptor and jumps to the offset (byteNum)
    if (read(filedesc, temp, 11) < 0){              //read() reads from the file and stores the specified number of characters (11, in this case) in temp
        printf("Error reading dirname");
    }
    entry.DIR_name = temp;       //find a way to convert hex to ascii

    //get dir attributes
    lseek(filedesc, byteNum+11, SEEK_SET);
    if (read(filedesc, temp, 1) < 0){
        printf("Error reading attributes");
    }
    entry.DIR_Attributes = littleToBigEndian(temp, 4);

    //get dir reserved space
    lseek(filedesc, byteNum+12, SEEK_SET);
    if (read(filedesc, temp, 1) < 0){
        printf("Error reading reserved space");
    }
    entry.DIR_NTRes = littleToBigEndian(temp, 4);

    //get dir creation time (tenths of a second)
    lseek(filedesc, byteNum+13, SEEK_SET);
    if (read(filedesc, temp, 1) < 0){
        printf("Error reading creation time tenths");
    }
    entry.DIR_CrtTimeTenth = littleToBigEndian(temp, 4);

    //get dir creation time
    lseek(filedesc, byteNum+14, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading creation time");
    }
    entry.DIR_CrtTime = littleToBigEndian(temp, 4);

    //get dir creation date
    lseek(filedesc, byteNum+16, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading creation date");
    }
    entry.DIR_CrtDate = littleToBigEndian(temp, 4);

    //get dir last access date
    lseek(filedesc, byteNum+18, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading last access date");
    }
    entry.DIR_LstAccDate = littleToBigEndian(temp, 4);

    //get dir first cluster high word
    lseek(filedesc, byteNum+20, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading first cluster high word");
    }
    entry.DIR_FstClusHI = littleToBigEndian(temp, 4);

    //get dir last write time
    lseek(filedesc, byteNum+22, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading last write time");
    }
    entry.DIR_WrtTime = littleToBigEndian(temp, 4);

    //get dir last write date
    lseek(filedesc, byteNum+24, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading last write date");
    }
    entry.DIR_WrtDate = littleToBigEndian(temp, 4);

    //get dir first cluster low word
    lseek(filedesc, byteNum+26, SEEK_SET);
    if (read(filedesc, temp, 2) < 0){
        printf("Error reading first cluster low word");
    }
    entry.DIR_FstClusLO = littleToBigEndian(temp, 4);

    //get dir last access date
    lseek(filedesc, byteNum+28, SEEK_SET);
    if (read(filedesc, temp, 4) < 0){
        printf("Error reading file size");
    }
    entry.DIR_FileSize = littleToBigEndian(temp, 4);
}
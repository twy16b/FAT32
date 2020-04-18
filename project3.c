#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include "instruction.h"

struct DIRENTRY {

    char DIR_Name[13];
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
    int rootDirectoryOffset;
    int currentDirectoryOffset;
    int currentFAToffset;
    char *fat32pwd;
    struct DIRENTRY *currentDirectories;
    int numDirectories;

} varStruct;

void fat32info(varStruct fat32vars);
void fat32size(FILE *image, varStruct fat32vars, instruction* instr_ptr);
void fat32ls(varStruct fat32vars, instruction* instr_ptr);
void fat32cd(FILE *image, varStruct *fat32vars, instruction* instr_ptr);	
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
void fat32initVars(FILE *image, varStruct *fat32vars);
void fillDirectoryEntry(FILE* image, struct DIRENTRY *dir);
void jumpToDirectory(varStruct *fat32vars, struct DIRENTRY dir);
void fillDirectories(FILE* image, varStruct *fat32vars);
int getNextClusterOffset(FILE *image, varStruct fat32vars, int offset);

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
		printf("%s%s> ", basename(argv[1]), fat32vars.fat32pwd);	//PROMPT

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
                fat32size(input, fat32vars, &instr);
            }
            else printf("size : Invalid number of arguments\n");
        }

        else if(strcmp(first, "ls")==0) {
            if(instr.numTokens < 3) {
                fat32ls(fat32vars, &instr);
            }
            else printf("ls : Invalid number of arguments\n");
        }

        else if(strcmp(first, "cd")==0) {
            if(instr.numTokens < 3) {
                fat32cd(input, &fat32vars, &instr);
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

    free(fat32vars.fat32pwd);
    free(fat32vars.currentDirectories);

    //Close file
    fclose(input);

    return 0;
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

void fat32size(FILE *image, varStruct fat32vars, instruction* instr_ptr) {
    struct DIRENTRY *dir;

    int sizeloop;
    for(sizeloop = 0; sizeloop < fat32vars.numDirectories; ++sizeloop) {

        dir = &fat32vars.currentDirectories[sizeloop];

        if(strcmp(dir->DIR_Name, instr_ptr->tokens[1]) == 0) {

            if(dir->DIR_Attr == 16) {
                printf("File is directory. Size is 0.\n");
                return;
            }

            else {
                printf("%s: %d\n", dir->DIR_Name, littleToBigEndian(dir->DIR_FileSize));
                return;
            }

        }
    }

    printf("File not found\n");

}

void fat32ls(varStruct fat32vars, instruction* instr_ptr) {

    int printloop;
    for(printloop = 0; printloop < fat32vars.numDirectories; ++printloop) {
        if(fat32vars.currentDirectories[printloop].DIR_Attr == 15) continue;
        if(fat32vars.currentDirectories[printloop].DIR_Attr == 16) printf("\033[0;34m");
        printf("%s\n", fat32vars.currentDirectories[printloop].DIR_Name);
        printf("\033[0m");
    }

}

void fat32cd(FILE *image, varStruct *fat32vars, instruction* instr_ptr) {

    struct DIRENTRY *dir;		

    //No arguments, move back to root directory
    if(instr_ptr->numTokens == 1) {

        fat32vars->currentDirectoryOffset =
        fat32vars->rootDirectoryOffset;

        free(fat32vars->fat32pwd);
        fat32vars->fat32pwd = malloc(1);
        fat32vars->fat32pwd[0] = '\0';

        fillDirectories(image, fat32vars);
        
        return;
    }

    //Handle the "." and ".." cases
    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {

        if(strcmp(instr_ptr->tokens[1], ".") == 0) {
            return;
        }

        else if(strcmp(instr_ptr->tokens[1], "..") == 0) {

            jumpToDirectory(fat32vars, fat32vars->currentDirectories[1]);
            fillDirectories(image, fat32vars);

            //Remove last directory in path
            char *temp = strstr(fat32vars->fat32pwd, "/..");
            --temp;
            while(*temp != '/'){--temp;};
            *temp = '\0';

            return;
        }
    }

    //Navigate to directory in instruction
    int cdloop;
    for(cdloop = 0; cdloop < fat32vars->numDirectories; ++cdloop) {

        dir = &fat32vars->currentDirectories[cdloop];

        if(dir->DIR_Attr == 15) continue;

        if(strcmp(dir->DIR_Name, instr_ptr->tokens[1]) == 0) {
            
            if(dir->DIR_Attr != 16) {
                printf("File is not directory.\n");
                return;
            }
            else {
                jumpToDirectory(fat32vars, *dir);
                fillDirectories(image, fat32vars);
                return;
            }
        }
    }
    printf("Directory not found\n");
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

    fat32vars->currentFAToffset = 
    8 +
    (fat32vars->BPB_BytsPerSec * 
    fat32vars->BPB_RsvdSecCnt);

    fat32vars->rootDirectoryOffset = 
    fat32vars->BPB_BytsPerSec * 
    (fat32vars->BPB_RsvdSecCnt + 
    (fat32vars->BPB_NumFATs * fat32vars->BPB_FATSz32));

    fat32vars->currentDirectoryOffset = fat32vars->rootDirectoryOffset;

    fat32vars->fat32pwd = malloc(1);
    fat32vars->fat32pwd[0] = '\0';    

    fillDirectories(image, fat32vars);
}

void fillDirectoryEntry(FILE* image, struct DIRENTRY *dir) {

    //Fill directory name, add null terminator
    fread(dir->DIR_Name, 1, 11, image);
    int pos = 0;
    while(pos < 13) {
        if(dir->DIR_Name[pos] == 32 || pos == 12) {
            dir->DIR_Name[pos] = '\0';
            break;
        }
        else ++pos;
    }

    dir->DIR_Attr = fgetc(image);
    dir->DIR_NTRes = fgetc(image);
    dir->DIR_CrtTimeTenth = fgetc(image);
    fread(dir->DIR_CrtTime, 1, 2, image);
    fread(dir->DIR_CrtDate, 1, 2, image);
    fread(dir->DIR_LstAccDate, 1, 2, image);
    fread(dir->DIR_FstClusHI, 1, 2, image);
    fread(dir->DIR_WrtTime, 1, 2, image);
    fread(dir->DIR_WrtDate, 1, 2, image);
    fread(dir->DIR_FstClusLO, 1, 2, image);
    fread(dir->DIR_FileSize, 1, 4, image);

}

void fillDirectories(FILE* image, varStruct *fat32vars) {
    struct DIRENTRY dir;
    int dirCount = 0;
    int temp;

    fseek(image, fat32vars->currentDirectoryOffset, SEEK_SET);
    int originalOffset = fat32vars->currentDirectoryOffset;

    free(fat32vars->currentDirectories);
    fat32vars->currentDirectories = malloc(sizeof(struct DIRENTRY));

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {

        fat32vars->currentDirectories = 
        realloc(fat32vars->currentDirectories, 
        sizeof(struct DIRENTRY)*3);

        fillDirectoryEntry(image, &dir);
        fat32vars->currentDirectories[0] = dir;
        ++dirCount;

        fillDirectoryEntry(image, &dir);
        fat32vars->currentDirectories[1] = dir;
        ++dirCount;
    }

    while(1) {

        fillDirectoryEntry(image, &dir);

        if(dir.DIR_Name[0] == 0 || dir.DIR_Name[0] == 46) break;

        fat32vars->currentDirectories[dirCount] = dir;
        ++dirCount;

        fat32vars->currentDirectories = 
        realloc(fat32vars->currentDirectories, 
        sizeof(struct DIRENTRY)*(dirCount+1));

        if(dirCount % 16 == 0) {
            temp = getNextClusterOffset(image, *fat32vars, fat32vars->currentDirectoryOffset);
            if(temp >= 268435447) break; 
            temp = (temp-2)*fat32vars->BPB_BytsPerSec + fat32vars->rootDirectoryOffset;
            printf("Next cluster offset: %d\n", temp);
            fat32vars->currentDirectoryOffset = temp;
            fseek(image, fat32vars->currentDirectoryOffset, SEEK_SET);
        };

    }
    fat32vars->numDirectories = dirCount;
    fat32vars->currentDirectoryOffset = originalOffset;
}

void jumpToDirectory(varStruct *fat32vars, struct DIRENTRY dir) {
    int dirSector;
    char *temp = malloc(4);
    temp[0] = dir.DIR_FstClusLO[0];
    temp[1] = dir.DIR_FstClusLO[1];
    temp[2] = dir.DIR_FstClusHI[0];
    temp[3] = dir.DIR_FstClusHI[1];

    dirSector = littleToBigEndian(temp);
    if(dirSector == 0) 
        fat32vars->currentDirectoryOffset = 
        fat32vars->rootDirectoryOffset;
    else
        fat32vars->currentDirectoryOffset = 
        fat32vars->rootDirectoryOffset +
        ((dirSector-2) * 512);

    //Reallocate PWD for new directory
    fat32vars->fat32pwd = 
    realloc(fat32vars->fat32pwd, 
    strlen(fat32vars->fat32pwd) + strlen(dir.DIR_Name) + 2);

    strcat(fat32vars->fat32pwd, "/");
    strcat(fat32vars->fat32pwd, dir.DIR_Name);
    
    return;
}

int getNextClusterOffset(FILE *image, varStruct fat32vars, int offset) {
    int FAToffset = 
    fat32vars.BPB_BytsPerSec *
    fat32vars.BPB_SecPerClus *
    fat32vars.BPB_RsvdSecCnt;

    int rootCluster =
    fat32vars.rootDirectoryOffset / fat32vars.BPB_BytsPerSec;

    int currentCluster =
    (offset / fat32vars.BPB_BytsPerSec) - rootCluster;

    fseek(image, FAToffset+((currentCluster+2)*4), SEEK_SET);

    uint8_t *temp;
    temp = malloc(4);

    fread(temp, 1, 4, image);

    int nextClusterOffset = littleToBigEndian(temp);

    printf("Next cluster = %d\n", nextClusterOffset);

    return littleToBigEndian(temp);
}
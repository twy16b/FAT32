#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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

//Initializes all the variables for the FAT32 volume
void fat32initVars(FILE *image, varStruct *fat32vars) {
    unsigned char temp[4];

    memset(temp, 0, 4);
    fseek(image, 11, SEEK_SET);
    fread(temp, 1, 2, image);
    fat32vars->BPB_BytsPerSec = littleToBigEndian(temp);

    fseek(image, 13, SEEK_SET);
    fat32vars->BPB_SecPerClus = fgetc(image);

    memset(temp, 0, 4);
    fseek(image, 14, SEEK_SET);
    fread(temp, 1, 2, image);
    fat32vars->BPB_RsvdSecCnt = littleToBigEndian(temp);

    fseek(image, 16, SEEK_SET);
    fat32vars->BPB_NumFATs = fgetc(image);

    memset(temp, 0, 4);
    fseek(image, 32, SEEK_SET);
    fread(temp, 1, 4, image);
    fat32vars->BPB_TotSec32 = littleToBigEndian(temp);

    memset(temp, 0, 4);
    fseek(image, 36, SEEK_SET);
    fread(temp, 1, 4, image);
    fat32vars->BPB_FATSz32 = littleToBigEndian(temp);

    memset(temp, 0, 4);
    fseek(image, 44, SEEK_SET);
    fread(temp, 1, 4, image);
    fat32vars->BPB_RootClus = littleToBigEndian(temp);

    fat32vars->rootDirectoryOffset = 
    fat32vars->BPB_BytsPerSec * 
    (fat32vars->BPB_RsvdSecCnt + 
    (fat32vars->BPB_NumFATs * fat32vars->BPB_FATSz32));

    fat32vars->currentDirectoryOffset = fat32vars->rootDirectoryOffset;

    fat32vars->fat32pwd = malloc(1);
    fat32vars->fat32pwd[0] = '\0';

    fat32vars->currentDirectories = malloc(1);
    fillDirectories(image, fat32vars);

    fat32vars->openFiles = malloc(sizeof(fileStruct));
    fat32vars->numOpenFiles = 0;
}

//Fills dir pointer with 32 byte directory entry at offset
void fillDirectoryEntry(FILE* image, struct DIRENTRY *dir, int offset) {

    fseek(image, offset, SEEK_SET);

    dir->entryOffset = offset;

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

//Fills the currentDirectories array with directory entries at currentDirectoryOffset
//currentDirectories array does not need to be emptied beforehand
void fillDirectories(FILE *image, varStruct *fat32vars) {
    struct DIRENTRY dir;
    int dirCount = 0;

    fseek(image, fat32vars->currentDirectoryOffset, SEEK_SET);
    int originalOffset = fat32vars->currentDirectoryOffset;

    free(fat32vars->currentDirectories);
    fat32vars->currentDirectories = malloc(sizeof(struct DIRENTRY));

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {

        fat32vars->currentDirectories = 
        realloc(fat32vars->currentDirectories, 
        sizeof(struct DIRENTRY)*3);

        fillDirectoryEntry(image, &dir, ftell(image));
        fat32vars->currentDirectories[0] = dir;
        ++dirCount;

        fillDirectoryEntry(image, &dir, ftell(image));
        fat32vars->currentDirectories[1] = dir;
        ++dirCount;
    }

    while(1) {
        fillDirectoryEntry(image, &dir, ftell(image));

        if(dir.DIR_Name[0] == 0) break;

        fat32vars->currentDirectories[dirCount] = dir;
        ++dirCount;

        fat32vars->currentDirectories = 
        realloc(fat32vars->currentDirectories, 
        sizeof(struct DIRENTRY)*(dirCount+1));

        if(dirCount % 16 == 0) {
            int nextOffset = getNextClusterOffset(image, fat32vars, fat32vars->currentDirectoryOffset);
            if(nextOffset == fat32vars->currentDirectoryOffset) break; 
            fat32vars->currentDirectoryOffset = nextOffset;
            fseek(image, fat32vars->currentDirectoryOffset, SEEK_SET);
        };

    }
    fat32vars->numDirectories = dirCount;
    fat32vars->currentDirectoryOffset = originalOffset;
}

//Changes currentDirectoryOffset to first cluster pointed to by dir
//Adds filename to the fat32pwd string
void jumpToDirectory(varStruct *fat32vars, struct DIRENTRY *dir) {
    
    fat32vars->currentDirectoryOffset = getDirectoryFirstOffset(fat32vars, dir);
    //printf("Jump to directory offset = %d\n", getDirectoryFirstOffset(fat32vars, dir));

    //Reallocate PWD for new directory
    fat32vars->fat32pwd = 
    realloc(fat32vars->fat32pwd, 
    strlen(fat32vars->fat32pwd) + strlen(dir->DIR_Name) + 2);

    strcat(fat32vars->fat32pwd, "/");
    strcat(fat32vars->fat32pwd, dir->DIR_Name);
    
    return;
}

//Get the FAT entry offset of the passed in cluster number
int getFATentryOffset (varStruct *fat32vars, int cluster) {
    int FAToffset = 
    fat32vars->BPB_BytsPerSec *
    fat32vars->BPB_SecPerClus *
    fat32vars->BPB_RsvdSecCnt;

    return (cluster * 4) + FAToffset;
}

//Returns the cluster number associated with the FAT entry at the passed in offset
int getFATentryCluster(varStruct *fat32vars, int offset) {
    int FAToffset = 
    fat32vars->BPB_BytsPerSec *
    fat32vars->BPB_SecPerClus *
    fat32vars->BPB_RsvdSecCnt;

    return (offset - FAToffset) / 4;
}

//Get the number of the next cluster pointed to by the FAT entry of passed in cluster nuber
//Returns same cluster number if it is the final cluster
int getNextClusterNumber(FILE *image, varStruct *fat32vars, int cluster) {

    fseek(image, getFATentryOffset(fat32vars, cluster), SEEK_SET);

    unsigned char temp[4];

    fread(temp, 1, 4, image);

    int nextCluster = littleToBigEndian(temp);

    if(nextCluster > (fat32vars->BPB_TotSec32/fat32vars->BPB_SecPerClus)) {
        return cluster;
    }

    else return nextCluster;
}

//Get the offset of the next cluster pointed to by the FAT entry of the passed in cluster offset
//Returns the same offset if it is the final cluster
int getNextClusterOffset(FILE *image, varStruct *fat32vars, int offset) {

    int cluster = offsetToCluster(fat32vars, offset);

    int nextCluster = getNextClusterNumber(image, fat32vars, cluster);

    if(nextCluster == cluster) { 
        return offset;
    }
    else {
        return clusterToOffset(fat32vars, nextCluster);
    }
}

//Turn both strings into uppercase and return 0 if they are the same.
int compareFilenames(char *filename1, char *filename2) {
    int loop;

    char temp1[strlen(filename1)+1];
    strcpy(temp1, filename1);
    char temp2[strlen(filename2)+1];
    strcpy(temp2, filename2);

    for(loop = 0; loop < strlen(temp1); ++loop) {
        temp1[loop] = toupper(temp1[loop]);
    }

    for(loop = 0; loop < strlen(temp2); ++loop) {
        temp2[loop] = toupper(temp2[loop]);
    }

    return strcmp(temp1, temp2);

}

//Clear out the data the directory pointer is tied to and recursively clear the FAT at each cluster
void clearFATentries (FILE *image, varStruct *fat32vars, struct DIRENTRY *dir) {

    int FAToffset = fat32vars->BPB_RsvdSecCnt * fat32vars->BPB_BytsPerSec;
    static int currentCluster;
    int previousCluster;

    if(dir != NULL) {
        currentCluster = getDirectoryFirstCluster(fat32vars, dir);
        if(currentCluster <= 2) return;
    }
    else {
        previousCluster = currentCluster;
        currentCluster = getNextClusterNumber(image, fat32vars, currentCluster);
    }

    int nextCluster = getNextClusterNumber(image, fat32vars, currentCluster);

    if(nextCluster != currentCluster) {
        clearFATentries (image, fat32vars, NULL);
    }

    unsigned char *temp = malloc(fat32vars->BPB_BytsPerSec);
    memset(temp, 0, fat32vars->BPB_BytsPerSec);

    fseek(image, clusterToOffset(fat32vars, currentCluster), SEEK_SET);
    fwrite(temp, 1, fat32vars->BPB_BytsPerSec, image);
    fseek(image, getFATentryOffset(fat32vars, currentCluster), SEEK_SET);
    fwrite(temp, 1, 4, image);
    currentCluster = previousCluster;
    free(temp);
    return;
}

//Convert a cluster number into an absolute offset
int clusterToOffset(varStruct *fat32vars, int cluster) {
    return
    (cluster - 2) *
    (fat32vars->BPB_BytsPerSec * fat32vars->BPB_SecPerClus) + 
    fat32vars->rootDirectoryOffset;
}

//Convert a cluster offset into the cluster number
int offsetToCluster(varStruct *fat32vars, int offset) {
    if(offset % fat32vars->BPB_BytsPerSec != 0) 
    printf("OFFSET IS NOT MULTIPLE OF BYTES PER SEC\n");

    return
    ((offset - fat32vars->rootDirectoryOffset) / 
    fat32vars->BPB_BytsPerSec) + 2;
}

//Get the first cluster number pointed to by the directory entry
//Returns 2 if first cluster is less than 2
int getDirectoryFirstCluster (varStruct *fat32vars, struct DIRENTRY *dir) {
    unsigned char temp[4];
    temp[0] = dir->DIR_FstClusLO[0];
    temp[1] = dir->DIR_FstClusLO[1];
    temp[2] = dir->DIR_FstClusHI[0];
    temp[3] = dir->DIR_FstClusHI[1];

    int firstCluster = littleToBigEndian(temp);

    if (firstCluster >= 2) return firstCluster;
    else return 2;
}

//Get the offset of the first cluster pointed to by the directory entry
//Returns root directory offset if next cluster is less than 2
int getDirectoryFirstOffset (varStruct *fat32vars, struct DIRENTRY *dir) {
    return clusterToOffset(fat32vars, getDirectoryFirstCluster(fat32vars, dir));
}

//This function will segfault if you pass a pointer with less than 4 bytes of memory allocated
int littleToBigEndian(unsigned char *address) {
    return address[0] | address[1] << 8 | address[2] << 16 | address[3] << 24;
}

//Returns the index of the file in the openFiles array if it exists.
//Returns -1 if not open.
int openFileIndex(varStruct *fat32vars, char *filename){
    int loop;
    fileStruct *file;
    for(loop = 0; loop < fat32vars->numOpenFiles; ++loop) {
        file = &fat32vars->openFiles[loop];
        if(!compareFilenames(basename(file->path), filename)) return loop;
    }
    return -1;
}

//Traverses FAT and finds the first free cluster
//Writes free cluster in first available slot in FAT
int getFirstFreeCluster(FILE *image, varStruct *fat32vars){

    int FAToffset = 
    fat32vars->BPB_BytsPerSec *
    fat32vars->BPB_SecPerClus *
    fat32vars->BPB_RsvdSecCnt;

    fseek(image, FAToffset, SEEK_SET);

    unsigned char FATentry[4];
    int freeClus;   //stores cluster num of first available cluster

    //search FAT until you come across a 0
    while(1){
        fread(FATentry, 1, 4, image);
        if(littleToBigEndian(FATentry) == 0) break;
    }
    fseek(image, -4, SEEK_CUR);
    
    //calculate first available cluster
    return getFATentryCluster(fat32vars, ftell(image));
}

//Marks the FAT entry for cluster to point to nextCluster
//If cluster is the final cluster, pass in 0 as nextCluster
void markFATentry(FILE *image, varStruct *fat32vars, int cluster, int nextCluster) {

    int FAToffset = 
    fat32vars->BPB_BytsPerSec *
    fat32vars->BPB_SecPerClus *
    fat32vars->BPB_RsvdSecCnt;

    unsigned char emptyHex[4];
    emptyHex[0] = 255;
    emptyHex[1] = 255;
    emptyHex[2] = 255;
    emptyHex[3] = 15;

    int FATclusterOffset = getFATentryOffset(fat32vars, cluster);
    
    //nextCluster being 0 means it is the final cluster of that file
    if(nextCluster == 0) {
        fseek(image, FATclusterOffset, SEEK_SET);
        fwrite(emptyHex, 1, 4, image);
        return;
    }

    int FATnextClusterOffset = getFATentryOffset(fat32vars, nextCluster);

    //Convert next cluster into little endian
    unsigned char nextHex[4];
    nextHex[0] = bitExtracted(nextCluster, 8, 1);
    nextHex[1] = bitExtracted(nextCluster, 8, 9);
    nextHex[2] = bitExtracted(nextCluster, 8, 17);
    nextHex[3] = bitExtracted(nextCluster, 8, 25);

    //Mark cluster to point to next cluster
    fseek(image, FATclusterOffset, SEEK_SET);
    fwrite(nextHex, 1, 4, image);

    //Mark next cluster as the last cluster
    fseek(image, FATnextClusterOffset, SEEK_SET);
    fwrite(emptyHex, 1, 4, image);

    return;
}

//fill in dir with appropriate values (dirName and first cluster, mainly)
void makeDirEntry(varStruct *fat32vars, struct DIRENTRY *dir, char* dirName, int firstClusterNum){
    
    //Set all bytes to 0 to clean allocated memory
    memset(dir, 0, 32);
    
    //Set first 11 bytes as spaces for the name entry
    memset(dir, 32, 11);
    
    //Copy passed in name
    int loop;
    for(loop = 0; loop < strlen(dirName); ++loop) {
        dir->DIR_Name[loop] = dirName[loop];
    }

    //Set attribute
    dir->DIR_Attr = ATTR_DIRECTORY;

    //extract lower bits of first cluster
    unsigned char lowBits[2];
    dir->DIR_FstClusLO[0] = bitExtracted(firstClusterNum, 8, 1);
    dir->DIR_FstClusLO[1] = bitExtracted(firstClusterNum, 8, 9);

    //extract higher bits of first cluster
    unsigned char highBits[2];
    dir->DIR_FstClusHI[0] = bitExtracted(firstClusterNum, 8, 17);
    dir->DIR_FstClusHI[1] = bitExtracted(firstClusterNum, 8, 25);
}

//Translate direntry into 32 bit string to be written to image
char *directoryToString(struct DIRENTRY *dir){

    char *dirStr = malloc(32);

    //Set all bytes to 0 to clean allocated memory
    memset(dirStr, 0, 32);
    
    //Set first 11 bytes as spaces for the name entry
    memset(dirStr, 32, 11);

    //Copy name
    memcpy(dirStr, dir->DIR_Name, strlen(dir->DIR_Name));

    //Copy other values
    memcpy(&dirStr[11], &dir->DIR_Attr, 21);

    return dirStr;
}

//Convert dirname and firstClusterNum into a directory entry string
char *makeDirEntryString(varStruct *fat32vars, char *dirname, int firstClusterNum) {
    char *dirString = malloc(32);

    //Set all bytes to 0 to clean allocated memory
    memset(dirString, 0, 32);

    //Set first 11 bytes as spaces for the name entry
    memset(dirString, 32, 11);

    //Copy passed in name
    int loop;
    for(loop = 0; loop < strlen(dirname); ++loop) {
        dirString[loop] = toupper(dirname[loop]);
    }

    //Set attribute
    dirString[11] = ATTR_DIRECTORY;

    //extract lower bits of first cluster
    unsigned char lowBits[2];
    dirString[26] = bitExtracted(firstClusterNum, 8, 1);
    dirString[27] = bitExtracted(firstClusterNum, 8, 9);

    //extract higher bits of first cluster
    unsigned char highBits[2];
    dirString[20] = bitExtracted(firstClusterNum, 8, 17);
    dirString[21] = bitExtracted(firstClusterNum, 8, 25);

    return dirString;
}

//Convert filename and firstClusterNum into a file entry string
char *makeFileEntryString(varStruct *fat32vars, char *filename) {
    char *fileString = malloc(32);

    //Set all bytes to 0 to clean allocated memory
    memset(fileString, 0, 32);

    //Set first 11 bytes as spaces for the name entry
    memset(fileString, 32, 11);

    //Copy passed in name
    int loop;
    for(loop = 0; loop < strlen(filename); ++loop) {
        fileString[loop] = toupper(filename[loop]);
    }

    //Set attribute
    fileString[11] = ATTR_ARCHIVE;

    return fileString;
}

//returns k bits of number at position p
int bitExtracted(int number, int k, int p) { 
    return (((1 << k) - 1) & (number >> (p - 1))); 
} 

//Returns the offset of the first available entry in the directory at directoryOffset
int getNextAvailableEntry(FILE *image, varStruct *fat32vars, int directoryOffset) {
    
    unsigned char readEntry[32];
    int currCluster = offsetToCluster(fat32vars,directoryOffset);
    int nextCluster = 0;
    fseek(image, directoryOffset, SEEK_SET);
    while(1){
        fread(readEntry, 1, 32, image);
        if(readEntry[0] == 0 || readEntry[0] == 229) break;
        if(ftell(image) % fat32vars->BPB_BytsPerSec == 0) {
            nextCluster = getNextClusterNumber(
                    image, 
                    fat32vars, 
                    currCluster);
            if(nextCluster == currCluster) {
                //Allocate a new cluster in the FAT
                nextCluster = getFirstFreeCluster(image, fat32vars);
                markFATentry(image, fat32vars, currCluster, nextCluster);
            }
            else currCluster = nextCluster;
            fseek(image, clusterToOffset(fat32vars, currCluster), SEEK_SET);
        }
    }
    fseek(image, -32, SEEK_CUR);
    int freeDirOffset = ftell(image);

    return freeDirOffset;
}

//takes cluster in a file and expands the clusters in that file
void expandCluster(FILE *image, varStruct *fat32vars, int cluster){
    int prevClus = cluster;
    while(1){
        int nextClus = getNextClusterNumber(image, fat32vars, prevClus);
        if (prevClus == nextClus){   //finds last cluster in that file
            int freeClus = getFirstFreeCluster(image, fat32vars);   //allocate first free cluster and make it the last cluster of file
            markFATentry(image, fat32vars, nextClus, freeClus);
            markFATentry(image, fat32vars, freeClus, 0);
            break;
        }
        prevClus = nextClus;
    }
   
}
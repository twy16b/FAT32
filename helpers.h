#ifndef HELPERS_H
#define HELPERS_H

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME 0x0F

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void parseInput(instruction* instr_ptr);
void addToken(instruction* instr_ptr, char* tok);
void addNull(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);

struct DIRENTRY {

    unsigned char DIR_Name[13];
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
    int entryOffset;

}__attribute__((packed));

typedef struct
{

    char *path;
    int offset;
    int mode;

} fileStruct;

typedef struct
{

    unsigned short BPB_BytsPerSec;
    unsigned char BPB_SecPerClus;
    unsigned short BPB_RsvdSecCnt;
    unsigned char BPB_NumFATs;
    unsigned int BPB_TotSec32;
    unsigned int BPB_FATSz32;
    unsigned int BPB_RootClus;
    int rootDirectoryOffset;
    int currentDirectoryOffset;
    int currentFAToffset;
    char *fat32pwd;
    struct DIRENTRY *currentDirectories;
    int numDirectories;
    fileStruct *openFiles;
    int numOpenFiles;

} varStruct;

void fat32initVars(FILE *image, varStruct *fat32vars);
void fillDirectoryEntry(FILE* image, struct DIRENTRY *dir, int offset);
void fillDirectories(FILE *image, varStruct *fat32vars);
void jumpToDirectory(varStruct *fat32vars, struct DIRENTRY *dir);
int getFATentryOffset (varStruct *fat32vars, int cluster);
int getNextClusterNumber(FILE *image, varStruct *fat32vars, int cluster);
int getNextClusterOffset(FILE *image, varStruct *fat32vars, int offset);
int compareFilenames(char *filename1, char *filename2);
void clearFATentries (FILE *image, varStruct *fat32vars, struct DIRENTRY *dir);
int clusterToOffset(varStruct *fat32vars, int cluster);
int offsetToCluster(varStruct *fat32vars, int offset);
int getDirectoryFirstCluster (varStruct *fat32vars, struct DIRENTRY *dir);
int getDirectoryFirstOffset (varStruct *fat32vars, struct DIRENTRY *dir);
int littleToBigEndian(unsigned char *address);
int openFileIndex(varStruct *fat32vars, char *filename);
int getFirstFreeCluster(FILE *image, varStruct *fat32vars);
void markFATentry(FILE *image, varStruct *fat32vars, int cluster, int nextCluster);
char *directoryToString(struct DIRENTRY *dir);
char *makeDirEntryString(varStruct *fat32vars, char *dirname, int firstFreeClus);
char *makeFileEntryString(varStruct *fat32vars, char *filename);
int bitExtracted(int number, int k, int p);
int getNextAvailableEntry(FILE *image, varStruct *fat32vars, int directoryOffset);
void expandCluster(FILE *image, varStruct *fat32vars, int cluster);

#endif
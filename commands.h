#ifndef COMMANDS_H
#define COMMANDS_H

void fat32info(varStruct *fat32vars);
void fat32size(FILE *image, varStruct *fat32vars, char *filename);
void fat32ls(FILE *image, varStruct *fat32vars, instruction* instr_ptr);
void fat32cd(FILE *image, varStruct *fat32vars, instruction* instr_ptr);	
void fat32creat(FILE *image, varStruct *fat32vars, instruction* instr_ptr);
void fat32mkdir(FILE *image, varStruct *fat32vars, char *directoryName);
void fat32mv(FILE *image, varStruct *fat32vars, char *filename1, char *filename2);
void fat32open(varStruct *fat32vars, char* filename, char *mode);
void fat32close(varStruct *fat32vars, char* filename);
void fat32read(FILE *image, varStruct *fat32vars, char* filename, char *offsetString, char *sizeString);
void fat32write(FILE *image, varStruct *fat32vars, char* filename, char* offsetString, char* sizeString, char* inputString);
void fat32rm(FILE *image, varStruct *fat32vars, char* filename);
void fat32cp(FILE *image, varStruct *fat32vars, instruction* instr_ptr);

#endif
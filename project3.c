#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include "commands.h"
#include "helpers.h"

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
    if((input = fopen(argv[1], "r+")) == NULL) {
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
                fat32info(&fat32vars);
            }
            else printf("info : Invalid number of arguments\n");
        }

        else if(strcmp(first, "size")==0) {
            if(instr.numTokens == 2) {
                fat32size(input, &fat32vars, instr.tokens[1]);
            }
            else printf("size : Invalid number of arguments\n");
        }

        else if(strcmp(first, "ls")==0) {
            if(instr.numTokens < 3) {
                fat32ls(input, &fat32vars, &instr);
            }
            else printf("ls : Invalid number of arguments\n");
        }

        else if(strcmp(first, "cd")==0) {
            if(instr.numTokens < 3) {
                fat32cd(input, &fat32vars, &instr);
	        }
            else printf("cd : Invalid number of arguments\n");
        }

        else if(strcmp(first, "creat")==0) {
            if(instr.numTokens == 2) {
                fat32creat(input, &fat32vars, &instr);
                fclose(input);
                input = fopen(argv[1], "r+");
            }
            else printf("create : Invalid number of arguments\n");
        }

        else if(strcmp(first, "mkdir")==0) {
            if(instr.numTokens == 2) {
                fat32mkdir(input, &fat32vars, instr.tokens[1]);
                fclose(input);
                input = fopen(argv[1], "r+");
            }
            else printf("mkdir : Invalid number of arguments\n");
        }

        else if(strcmp(first, "mv")==0) {
            if(instr.numTokens == 3) {
                fat32mv(input, &fat32vars, instr.tokens[1], instr.tokens[2]);
                fclose(input);
                input = fopen(argv[1], "r+");
            }
            else printf("mv : Invalid number of arguments\n");
        }

        else if(strcmp(first, "open")==0) {
            if(instr.numTokens == 3) {
                fat32open(&fat32vars, instr.tokens[1], instr.tokens[2]);
            }
            else printf("open : Invalid number of arguments\n");
        }

        else if(strcmp(first, "close")==0) {
            if(instr.numTokens == 2) {
                fat32close(&fat32vars, instr.tokens[1]);
            }
            else printf("close : Invalid number of arguments\n");
        }

        else if(strcmp(first, "read")==0) {
            if(instr.numTokens == 4) {
                fat32read(input, &fat32vars, instr.tokens[1], instr.tokens[2], instr.tokens[3]);                
            }
            else printf("read : Invalid number of arguments\n");
        }

        else if(strcmp(first, "write")==0) {
            //ensure input string is enclosed by "" and store it in char* inputString to be passed to function
            if (instr.numTokens >= 5){
                if (instr.tokens[4][0] == 34){      //if input string starts with "
                    int lastTokenSize = strlen(instr.tokens[instr.numTokens-1]);
                    if (instr.tokens[instr.numTokens-1][lastTokenSize-1] == 34){    //if input string ends with "
                        char* inputString = malloc(100);
                        strcpy(inputString, instr.tokens[4]);
                        int loop;
                        for (loop = 5; loop < instr.numTokens; loop++){
                            strcat(inputString, " ");
                            strcat(inputString, instr.tokens[loop]);
                        }
                        fat32write(input, &fat32vars, instr.tokens[1], instr.tokens[2], instr.tokens[3], inputString);
                        fclose(input);
                        input = fopen(argv[1], "r+");
                        free(inputString);
                    }
                    else{
                        printf("write : End of Input string formatted incorrectly\n");
                    }
                }
                else{
                        printf("write : Start of Input string formatted incorrectly\n");
                }
                
            }
            else printf("write : Invalid number of arguments\n");
        }

        else if(strcmp(first, "rm")==0) {
            if(instr.numTokens == 2) {
                fat32rm(input, &fat32vars, instr.tokens[1]);
                fclose(input);
                input = fopen(argv[1], "r+");
            }
            else printf("rm : Invalid number of arguments\n");
        }

        else if(strcmp(first, "cp")==0) {
            if(instr.numTokens == 3) {
                fat32cp(input, &fat32vars, &instr);
                fclose(input);
                input = fopen(argv[1], "r+");
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
    free(fat32vars.openFiles);

    //Close file
    fclose(input);

    return 0;
}
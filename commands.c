#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void fat32info(varStruct *fat32vars) {

    printf("Bytes per sector: %d\n", fat32vars->BPB_BytsPerSec);
    printf("Sectors per cluster: %d\n", fat32vars->BPB_SecPerClus);
    printf("Reserved sector count: %d\n", fat32vars->BPB_RsvdSecCnt);
    printf("Number of FATs: %d\n", fat32vars->BPB_NumFATs);
    printf("Total Sectors: %d\n", fat32vars->BPB_TotSec32);
    printf("FAT size: %d\n", fat32vars->BPB_FATSz32);
    printf("Root Cluster: %d\n", fat32vars->BPB_RootClus);

}

void fat32size(FILE *image, varStruct *fat32vars, char *filename) {
    struct DIRENTRY *dir;

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(filename, ".") || !strcmp(filename, "..")) {
            printf("Invalid file\n");
            return;
        }
    }

    int loop;
    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(compareFilenames(dir->DIR_Name, filename) == 0) {

            if(dir->DIR_Attr == 16) {
                printf("File is directory.\n");
                return;
            }

            else {
                printf("Size of %s: %d bytes\n", dir->DIR_Name, littleToBigEndian(dir->DIR_FileSize));
                return;
            }

        }
    }

    printf("File not found\n");

}

void fat32ls(FILE *image, varStruct *fat32vars, instruction* instr_ptr) {

    int loop;

    if(instr_ptr->numTokens == 2) {
        struct DIRENTRY *dir;
        for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

            dir = &fat32vars->currentDirectories[loop];

            if(compareFilenames(dir->DIR_Name, instr_ptr->tokens[1]) == 0) {

                if(dir->DIR_Attr == 16) {
                    int originalOffset = fat32vars->currentDirectoryOffset;
                    fat32vars->currentDirectoryOffset = getDirectoryFirstOffset(fat32vars, dir);
                    fillDirectories(image, fat32vars);
                    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {
                        if(fat32vars->currentDirectories[loop].DIR_Attr == 15 || 
                        fat32vars->currentDirectories[loop].DIR_Name[0] == 229) continue;
                        if(fat32vars->currentDirectories[loop].DIR_Attr == 16) printf("\033[0;34m");
                        printf("%s\n", fat32vars->currentDirectories[loop].DIR_Name);
                        printf("\033[0m");
                    }
                    fat32vars->currentDirectoryOffset = originalOffset;
                    fillDirectories(image, fat32vars);
                    return;
                }

                else {
                    printf("Entry is a file.\n");
                    return;
                }

            }
        }
        printf("Directory not found.\n");
        return;
    }

    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {
        if(fat32vars->currentDirectories[loop].DIR_Attr == 15 || 
        fat32vars->currentDirectories[loop].DIR_Name[0] == 229) continue;
        if(fat32vars->currentDirectories[loop].DIR_Attr == 16) printf("\033[0;34m");
        printf("%s\n", fat32vars->currentDirectories[loop].DIR_Name);
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

            jumpToDirectory(fat32vars, &fat32vars->currentDirectories[1]);
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
    int loop;
    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(dir->DIR_Attr == 15) continue;

        if(compareFilenames(dir->DIR_Name, instr_ptr->tokens[1]) == 0) {
            
            if(dir->DIR_Attr != 16) {
                printf("File is not directory.\n");
                return;
            }
            else {
                jumpToDirectory(fat32vars, dir);
                fillDirectories(image, fat32vars);
                return;
            }
        }
    }
    printf("Directory not found\n");
}

void fat32creat(FILE *image, varStruct *fat32vars, instruction* instr_ptr) {
    struct DIRENTRY *dir;

    //Check to see if file already exists
    int loop;
    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(compareFilenames(dir->DIR_Name, instr_ptr->tokens[1]) == 0) {

            if(dir->DIR_Attr == 16) {
                printf("File is directory.\n");
                return;
            }
            else {
                printf("File already exists.\n");
                return;
            }

        }
    }

    int currDirCluster = offsetToCluster(fat32vars, fat32vars->currentDirectoryOffset);
    if(currDirCluster == fat32vars->BPB_RootClus) currDirCluster = 0;

    //find first empty space in current directory, store in freeDirOffset
    int freeDirOffset = getNextAvailableEntry(image, fat32vars, fat32vars->currentDirectoryOffset);
    //Create directory entry in current directory
    char* fileEntry = makeFileEntryString(fat32vars, instr_ptr->tokens[1]);
    fseek(image, freeDirOffset, SEEK_SET);
    fwrite(fileEntry, 1, 32, image);
    free(fileEntry);


    //update currentDirectories with newly created directory
    fillDirectories(image, fat32vars);


}

void fat32mkdir(FILE *image, varStruct *fat32vars, char *directoryName) {

    if(strlen(directoryName) > 8) {
        printf("Directory name too long.\n");
        return;
    }

    //Check if that directory already exists
    int loop;
    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {
        if (!compareFilenames(directoryName, fat32vars->currentDirectories[loop].DIR_Name)) {
            printf("Directory already exists\n");
            return;
        }
    }

    //find first empty space in current directory, store in freeDirOffset
    int freeDirOffset = getNextAvailableEntry(image, fat32vars, fat32vars->currentDirectoryOffset);

    int firstFreeClus = getFirstFreeCluster(image, fat32vars);
    markFATentry(image, fat32vars, firstFreeClus, 0);
    int freeClusOffset = clusterToOffset(fat32vars, firstFreeClus);
    int currDirCluster = offsetToCluster(fat32vars, fat32vars->currentDirectoryOffset);
    if(currDirCluster == fat32vars->BPB_RootClus) currDirCluster = 0;

    //Create directory entry in current directory
    char* dirEntry = makeDirEntryString(fat32vars, directoryName, firstFreeClus);
    fseek(image, freeDirOffset, SEEK_SET);
    fwrite(dirEntry, 1, 32, image);
    free(dirEntry);

    //Create "." and ".." directories in new directory
    dirEntry = makeDirEntryString(fat32vars, ".", firstFreeClus);
    fseek(image, freeClusOffset, SEEK_SET);
    fwrite(dirEntry, 1, 32, image);
    free(dirEntry);
    dirEntry = makeDirEntryString(fat32vars, "..", currDirCluster);
    fseek(image, freeClusOffset+32, SEEK_SET);
    fwrite(dirEntry, 1, 32, image);
    free(dirEntry);

    //update currentDirectories with newly created directory
    fillDirectories(image, fat32vars);
}

void fat32mv(FILE *image, varStruct *fat32vars, char *FROM, char *TO) {
    struct DIRENTRY *dir, *dirFROM = NULL, *dirTO = NULL;
    int fromloop, toloop;

    if(strlen(TO) > 8) {
        printf("Filename too long\n");
        return;
    }

    //Handle the "." and ".." cases for FROM
    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(FROM, ".") || !strcmp(FROM, "..")) {
            printf("Invalid file\n");
            return;
        }
        if(!strcmp(TO, ".")) {
            return;
        }
    }

    //Navigate to file in FROM
    for(fromloop = 0; fromloop < fat32vars->numDirectories; ++fromloop) {

        dir = &fat32vars->currentDirectories[fromloop];

        if(dir->DIR_Attr == 15) continue;

        if(!compareFilenames(dir->DIR_Name, FROM)) {
            
            if(dir->DIR_Attr == 16) {
                printf("FROM is a directory.\n");
                return;
            }
            else {
                dirFROM = dir;
                break;
            }
        }
    }

    //FROM was not found, print error
    if(dirFROM == NULL) {
        printf("%s not found\n", FROM);
        return;
    }

    if(openFileIndex(fat32vars, dirFROM->DIR_Name) >= 0) {
        printf("Cannot move opened file\n");
        return;
    }

    //Navigate to directory in TO
    for(toloop = 0; toloop < fat32vars->numDirectories; ++toloop) {

        dir = &fat32vars->currentDirectories[toloop];

        if(dir->DIR_Attr == 15) continue;

        if(!compareFilenames(dir->DIR_Name, TO)) {
            if(dir->DIR_Attr != 16) {
                printf("File already exists\n");
                return;
            }
            else {
                dirTO = dir;
                break;
            }
        }
    }

    if(dirTO != NULL) {
        int nextAvailableEntry = getNextAvailableEntry(image, fat32vars, getDirectoryFirstOffset(fat32vars, dirTO));
        unsigned char * temp;
        temp = directoryToString(dirFROM);
        fseek(image, nextAvailableEntry, SEEK_SET);
        fwrite(temp, 32, 1, image);
        free(temp);
        
        int dirSize = 1;
        dir = &fat32vars->currentDirectories[fromloop];
        if(fromloop > 0) {
            dir = &fat32vars->currentDirectories[fromloop-1];
            if(dir->DIR_Attr == 15) dirSize = 2;
            else dir = &fat32vars->currentDirectories[fromloop];
        }
        fseek(image, dir->entryOffset, SEEK_SET);
        temp = malloc(32*dirSize);
        memset(temp, 0, 32*dirSize);
        if(fromloop < fat32vars->numDirectories-1) { 
            temp[0] = 229;
            if(dirSize==2) temp[32] = 229;
        }
        fwrite(temp, 1, 32*dirSize, image);
        free(temp);
        fillDirectories(image, fat32vars);
        return;
    }

    else {
        char temp[11];
        memset(temp, 32, 11);
        int loop;
        for(loop = 0; loop < strlen(TO); ++loop) {
            temp[loop] = toupper(TO[loop]);
        }
        fseek(image, dirFROM->entryOffset, SEEK_SET);
        fwrite(temp, 1, 11, image);
        fillDirectories(image, fat32vars);
        return;
    }

}

void fat32open(varStruct *fat32vars, char* filename, char *mode) {
    struct DIRENTRY *dir;
    fileStruct *file;
    int loop;

    int modeInt;
    if(!strcmp(mode, "r")) modeInt = 1;
    else if(!strcmp(mode, "w")) modeInt = 2;
    else if(!strcmp(mode, "rw") || !strcmp(mode, "wr")) modeInt = 3;
    else {
        printf("Invalid mode\n");
        return;
    }

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(filename, ".") || !strcmp(filename, "..")) {
            printf("Invalid file\n");
            return;
        }
    }

    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(dir->DIR_Attr == 15) continue;

        if(compareFilenames(dir->DIR_Name, filename) == 0) {

            for(loop = 0; loop < fat32vars->numOpenFiles; ++loop) {
                if(getDirectoryFirstOffset(fat32vars, dir) == fat32vars->openFiles[loop].offset &&
                !compareFilenames(basename(fat32vars->openFiles[loop].path), filename)) {
                    printf("File already open.\n");
                    return;
                }
            }

            file = &fat32vars->openFiles[fat32vars->numOpenFiles];

            char *fullpath = malloc(strlen(fat32vars->fat32pwd) + strlen(filename) + 2);
            strcpy(fullpath, fat32vars->fat32pwd);
            strcat(fullpath, "/");
            strcat(fullpath, filename);
            file->path = fullpath;
            printf("Opened file %s\n", file->path);

            file->offset = getDirectoryFirstOffset(fat32vars, dir);
            file->mode = modeInt;
            ++fat32vars->numOpenFiles;

            fat32vars->openFiles = 
            realloc(fat32vars->openFiles, 
            sizeof(fileStruct)*(fat32vars->numOpenFiles+1));
            return;
        }
    }
    printf("File not found\n");
}

void fat32close(varStruct *fat32vars, char* filename) {
    struct DIRENTRY *dir;
    fileStruct *file;
    int loop;

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(filename, ".") || !strcmp(filename, "..")) {
            printf("Invalid file\n");
            return;
        }
    }

    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(dir->DIR_Attr == 15) continue;

        if(compareFilenames(dir->DIR_Name, filename) == 0) {

            if(dir->DIR_Attr == 16) {
                printf("File is directory.\n");
                return;
            }
            else {
                for(loop = 0; loop < fat32vars->numOpenFiles; ++loop) {
                    file = &fat32vars->openFiles[loop];
                    if(!strcmp(basename(file->path), filename)) {

                        printf("Closed file %s\n", file->path);

                        memmove(
                            file, 
                            &fat32vars->openFiles[loop+1], 
                            sizeof(fileStruct) * (fat32vars->numOpenFiles - loop));

                        --fat32vars->numOpenFiles;
                        fat32vars->openFiles = realloc(fat32vars->openFiles, sizeof(fileStruct)*(fat32vars->numOpenFiles+1));
                        return;
                    }
                }
                printf("File not open\n");
                return;
            }
        }
    }
    printf("File not found\n");
}

void fat32read(FILE *image, varStruct *fat32vars, char* filename, char *offsetString, char *sizeString) {
    struct DIRENTRY *dir;
    fileStruct *file;
    int loop, offset, size;

    offset = atoi(offsetString);
    size = atoi(sizeString);

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(filename, ".") || !strcmp(filename, "..")) {
            printf("Invalid file\n");
            return;
        }
    }

    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(dir->DIR_Attr == 15) continue;

        if(compareFilenames(dir->DIR_Name, filename) == 0) {
            int isOpen;
            if((isOpen = openFileIndex(fat32vars, dir->DIR_Name)) < 0) {
                printf("File not open\n");
                return;
            }
            if(littleToBigEndian(dir->DIR_FileSize) == 0) {
                printf("File is empty.\n");
                return;
            }

            file = &fat32vars->openFiles[isOpen];

            if(file->mode == 2) {
                printf("File is in write only mode\n");
                return;
            }

            int filesize = littleToBigEndian(dir->DIR_FileSize);

            if((offset + size) > filesize) {
                printf("Offset + Size is too large\n");
                return;
            }

            int readCluster = getDirectoryFirstCluster(fat32vars, dir);
            int readOffset = clusterToOffset(fat32vars, readCluster) + offset;
            fseek(image, readOffset, SEEK_SET);
            unsigned char *output = malloc(size+1);
            for(loop = 0; loop < size; ++loop) {
                output[loop] = fgetc(image);
                if((ftell(image)%fat32vars->BPB_BytsPerSec) == 0) {
                    readCluster = getNextClusterNumber(image, fat32vars, readCluster);
                    readOffset = clusterToOffset(fat32vars, readCluster);
                    fseek(image, readOffset, SEEK_SET);
                }
            }

            output[size] = '\0';
            printf("%s\n", output);
            free(output);
            return;
        }
    }
    printf("File not found\n");
}

void fat32write(FILE *image, varStruct *fat32vars, char* filename, char* offsetString, char* sizeString, char* inputString) {
    struct DIRENTRY *dir;
    fileStruct *file;
    int loop1, loop2, offset, size, readSector, bpc;

    offset = atoi(offsetString);
    size = atoi(sizeString);

    //remove quotes from inputString
    char *writeString = malloc(size);
    memset(writeString, 0, size);
    inputString++;
    inputString[strlen(inputString)-1] = 0;
    memcpy(writeString, inputString, strlen(inputString));

    bpc = fat32vars->BPB_BytsPerSec * fat32vars->BPB_SecPerClus;

    //cannot write to . and ..
    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(filename, ".") || !strcmp(filename, "..")) {
            printf("Invalid file\n");
            return;
        }
    }

    for(loop1 = 0; loop1 < fat32vars->numDirectories; ++loop1) {

        dir = &fat32vars->currentDirectories[loop1];

        if(dir->DIR_Attr == 15) continue;

        if(compareFilenames(dir->DIR_Name, filename) == 0) {
            if(dir->DIR_Attr == 16) {
                printf("File is directory.\n");
                return;
            }
            else {
                for(loop2 = 0; loop2 < fat32vars->numOpenFiles; ++loop2) {
                    file = &fat32vars->openFiles[loop2];
                    if(!strcmp(basename(file->path), filename)) {
                        if(file->mode == 1) {
                            printf("File is in read only mode\n");
                            return;
                        }
                        int writeCluster = getDirectoryFirstCluster(fat32vars, dir);
                        int filesize = littleToBigEndian(dir->DIR_FileSize);
                        if((offset + size) > filesize) {    //have to make more clusters
                            //calculate number of extra clusters to make
                            int i;
                            int currClusters = filesize / bpc;
                            if (filesize % bpc > 0){
                                currClusters++;
                            }
                            int finalClusters = (offset+size) / bpc;
                            if ((offset+size) % bpc > 0){
                                finalClusters++;
                            }
                            int extraClusters = finalClusters - currClusters;
                            //create new clusters
                            for (i = 0; i < extraClusters; i++){
                                expandCluster(image, fat32vars, writeCluster);
                            }
                            
                            //increase filesize and write to direntry
                            int newFileSize = offset+size;
                            unsigned char sizeEntry[4];
                            dir->DIR_FileSize[0] = bitExtracted(newFileSize, 8, 1);
                            dir->DIR_FileSize[1] = bitExtracted(newFileSize, 8, 9);
                            dir->DIR_FileSize[2] = bitExtracted(newFileSize, 8, 17);
                            dir->DIR_FileSize[3] = bitExtracted(newFileSize, 8, 25);
                            fseek(image, dir->entryOffset+28, SEEK_SET);
                            fwrite(dir->DIR_FileSize, 1, 4, image);
                        }
                        int writeOffset = clusterToOffset(fat32vars, writeCluster) + offset;
                        fseek(image, writeOffset, SEEK_SET);
                        for(loop2 = 0; loop2 < size; ++loop2) {     //write 1 char at a time, moving to next cluster if needed
                            fputc(writeString[loop2], image);
                            if((ftell(image)%fat32vars->BPB_BytsPerSec) == 0) {
                                int currCluster = offsetToCluster(fat32vars, ftell(image) - fat32vars->BPB_BytsPerSec);
                                offset -= (loop2+1);
                                writeCluster = getNextClusterNumber(image, fat32vars, currCluster);
                                writeOffset = clusterToOffset(fat32vars, writeCluster);
                                fseek(image, writeOffset, SEEK_SET);
                            }
                        }
                        free(writeString);
                        return;
                    }
                }
                printf("File is not open\n");
            }
        }

    }
}

void fat32rm(FILE *image, varStruct *fat32vars, char* filename) {
    struct DIRENTRY *dir;
    int loop;
    int dirSize = 1;

    if(fat32vars->currentDirectoryOffset != fat32vars->rootDirectoryOffset) {
        if(!strcmp(filename, ".") || !strcmp(filename, "..")) {
            printf("Invalid file\n");
            return;
        }
    }

    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(dir->DIR_Attr == 15) continue;

        if(compareFilenames(dir->DIR_Name, filename) == 0) {

            if(dir->DIR_Attr == 16) {
                printf("File is directory.\n");
                return;
            }
            else {
                if(openFileIndex(fat32vars, filename) >= 0) fat32close(fat32vars, filename);

                //Clear out FAT entries
                clearFATentries(image, fat32vars, dir);
                
                //Clear out directory entry
                if(loop > 0) {
                    dir = &fat32vars->currentDirectories[loop-1];
                    if(dir->DIR_Attr == 15) dirSize = 2;
                    else dir = &fat32vars->currentDirectories[loop];
                }
                fseek(image, dir->entryOffset, SEEK_SET);
                unsigned char *temp = malloc(32*dirSize);
                memset(temp, 0, 32*dirSize);
                if(loop < fat32vars->numDirectories-1) { 
                    temp[0] = 229;
                    if(dirSize==2) temp[32] = 229;
                }
                fwrite(temp, 1, 32*dirSize, image);
                free(temp);
                fillDirectories(image, fat32vars);
                return;
            }
        }
    }
    printf("File not found\n");
}

void fat32cp(FILE *image, varStruct *fat32vars, instruction* instr_ptr) {
    struct DIRENTRY *dir;
    char filename[200], TO[200];

    strcpy(filename, instr_ptr->tokens[1]);

    if(instr_ptr->numTokens == 3){
      strcpy(TO, "TO");
    }
    else if(instr_ptr->numTokens == 4){

      strcpy(TO, instr_ptr->tokens[3]);
      int loop, found = 0;
      for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

          dir = &fat32vars->currentDirectories[loop];

          if(compareFilenames(dir->DIR_Name, TO) == 0) {
              if(dir->DIR_Attr == 16){
                //printf("Directory destination exists.\n");
                found = 1;
              }
          }
      }

      if(!found){
        strcpy(TO, "TO");
      }

    }


    //Check to see if file already exists
    int loop, found = 1;
    for(loop = 0; loop < fat32vars->numDirectories; ++loop) {

        dir = &fat32vars->currentDirectories[loop];

        if(compareFilenames(dir->DIR_Name, filename) == 0) {

            if(dir->DIR_Attr == 16) {
                printf("Directory with the same name exits.\n");
                return;
            }

        }
    }

    //printf("cp %s to %s\n",filename, TO);

    ///////////////////////////////////Error Checking//////////////////////////////

    struct DIRENTRY  *dirFROM = NULL, *dirTO = NULL;
    int fromloop, toloop;

    //Navigate to file in FROM
    for(fromloop = 0; fromloop < fat32vars->numDirectories; ++fromloop) {

        dir = &fat32vars->currentDirectories[fromloop];

        if(dir->DIR_Attr == 15) continue;

        if(!compareFilenames(dir->DIR_Name, filename)) {

            if(dir->DIR_Attr == 16) {
                printf("FROM is a directory.\n");
                return;
            }
            else {
                dirFROM = dir;
                break;
            }
        }
    }

    // //FROM was not found, print error
    // if(dirFROM == NULL) {
    //     printf("%s not found\n", FROM);
    //     return;
    // }


    if(openFileIndex(fat32vars, filename) >= 0) {
        printf("Cannot move opened file\n");
        return;
    }

    //Navigate to directory in TO
    for(toloop = 0; toloop < fat32vars->numDirectories; ++toloop) {

        dir = &fat32vars->currentDirectories[toloop];

        if(dir->DIR_Attr == 15) continue;

        if(!compareFilenames(dir->DIR_Name, TO)) {
            if(dir->DIR_Attr != 16) {
                //printf("File already exists\n");
                return;
            }
            else {
                dirTO = dir;
                break;
            }
        }
    }

    if(dirTO != NULL) {
        int currentOffset = getDirectoryFirstOffset(fat32vars, dirTO);
        fseek(image, currentOffset, SEEK_SET);
        struct DIRENTRY tempDir;
        while(1){
            fillDirectoryEntry(image, &tempDir, ftell(image));
            if (tempDir.DIR_Name[0] == 0 || tempDir.DIR_Name[0] == 229) break;
            if(ftell(image) % fat32vars->BPB_BytsPerSec == 0) {
                int currentOffset = getNextClusterOffset(
                    image,
                    fat32vars,
                    currentOffset);
                fseek(image, currentOffset, SEEK_SET);
            }
        }
        unsigned char *temp;
        temp = directoryToString(dirFROM);
        fseek(image, tempDir.entryOffset, SEEK_SET);
        fwrite(temp, 32, 1, image);
        free(temp);

        int dirSize = 1;
        dir = dir = &fat32vars->currentDirectories[fromloop];
        if(fromloop > 0) {
            dir = &fat32vars->currentDirectories[fromloop-1];
            if(dir->DIR_Attr == 15) dirSize = 2;
            else dir = &fat32vars->currentDirectories[fromloop];
        }
        fseek(image, dir->entryOffset, SEEK_SET);
        fillDirectories(image, fat32vars);
        return;
    }

    else {
        char temp[11];
        memset(temp, 32, 11);
        int loop;
        for(loop = 0; loop < strlen(TO); ++loop) {
            temp[loop] = toupper(TO[loop]);
        }
        fseek(image, dirFROM->entryOffset, SEEK_SET);
        fwrite(temp, 1, 11, image);
        fillDirectories(image, fat32vars);
        return;
    }
}

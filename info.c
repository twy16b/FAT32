#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int littleToBigEndian(uint8_t *address, int bytes) {
    if(bytes == 2) return address[0] | address[1] << 8;
    if(bytes == 4) return address[0] | address[1] << 8 | address[2] << 16 | address[3] << 24;
}

void info(uint8_t *BYTE_ARRAY) {

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

int main (int argc, char* argv[]) {
    //Variables
    FILE *input;
    int offset;
    uint8_t inputByte;
    uint8_t *BYTE_ARRAY;
    BYTE_ARRAY = malloc(100000000); //100 MB

    //Check arguments
    //Format "./info [fat32 image filename]"
    if(argc != 2) {
        printf("Invalid number of arguments\n");
        return 1;
    }

    //Open file passed in through command line
    //"rb" opens file for reading as binary
    input = fopen(argv[1], "rb");

    //Loop through entire file putting each byte in the BYTE_ARRAY
    offset = 0;
    while(!feof(input)) {
        inputByte = fgetc(input);

        BYTE_ARRAY[offset] = inputByte;

        ++offset;
    };

    //Print info
    info(BYTE_ARRAY);

    //Close file
    fclose(input);

    free(BYTE_ARRAY);

    return 0;
}


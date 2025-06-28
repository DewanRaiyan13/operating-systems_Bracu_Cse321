#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BLOCK_SIZE 4096
#define TOTAL_BLOCKS 64
#define INODE_SIZE 256
#define INODE_COUNT (5 * BLOCK_SIZE / INODE_SIZE)
#define INODE_BITMAP_SIZE ((INODE_COUNT + 7) / 8)
#define MAGIC_NUMBER 0xD34D
#define DATA_BLOCK_START 8
#define INODE_TABLE_START 3
#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK 2
#define NUM_DATA_BLOCKS (TOTAL_BLOCKS - DATA_BLOCK_START)
#define NUM_DIRECT_POINTERS 12

typedef struct {
        uint16_t magicNumber;
        uint32_t blockSize;
        uint32_t totalBlocks;
        uint32_t inodeBitmap;
        uint32_t dataBitmap;
        uint32_t inodeTable;
        uint32_t dataStart;
        uint32_t inodeSize;
        uint32_t inodeCount;
        uint8_t reservedBytes[4058];
} Superblock;

typedef struct {
        uint32_t fileMode;
        uint32_t userId;
        uint32_t groupId;
        uint32_t fileSize;
        uint32_t accessTime;
        uint32_t creationTime;
        uint32_t modificationTime;
        uint32_t deletionTime;
        uint32_t hardLinks;
        uint32_t dataBlocks;
        uint32_t directPointers[NUM_DIRECT_POINTERS];
        uint32_t indirectPointer;
        uint32_t doubleIndirectPointer;
        uint32_t tripleIndirectPointer;
        uint8_t reservedBytes[156];
} Inode;

int fd;
uint8_t inodeBmap[BLOCK_SIZE];
uint8_t dataBmap[BLOCK_SIZE];
uint8_t newInodeBmap[INODE_BITMAP_SIZE];
uint8_t newDataBmap[BLOCK_SIZE];
Superblock superBlock;

int superblock_validator() {
        printf("\nSuperblock Validator:  \n");
        if (pread(fd, &superBlock, sizeof(Superblock), 0) != sizeof(Superblock)) {
                fprintf(stderr, "Error reading superblock\n");
                return 0;
        }
        int valid = 1;
        if (superBlock.magicNumber == MAGIC_NUMBER) {
                printf("Magic number (must be 0x%x)- checked and verified\n", MAGIC_NUMBER);
        } else {
                fprintf(stderr, "Magic number (must be 0x%x)- invalid, expected 0x%x, got 0x%x\n",
                        MAGIC_NUMBER, MAGIC_NUMBER, superBlock.magicNumber);
                valid = 0;
        }
        if (superBlock.blockSize == BLOCK_SIZE) {
                printf("Block size (must be %u)- checked and verified\n", BLOCK_SIZE);
        } else {
                fprintf(stderr, "Block size (must be %u)- invalid, expected %u, got %u\n",
                        BLOCK_SIZE, BLOCK_SIZE, superBlock.blockSize);
                valid = 0;
        }
        if (superBlock.totalBlocks == TOTAL_BLOCKS) {
                printf("Total number of blocks (must be %u)- checked and verified\n", TOTAL_BLOCKS);
        } else {
                fprintf(stderr, "Total number of blocks (must be %u)- invalid, expected %u, got %u\n",
                        TOTAL_BLOCKS, TOTAL_BLOCKS, superBlock.totalBlocks);
                valid = 0;
        }
        int pointersValid = 1;
        if (superBlock.inodeBitmap != INODE_BITMAP_BLOCK) {
                fprintf(stderr, "Inode bitmap pointer (must be %u)- invalid, expected %u, got %u\n",
                        INODE_BITMAP_BLOCK, INODE_BITMAP_BLOCK, superBlock.inodeBitmap);
                pointersValid = 0;
        }
        if (superBlock.dataBitmap != DATA_BITMAP_BLOCK) {
                fprintf(stderr, "Data bitmap pointer (must be %u)- invalid, expected %u, got %u\n",
                        DATA_BITMAP_BLOCK, DATA_BITMAP_BLOCK, superBlock.dataBitmap);
                pointersValid = 0;
        }
        if (superBlock.inodeTable != INODE_TABLE_START) {
                fprintf(stderr, "Inode table start (must be %u)- invalid, expected %u, got %u\n",
                        INODE_TABLE_START, INODE_TABLE_START, superBlock.inodeTable);
                pointersValid = 0;
        }
        if (superBlock.dataStart != DATA_BLOCK_START) {
                fprintf(stderr, "Data block start (must be %u)- invalid, expected %u, got %u\n",
                        DATA_BLOCK_START, DATA_BLOCK_START, superBlock.dataStart);
                pointersValid = 0;
        }
        if (pointersValid) {
                printf("Validity of key block pointers: inode bitmap, data bitmap, inode table start, data block start - checked and verified\n");
        } else {
                valid = 0;
        }
        if (superBlock.inodeSize == INODE_SIZE && superBlock.inodeCount == INODE_COUNT) {
                printf("Inode size (%u) and count constraints (%u)- checked and verified\n", INODE_SIZE, INODE_COUNT);
        } else {
                fprintf(stderr, "Inode size (%u) and count constraints (%u)- invalid, got size %u, count %u\n",
                        INODE_SIZE, INODE_COUNT, superBlock.inodeSize, superBlock.inodeCount);
                valid = 0;
        }
        if (!valid) {
                printf("Fixing superblock...\n\n");
                superBlock.magicNumber = MAGIC_NUMBER;
                superBlock.blockSize = BLOCK_SIZE;
                superBlock.totalBlocks = TOTAL_BLOCKS;
                superBlock.inodeBitmap = INODE_BITMAP_BLOCK;
                superBlock.dataBitmap = DATA_BITMAP_BLOCK;
                superBlock.inodeTable = INODE_TABLE_START;
                superBlock.dataStart = DATA_BLOCK_START;
                superBlock.inodeSize = INODE_SIZE;
                superBlock.inodeCount = INODE_COUNT;
                if (pwrite(fd, &superBlock, sizeof(Superblock), 0) != sizeof(Superblock)) {
                        printf("Error writing superblock\n");
                } else {
                        printf("Fixed superblock\n\n");
                }
        }
        return valid;
}

int inode_bitmap_consistency_checker() {
        printf("\n\nInode Bitmap Consistency Checker:\n");
        if (pread(fd, inodeBmap, BLOCK_SIZE, INODE_BITMAP_BLOCK * BLOCK_SIZE) != BLOCK_SIZE) {
                printf("Error reading inode bitmap\n");
                return 0;
        }
        int valid = 1;
        int validInodes = 0, invalidInodes = 0;
        int markedInodes = 0, unmarkedInodes = 0;
        int inconsistencies = 0;
        memset(newInodeBmap, 0, INODE_BITMAP_SIZE);
        for (int i = 0; i < INODE_COUNT; i++) {
                Inode inode;
                off_t offset = (INODE_TABLE_START * BLOCK_SIZE) + (i * INODE_SIZE);
                if (pread(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                        printf("Error reading inode %d\n", i);
                        valid = 0;
                        continue;
                }
                int isValid = (inode.hardLinks > 0 && inode.deletionTime == 0);
                int isMarked = (inodeBmap[i / 8] >> (i % 8)) & 1;
                if (isValid) validInodes++; else invalidInodes++;
                if (isMarked) markedInodes++; else unmarkedInodes++;
                if (isValid && !isMarked) {
                        printf("Inode %d is valid but not marked in bitmap\n", i);
                        valid = 0;
                        inconsistencies++;
                        newInodeBmap[i / 8] |= (1 << (i % 8));
                } else if (!isValid && isMarked) {
                        printf("Inode %d is invalid but marked in bitmap\n", i);
                        valid = 0;
                        inconsistencies++;
                        memset(&inode, 0, INODE_SIZE);
                        if (pwrite(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                                printf("Error clearing inode %d\n", i);
                        } else {
                                printf("Cleared invalid inode %d\n", i);
                        }
                } else if (isValid && isMarked) {
                        newInodeBmap[i / 8] |= (1 << (i % 8));
                }
        }
        printf("Summary: %d valid, %d invalid; %d marked, %d unmarked\n",
                validInodes, invalidInodes, markedInodes, unmarkedInodes);
        if (inconsistencies == 0) {
                printf("Inode bitmap has no inconsistencies\n\n");
        }
        if (!valid) {
                if (pwrite(fd, newInodeBmap, INODE_BITMAP_SIZE, INODE_BITMAP_BLOCK * BLOCK_SIZE) != INODE_BITMAP_SIZE) {
                        fprintf(stderr, "Error writing inode bitmap\n");
                } else {
                        printf("Fixed inode bitmap\n\n");
                }
                memcpy(inodeBmap, newInodeBmap, INODE_BITMAP_SIZE);
        }
        return valid;
}

int data_bitmap_consistency_checker() {
        printf("\nData Bitmap Consistency Checker: \n");
        if (pread(fd, dataBmap, BLOCK_SIZE, DATA_BITMAP_BLOCK * BLOCK_SIZE) != BLOCK_SIZE) {
                printf("Error reading data bitmap\n");
                return 0;
        }
        int valid = 1;
        int markedBlocks = 0, unmarkedBlocks = 0;
        int referencedBlocks = 0, unreferencedBlocks = 0;
        int inconsistencies = 0;
        memset(newDataBmap, 0, BLOCK_SIZE);
        int blockRefCount[TOTAL_BLOCKS] = {0};
        for (int i = 0; i < INODE_COUNT; i++) {
                Inode inode;
                off_t offset = (INODE_TABLE_START * BLOCK_SIZE) + (i * INODE_SIZE);
                if (pread(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                        fprintf(stderr, "Error reading inode %d\n", i);
                        valid = 0;
                        continue;
                }
                if (inode.hardLinks == 0 || inode.deletionTime != 0) {
                        continue;
                }
                for (int j = 0; j < NUM_DIRECT_POINTERS; j++) {
                        if (inode.directPointers[j] >= DATA_BLOCK_START && inode.directPointers[j] < TOTAL_BLOCKS) {
                                blockRefCount[inode.directPointers[j]]++;
                                newDataBmap[(inode.directPointers[j] - DATA_BLOCK_START) / 8] |= (1 << ((inode.directPointers[j] - DATA_BLOCK_START) % 8));
                        } else if (inode.directPointers[j] != 0) {
                                printf("Inode %d has invalid direct block %d: %u\n", i, j, inode.directPointers[j]);
                                valid = 0;
                                inconsistencies++;
                                uint32_t invalidBlock = inode.directPointers[j];
                                inode.directPointers[j] = 0;
                                if (pwrite(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                                        printf("Error clearing invalid block pointer in inode %d\n", i);
                                } else {
                                        printf("Cleared invalid block pointer %u in inode %d\n", invalidBlock, i);
                                }
                        }
                }
        }
        for (int i = 0; i < NUM_DATA_BLOCKS; i++) {
                int block = i + DATA_BLOCK_START;
                int isMarked = (dataBmap[i / 8] >> (i % 8)) & 1;
                if (isMarked) markedBlocks++; else unmarkedBlocks++;
                if (blockRefCount[block] > 0) referencedBlocks++; else unreferencedBlocks++;
                if (isMarked && blockRefCount[block] == 0) {
                        printf("Data block %d marked used but not referenced\n", block);
                        valid = 0;
                        inconsistencies++;
                } else if (!isMarked && blockRefCount[block] > 0) {
                        printf("Data block %d referenced but not marked used\n", block);
                        valid = 0;
                        inconsistencies++;
                        newDataBmap[i / 8] |= (1 << (i % 8));
                }
        }
        printf("Summary: %d marked, %d unmarked; %d referenced, %d unreferenced\n",
                markedBlocks, unmarkedBlocks, referencedBlocks, unreferencedBlocks);
        if (inconsistencies == 0) {
                printf("Data bitmap has no inconsistencies\n\n");
        }
        if (!valid) {
                if (pwrite(fd, newDataBmap, BLOCK_SIZE, DATA_BITMAP_BLOCK * BLOCK_SIZE) != BLOCK_SIZE) {
                        printf("Error writing data bitmap\n");
                } else {
                        printf("Fixed data bitmap\n\n");
                }
                memcpy(dataBmap, newDataBmap, BLOCK_SIZE);
        }
        return valid;
}

int duplicate_checker() {
        printf("Duplicate Checker: \n");
        int valid = 1;
        int blockRefCount[TOTAL_BLOCKS] = {0};
        int validInodesChecked = 0;
        for (int i = 0; i < INODE_COUNT; i++) {
                Inode inode;
                off_t offset = (INODE_TABLE_START * BLOCK_SIZE) + (i * INODE_SIZE);
                if (pread(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                        printf("Error reading inode %d\n", i);
                        valid = 0;
                        continue;
                }
                if (inode.hardLinks == 0 || inode.deletionTime != 0) {
                        continue;
                }
                validInodesChecked++;
                for (int j = 0; j < NUM_DIRECT_POINTERS; j++) {
                        if (inode.directPointers[j] >= DATA_BLOCK_START && inode.directPointers[j] < TOTAL_BLOCKS) {
                                blockRefCount[inode.directPointers[j]]++;
                        }
                }
        }
        int duplicatesFound = 0;
        for (int i = DATA_BLOCK_START; i < TOTAL_BLOCKS; i++) {
                if (blockRefCount[i] > 1) {
                        printf("block %d reference by %d inodes\n", i, blockRefCount[i]);
                        valid = 0;
                        duplicatesFound++;
                        int kept = 0;
                        for (int j = 0; j < INODE_COUNT; j++) {
                                Inode inode;
                                off_t offset = (INODE_TABLE_START * BLOCK_SIZE) + (j * INODE_SIZE);
                                if (pread(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                                        printf("Error reading inode %d\n", j);
                                        continue;
                                }
                                if (inode.hardLinks == 0 || inode.deletionTime != 0) {
                                        continue;
                                }
                                for (int k = 0; k < NUM_DIRECT_POINTERS; k++) {
                                        if (inode.directPointers[k] == i) {
                                                if (kept == 0 && blockRefCount[i] > 1) {
                                                        printf("Keeping reference to block %d in inode %d\n", i, j);
                                                        kept = 1;
                                                } else {
                                                        inode.directPointers[k] = 0;
                                                        if (pwrite(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                                                                printf("Error clearing duplicate block pointer in inode %d\n\n", j);
                                                        } else {
                                                                printf("Cleared duplicate reference to block %d in inode %d\n", i, j);
                                                        }
                                                        blockRefCount[i]--;
                                                }
                                        }
                                }
                        }
                }
        }
        printf("Checked %d valid inodes for duplicates\n", validInodesChecked);
        if (duplicatesFound == 0) {
                printf("No duplicate blocks found\n\n");
        }
        return valid;
}

int badBlock_checker() {
        printf("\nBad Block Checker: \n");
        int valid = 1;
        int validInodesChecked = 0;
        int badBlocksFound = 0;
        for (int i = 0; i < INODE_COUNT; i++) {
                Inode inode;
                off_t offset = (INODE_TABLE_START * BLOCK_SIZE) + (i * INODE_SIZE);
                if (pread(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                        printf("Error reading inode %d\n", i);
                        valid = 0;
                        continue;
                }
                if (inode.hardLinks == 0 || inode.deletionTime != 0) {
                        continue;
                }
                validInodesChecked++;
                for (int j = 0; j < NUM_DIRECT_POINTERS; j++) {
                        if (inode.directPointers[j] != 0 && (inode.directPointers[j] < DATA_BLOCK_START || inode.directPointers[j] >= TOTAL_BLOCKS)) {
                                printf("Inode %d has bad block %d: %u\n", i, j, inode.directPointers[j]);
                                valid = 0;
                                badBlocksFound++;
                                uint32_t badBlock = inode.directPointers[j];
                                inode.directPointers[j] = 0;
                                if (pwrite(fd, &inode, INODE_SIZE, offset) != INODE_SIZE) {
                                        printf("Error clearing bad block pointer in inode %d\n", i);
                                } else {
                                        printf("Cleared bad block %u in inode %d\n", badBlock, i);
                                }
                        }
                }
        }
        printf("Checked %d valid inodes for bad blocks\n", validInodesChecked);
        if (badBlocksFound == 0) {
                printf("No bad blocks found\n\n");
        }
        return valid;
}

int main(int argc, char *argv[]) {
        if (argc != 2) {
                printf("Not Enough Parameters. Give the img file in the terminal\n");
                return 1;
        }
        fd = open(argv[1], O_RDWR);
        if (fd < 0) {
                perror("Error opening file system image");
                return 1;
        }
        int allValid = 1;
        if (!superblock_validator()) {
                allValid = 0;
        }
        if (!inode_bitmap_consistency_checker()) {
                allValid = 0;
        }
        if (!data_bitmap_consistency_checker()) {
                allValid = 0;
        }
        if (!duplicate_checker()) {
                allValid = 0;
        }
        if (!badBlock_checker()) {
                allValid = 0;
        }
        if (!allValid) {
                printf("\nRe-validating file system...\n");
                allValid = 1;
                if (!superblock_validator()) {
                        fprintf(stderr, "Superblock still invalid\n");
                        allValid = 0;
                }
                if (!inode_bitmap_consistency_checker()) {
                        fprintf(stderr, "Inode bitmap still invalid\n");
                        allValid = 0;
                }
                if (!data_bitmap_consistency_checker()) {
                        fprintf(stderr, "Data bitmap still invalid\n");
                        allValid = 0;
                }
                if (!duplicate_checker()) {
                        fprintf(stderr, "Duplicates still present\n");
                        allValid = 0;
                }
                if (!badBlock_checker()) {
                        fprintf(stderr, "Bad blocks still present\n");
                        allValid = 0;
                }
        }
        printf("\nValidation complete.\n");
        if (allValid) {
                printf("File system is consistent\n");
        } else {
                printf("File system still has errors\n");
        }
        close(fd);
        if (allValid) {
                return 0;
        } else {
                return 1;
        }
}

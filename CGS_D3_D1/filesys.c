/* filesys.c
 * 
 * provides interface to virtual disk
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"


diskblock_t  virtualDisk [MAXBLOCKS] ;           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t   FAT         [MAXBLOCKS] ;           // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t   rootDirIndex            = 0 ;       // rootDir will be set by format
direntry_t * currentDir              = NULL ;
fatentry_t   currentDirIndex         = 0 ;

/* writedisk : writes virtual disk out to physical disk
 * 
 * in: file name of stored virtual disk
 */

void writedisk ( const char * filename )
{
   printf ( "writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data ) ;
   FILE * dest = fopen( filename, "w" ) ;
   if ( fwrite ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
   fclose(dest) ;
   
}

void readdisk ( const char * filename )
{
   FILE * dest = fopen( filename, "r" ) ;
   if ( fread ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
      fclose(dest) ;
}


/* the basic interface to the virtual disk
 * this moves memory around
 */

void writeblock ( diskblock_t * block, int block_address )
{
   //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
   memmove ( virtualDisk[block_address].data, block->data, BLOCKSIZE ) ;
   //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}

void copyFat(fatentry_t * FAT, unsigned int numOfFatBlocks)
{
	diskblock_t block;	
	unsigned int index = 0;
	for (unsigned int i = 1; i <= numOfFatBlocks; i++)
	{
		for (unsigned int j = 0; j < FATENTRYCOUNT; j++)
		{
			block.fat[j] = FAT[index];
			++index;
		}
		writeblock(&block, i);
	}
}

/* read and write FAT
 * 
 * please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
 *              our blocksize for the virtual disk is 1024, therefore
 *              we can store 512 FAT entries in one block
 * 
 *              how many disk blocks do we need to store the complete FAT:
 *              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
 *                each block is 1024 bytes long
 *              - our FAT has MAXBLOCKS entries, which is currently 1024
 *                each FAT entry is a fatentry_t, which is currently 2 bytes
 *              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
 *                FAT
 *              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat entries
 */

/* implement format()
 */
void format (char * disk)
{
	diskblock_t block;
	unsigned int i = 1;
	for (i = 0; i < BLOCKSIZE; i++)
	{
		block.data[i] = '\0';
	}
	// This copies a block of memory
	memcpy(block.data, disk, strlen(disk));
	writeblock(&block, 0);
	FAT[0] = ENDOFCHAIN;
	// unsigned so it's only positive
	unsigned int numOfFatBlocks;
	numOfFatBlocks = (unsigned int)(MAXBLOCKS + (FATENTRYCOUNT - 1)) / FATENTRYCOUNT;
	for (i = 1; i < numOfFatBlocks; i++)
	{
		FAT[i] = i + 1;
	}
	FAT[numOfFatBlocks] = ENDOFCHAIN; // End of FAT table
	FAT[numOfFatBlocks + 1] = ENDOFCHAIN; // root
	for (i = numOfFatBlocks + 2; i < MAXBLOCKS; i++)
	{
		FAT[i] = UNUSED;
	}
}

/* use this for testing
 */

void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}


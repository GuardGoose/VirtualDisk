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

void copyFat(fatentry_t *FAT)
{
  int numOfFatBlocks = (int)(MAXBLOCKS+(FATENTRYCOUNT-1))/FATENTRYCOUNT;
  diskblock_t block;
  int index = 0;
  for (int i = 1; i <= numOfFatBlocks; i++){
    for (int j = 0; j < FATENTRYCOUNT; j++){
      block.fat[j] = FAT[index];
      ++index;
    }
    writeblock(&block, i);
  }
}

void format (char * disk){
   diskblock_t block ;
   int i = 1;
   for (i = 0; i < BLOCKSIZE; i++){
     block.data[i] = '\0';
   }
   memcpy(block.data, disk, strlen(disk));
   writeblock(&block, 0);
   FAT[0] = ENDOFCHAIN;
   unsigned int numOfFatBlocks = (MAXBLOCKS+(FATENTRYCOUNT - 1))/FATENTRYCOUNT;
   for (i = 1; i < numOfFatBlocks; i++){
     FAT[i] = i+1;
   }
   FAT[numOfFatBlocks] = ENDOFCHAIN; //end of fat table
   FAT[numOfFatBlocks+1] = ENDOFCHAIN; // root dir
   for(i = numOfFatBlocks+2; i < MAXBLOCKS; i++){
     FAT[i] = UNUSED;
   }
   copyFat(FAT);
   diskblock_t rootBlock;
   //call init block and get clean block
   rootBlock.dir.isdir = TRUE;
   rootBlock.dir.nextEntry = FALSE; //does not have to be if the block is zeroed
   rootDirIndex = numOfFatBlocks + 1;
   currentDirIndex = numOfFatBlocks + 1;
   direntry_t *emptyDir = calloc(1, sizeof(direntry_t));
   emptyDir->isdir = TRUE;
   emptyDir->unused = TRUE;
   emptyDir->filelength = 0;
   strcpy(emptyDir->name,"\0");
   for (i = 0; i < DIRENTRYCOUNT; i++){
     rootBlock.dir.entryList[i] = *emptyDir;
   }
   free(emptyDir);
   writeblock(&rootBlock, rootDirIndex);
}

int getFreeBlock()
{
  for(int i = 0; i < MAXBLOCKS; i++)
  {
    if (FAT[i] == UNUSED)
    	{
    		return i;
    	}
  }
  return -1;
}

diskblock_t initBlock(int index, const char type)
{
  diskblock_t block;
  if(type == DIR)
  {
    block.dir.isdir = TRUE;
    block.dir.nextEntry = 0;
    direntry_t *newEntry = malloc(sizeof(direntry_t));
    newEntry->unused = TRUE;
    newEntry->filelength = 0;
    for(int i = 0; i < DIRENTRYCOUNT; i ++){
      memcpy(&block.dir.entryList[i], newEntry, DIRENTRYCOUNT);
    }
    free(newEntry);
  }
  else
  {
    for (int i = 0; i < BLOCKSIZE; i++) 
    {
      block.data[i] = '\0';
    }
  }
  writeblock(&block, index);
  return block;
}

int findEntryIndex(const char * name)
{
  while(TRUE)
  {
    for(int i = 0; i < DIRENTRYCOUNT; i++)
    {
      if (strcmp(virtualDisk[currentDirIndex].dir.entryList[i].name, name) == 0)
      {
        return i;
      }
    }
    if(FAT[currentDirIndex] == ENDOFCHAIN)
    {
    	break;
    }
    currentDirIndex = FAT[currentDirIndex];
  }
  return -1;
}

int myRm (const char * name)
{
  int firstIndex = findEntryIndex(name);
  if (firstIndex == -1)
  {
    return 1;
  }
  int fatBlockIndex = virtualDisk[currentDirIndex].dir.entryList[firstIndex].firstBlock;
  virtualDisk[currentDirIndex].dir.entryList[firstIndex].firstBlock = 0;
  virtualDisk[currentDirIndex].dir.entryList[firstIndex].isdir = FALSE;
  virtualDisk[currentDirIndex].dir.entryList[firstIndex].unused = TRUE;
  virtualDisk[currentDirIndex].dir.entryList[firstIndex].filelength = 0;
  for (int i = 0; i < MAXNAME; i++) 
  {
    virtualDisk[currentDirIndex].dir.entryList[firstIndex].name[i] = '\0';
  }
  int nextIndex = fatBlockIndex;
  while(TRUE)
  {
    if(FAT[nextIndex] == ENDOFCHAIN)
    {
      FAT[nextIndex] = UNUSED;
      copyFat(FAT);
      return 0;
    }
    nextIndex = FAT[fatBlockIndex];
    FAT[fatBlockIndex] = UNUSED;
    fatBlockIndex = nextIndex;
  }
}

char myfgetc(MyFILE * stream)
{
  if (stream->pos >= BLOCKSIZE)
  {
    if (FAT[stream->blockno] == ENDOFCHAIN)
    {
      return EOF;
    }
    stream->blockno = FAT[stream->blockno];
    memcpy(&stream->buffer, &virtualDisk[stream->blockno], BLOCKSIZE);
    stream->pos = 1;
    return stream->buffer.data[stream->pos - 1];
  }
  if(stream->buffer.data[stream->pos] == EOF){
    return EOF;
  }
  ++stream->pos;
  return stream->buffer.data[stream->pos-1];
}

int myfputc(char a, MyFILE * stream)
{
  if (strcmp(stream->mode, "r") == 0) 
  	{
  		return 1;
  	}
  if (stream->pos >= BLOCKSIZE)
  {
    writeblock(&stream->buffer,stream->blockno);
    stream->pos = 0;
    int index = getFreeBlock();
    if(index == -1)
    {
      return 1;
    }
    diskblock_t newBlock = initBlock(index, DATA);
    memcpy(&stream->buffer, &newBlock, BLOCKSIZE);
    FAT[stream->blockno] = index;
    FAT[index] = ENDOFCHAIN;
    stream->blockno = index;
    copyFat(FAT);
  }
  stream->filelength += 1;
  stream->buffer.data[stream->pos] = a;
  stream->pos += 1;
  return 0;
}

MyFILE * myfopen(char * name,const char mode){
  if(mode == 'w')
  {
    myRm(name);
    int index = getFreeBlock();
    FAT[index] = ENDOFCHAIN;
    int i;
    for(i = 0; i < DIRENTRYCOUNT; i++)
    {
      if (virtualDisk[currentDirIndex].dir.entryList[i].unused == TRUE)
	  {
        virtualDisk[currentDirIndex].dir.entryList[i].unused = FALSE;
        strcpy(virtualDisk[currentDirIndex].dir.entryList[i].name, name);
        virtualDisk[currentDirIndex].dir.entryList[i].firstBlock = index;
        currentDir = &virtualDisk[currentDirIndex].dir.entryList[i];
        break;
      }
    }
    MyFILE * file = malloc(sizeof(MyFILE));
    memset(file, '\0', BLOCKSIZE);
    file->pos = 0;
    file->writing = 1;
    file->filelength = currentDir->filelength;
    strcpy(file->mode, &mode);
    file->blockno = index;
    memmove (&file->buffer, &virtualDisk[index], BLOCKSIZE );
    copyFat(FAT);
    return file;
  }
  int dirEntryIndex = findEntryIndex(name);
  MyFILE * file = malloc(sizeof(MyFILE));
  file->pos = 0;
  file->writing = 0;
  strcpy(file->mode, &mode);
  file->blockno = virtualDisk[currentDirIndex].dir.entryList[dirEntryIndex].firstBlock;
  currentDir = &virtualDisk[currentDirIndex].dir.entryList[dirEntryIndex];
  memcpy(&file->buffer, &virtualDisk[file->blockno], BLOCKSIZE);
  return file;
}

int myfclose(MyFILE *file)
{
  if (file->writing == 1)
  {
    myfputc(EOF, file);
    currentDir->filelength = file->filelength;
    writeblock(&file->buffer, file->blockno);
  }
  currentDir = NULL;
  free(file);
  return 0;
}

void printBlock ( int blockIndex )
{
   printf ( "virtualDisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"

MyFILE * myfopen(char * name, const char mode);
int myfputc(char a, MyFILE * stream);
char myfgetc(MyFILE * stream);
int myfclose(MyFILE *file);



void cgsD()
{
  format("CS3026 Operating Systems Assessment");
  writedisk("virtualdiskD3_D1\0");
}

void cgsC()
{
  MyFILE *myFile = myfopen("testfile.txt", 'w');
  char string[] = "4096bytes";
  int i;
  for (i = 0; i < 4*BLOCKSIZE; i++)
  {
    myfputc('A', myFile);
  }
  myfclose(myFile);
  FILE *realFile = fopen("testfileC3_C1_copy.txt", "w");
  MyFILE *myFile2 = myfopen("testfile.txt", 'r');
  while(1)
  {
    char character = myfgetc(myFile2);
    if (character == EOF)
	{
      break;
    }
    printf("%c", character);
    fprintf(realFile, "%c", character);
  }
  printf("\n");
  myfclose(myFile2);
  fclose(realFile);
  writedisk("virtualdiskC3_C1\0");
}

void main()
{
  //cgsD();
  cgsC();
}
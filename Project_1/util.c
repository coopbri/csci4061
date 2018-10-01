/************************
 * util.c
 *
 * utility functions
 *
 ************************/

#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**********************************************************************************************************************
 * These functions are just some handy file functions.
 * We have not yet covered opening and reading from files in C,
 * so we're saving you the pain of dealing with it, for now.
 *********************************************************************************************************************/


/*-------------------------------------------------------FILE OPENER-------------------------------------------------*/
FILE* file_open(char* filename)
{
	FILE* fp = fopen(filename, "r");
	if(fp == NULL)
	{
		fprintf(stderr, "make4061: %s: No such file or directory.\n", filename);
		exit(1);
	}
	return fp;
}
/*-------------------------------------------------------END OF FILE OPENER------------------------------------------*/


/*-------------------------------------------------------FILE LINE READER--------------------------------------------*/
char* file_getline(char* buffer, FILE* fp)
{
	buffer = fgets(buffer, 1024, fp);
	return buffer;
}
/*-------------------------------------------------------END OF FILE LINE READER-------------------------------------*/


/*-------------------------------------------------------FILE ACCESSBILITY-------------------------------------------*/
int does_file_exist(char * lpszFileName)
{
	//access functions returns -1 if there is an error
	return access(lpszFileName, F_OK);
}
/*-------------------------------------------------------ENDS OF FILE ACCESSBILITY-----------------------------------*/


/*-------------------------------------------------------FILE TIMESTAMP----------------------------------------------*/
int get_file_modification_time(char *lpszFileName)
{
	//Step1: Check the return value of access function from the does_file_exist
	if(does_file_exist(lpszFileName) != -1)
	{
		//Step1.1: Use the stat function to check the statistics of the file
		struct stat buf;
		stat(lpszFileName, &buf);
		return buf.st_mtime;
	}
	return -1;
}
/*-------------------------------------------------------END OF FILE TIMESTAMP---------------------------------------*/


/*-------------------------------------------------------TIMESTAMP COMPARATOR----------------------------------------*/
int compare_modification_time(char * lpsz1, char * lpsz2)
{	

	//Step1: Invoke the stat function in get_file_modification_time to get the timestamp of the files
	int nTime1 = get_file_modification_time(lpsz1);
	int nTime2 = get_file_modification_time(lpsz2);

	//Step2: Based on the return value check the possible combinations below

	//Either OR Both of the file is missing
	if(nTime1 == -1 || nTime2 == -1)
		return -1;

	//Both the files have identical timestamp
	if(nTime1 == nTime2)   //need to check this why can't it be else if
		return 0;

	//File1 is modified later than File2
	else if(nTime1 > nTime2)
		return 1;

	//File1 is modified earlier than File2
	else
		return 2;
}
/*-------------------------------------------------------END OF TIMESTAMP COMPARATOR----------------------------------*/


/*-------------------------------------------------------PARSE TOKENS------------------------------------------------*/
//Tis is to tokenize the input string
int parse_into_tokens(char *input_string, char *tokens[], char *delim)
{
	int i=0;
	char *tok = strtok(input_string, delim);
	while(tok!=NULL && i<ARG_MAX)
	{
		tokens[i] = tok;
		i++;
		tok = strtok(NULL, delim);
	}
	tokens[i] = NULL;
	return i;
}
/*-------------------------------------------------------END OF PARSE TOKENS-----------------------------------------*/


/*-------------------------------------------------------FIND NODE INDEX --------------------------------------------*/
int find_target(char * TargetName, target_t t[], int nTargetCount)
{
	//Step1: Find the node number or the array location with equivalent target name
	int i;
	for(i=0;i<nTargetCount;i++)
		if(strcmp(TargetName, t[i].TargetName) == 0)
			return i;
	return -1;
}
/*-------------------------------------------------------END OF NODE INDEX-------------------------------------------*/


/*-------------------------------------------------------MAKEFILE PARSER---------------------------------------------*/
int parse(char * lpszFileName, target_t * const t)
{
	char szLine[1024];
	char * lpszLine = NULL;
	char * TargetName = NULL;
	char * lpszDependency = NULL;
	FILE * fp = file_open(lpszFileName);
	int nTargetCount = 0;
	int nLine = 0;
	target_t * pTarget = t;
	int nPreviousTarget = 0;

	if(fp == NULL)
		return -1;

	while(file_getline(szLine, fp) != NULL)
  {
		nLine++;
		// this loop will go through the given file, one line at a time
		// this is where you need to do the work of interpreting
		// each line of the file to be able to deal with it later
		lpszLine = strtok(szLine, "\n"); //Remove newline character at end if there is one
		if(lpszLine == NULL || *lpszLine == '#') //skip if blank or comment
			continue;

		//Remove leading whitespace
		while(*lpszLine == ' ')
			lpszLine++;

		//skip if whitespace-only
		if(strlen(lpszLine) <= 0)
			continue;

		//Multi target is not allowed.
		if(*lpszLine == '\t') //Commmand
		{
			lpszLine++;
			if(strlen(pTarget->TargetName) == 0)
			{
				fprintf(stderr, "%s: line:%d *** specifying multiple commands is not allowed.  Stop.\n", lpszFileName, nLine);
				return -1;
			}

			strcpy(pTarget->Command, lpszLine);
			nPreviousTarget = 0;
			pTarget++;
		}
		else	//Target
		{
			//check : exist Syntax check
			if(strchr(lpszLine, ':') == NULL)
			{
				fprintf(stderr, "%s: line:%d *** missing separator.  Stop.\n", lpszFileName, nLine);
				return -1;
			}
			//Previous target don't have a command???
			if(nPreviousTarget == 1)
				pTarget++;

			//Not currently inside a target, look for a new one???
			TargetName = strtok(lpszLine, ":");

			if(TargetName != NULL && strlen(TargetName) > 0)
			{
				strcpy(pTarget->TargetName, TargetName);
				lpszDependency = strtok(NULL, " ");

				while (lpszDependency != NULL)
				{
					strcpy(pTarget->DependencyNames[pTarget->DependencyCount], lpszDependency);
					pTarget->DependencyCount++;
					lpszDependency = strtok(NULL, " ");
				}
				nTargetCount++;
				nPreviousTarget = 1;
			}
			else //error
			{
				fprintf(stderr, "%s: line:%d *** missing separator.  Stop.\n", lpszFileName, nLine);
				return -1;
			}
		}
  }

	fclose(fp);

	return nTargetCount;
}
/*-------------------------------------------------------END OF MAKEFILE PARSER--------------------------------------*/

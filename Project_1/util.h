/********************
 * util.h
 *
 * You may put your utility function definitions here
 * also your structs, if you create any
 *********************/
#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_NODES 10

/*-------------------------------------------------------HELPER DEFINES----------------------------------------------*/
#define ARG_MAX 1023
#define TRUE 1
#define FALSE 0
#define ERROR -1
#define EXISTS 1
#define NEEDS_BUILDING 2
#define UNFINISHED 0
#define FINISHED 1
/*-------------------------------------------------------END OF HELPER DEFINES---------------------------------------*/

/*-------------------------------------------------------STRUCTURE DEFINITION----------------------------------------*/
/*
 * A Vertex in the DAG
 */
typedef struct target{
  char TargetName[64]; //Target name
  int  DependencyCount; //Number of dependencies
  char DependencyNames[10][64]; //Names of all the dependencies
  char Command[256]; //Command that needs to be executed for this target
  int  Status; //Status of the target(Ready, Finished etc. based on your implementation)
}target_t;
/*-------------------------------------------------------END OF STRUCTURE DEFINITION---------------------------------*/


/*-------------------------------------------------------FILE ACCESSBILITY-------------------------------------------*/
/* Return -1 if file does not exist, otherwise return 0 */
int does_file_exist(char *FileName);
/*-------------------------------------------------------ENDS OF FILE ACCESSBILITY-----------------------------------*/


/*-------------------------------------------------------TIMESTAMP COMPARATOR----------------------------------------*/
/*
 * Compare timestamp of File1 and File2.
 *   Return -1 if File1 and/or File2 do not exist;
 *   Return 0 if File1 and File2 have identical timestamp;
 *   Return 1 if File1 is modified later than File2;
 *   Return 2 if File1 is modified earlier than File2.
 */
int compare_modification_time(char *FileName1, char *FileName2);
/*-------------------------------------------------------END OF TIMESTAMP COMPARATOR----------------------------------*/


/*-------------------------------------------------------MAKEFILE PARSER---------------------------------------------*/
/*
 * Parse input Makefile.
 *
 * Input:
 *   Makefile - file name
 *   targets - a pre-allocated array holding parsed target objects.
 * Result:
 *    0 SUCCESS
 *   -1 FAIL
 */
int parse(char *Makefile, target_t targets[]);
/*-------------------------------------------------------END OF MAKEFILE PARSER--------------------------------------*/


/*-------------------------------------------------------FIND NODE INDEX --------------------------------------------*/
/*
 * Compare TargetName with the TargetName in each target object.
 * Return the index of the target when found, otherwise return -1.
 *
 * Input:
 *   targets - targets array in main()
 *   nTargetCount - number of target objects in Makefile,
 *      which is returned by parse().
 * Result:
 *   Index of the given target
 *   -1 NOT FOUND
 */
int find_target(char *TargetName, target_t targets[], int nTargetCount);
/*-------------------------------------------------------END OF NODE INDEX-------------------------------------------*/


/*-------------------------------------------------------MAKEARGV TOKENS---------------------------------------------*/
/*
 * Tokenize Command to be the argv for execvp
 */
int parse_into_tokens(char *input_string, char *tokens[], char *delim);
/*-------------------------------------------------------END OF MAKEARGV TOKENS--------------------------------------*/


#endif

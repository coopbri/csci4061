/* CSci4061 F2018 Assignment 1
* login: cselabs_login_name                           (update)
* date : mm/dd/yy                                     (update)
* name: Brian Cooper, Chase Rogness, Jared Erlien
* id:     coope824,     rogne066,      erlie003   */

// This is the main file for the code
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"

/*-------------------------------------------------------HELPER FUNCTIONS PROTOTYPES---------------------------------*/
void show_error_message(char * ExecName);
//Write your functions prototypes here
void show_targets(target_t targets[], int nTargetCount);
void build(target_t target);
/*-------------------------------------------------------END OF HELPER FUNCTIONS PROTOTYPES--------------------------*/


/*-------------------------------------------------------HELPER FUNCTIONS--------------------------------------------*/

//This is the function for writing an error to the stream
//It prints the same message in all the cases
void show_error_message(char * ExecName)
{
	fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", ExecName);
	fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	exit(0);
}

//Write your functions here

//Phase1: Warmup phase for parsing the structure here. Do it as per the PDF (Writeup)
void show_targets(target_t targets[], int nTargetCount)
{
	//Write your warmup code here
	for(int i = 0; i < nTargetCount; i++) {
		int dependencyCount = targets[i].DependencyCount;
		printf("Target Name: %s\n", targets[i].TargetName);
		printf("DependencyCount: %d\n", dependencyCount);
		if(dependencyCount > 0) {
			printf("DependencyNames: ");
			for(int j = 0; j < dependencyCount; j++) {
				printf("%s ", targets[i].DependencyNames[j]);
			}
			printf("\n");
		}
		printf("Command: %s\n", targets[i].Command);
		printf("\n");
	}
}

void build(target_t target) {
	target.Status = 1;
	// While you have a DependencyCount of more than 0 we must continue until
	// We have a 0 DependencyCount.
	while(target.Status != 0) {
		// For loop from 0 to the DependencyCount of the target.
		// This is used to go through each Dependency and do checking.
		for(int i = 0; i < target.DependencyCount; i++) {
			// Check to see if the file already exists
			// If it exists, we will then need to check it's timestamp.
			if(does_file_exist(target.DependencyNames[i]) == 0) {
				// Get back an integer to tell you whether or not something must be rebuilt.
				int timestamp_check = compare_modification_time(target.TargetName, target.DependencyNames[i]);
				// Either both where just built or the target has been modified and the
				// Dependency has not therefore you don't need to rebuild.
				if(timestamp_check == 1 || timestamp_check == 0) {
					printf("Built Dependency Name: %s\n", target.DependencyNames[i]);
				}
				// If the Dependency needs to be built, you will recall build(target.DependencyNames[i]).
				else if(timestamp_check == 2) {
					printf("Need to be Rebuilt Dependency Name: %s\n", target.DependencyNames[i]);
				}
				// This case should never happen, but if so there will just be an error.
				else {
					printf("Error, no such file: %s", target.DependencyNames[i]);
					break;
				}
			}
			// If the file does not exist you will recall build.
			else {
				printf("Dependency Name: %s\n", target.DependencyNames[i]);
			}
		}
		target.Status = 0; //Remove this line.
		printf("Target Name: %s\n", target.TargetName);
	}
	// Fork/Exec/Wait statements happen down here.
}

/*-------------------------------------------------------END OF HELPER FUNCTIONS-------------------------------------*/


/*-------------------------------------------------------MAIN PROGRAM------------------------------------------------*/
//Main commencement
int main(int argc, char *argv[])
{
  target_t targets[MAX_NODES];
  int nTargetCount = 0;

  /* Variables you'll want to use */
  char Makefile[64] = "Makefile";
  char TargetName[64];

  /* Declarations for getopt. For better understanding of the function use the man command i.e. "man getopt" */
  extern int optind;   		// It is the index of the next element of the argv[] that is going to be processed
  extern char * optarg;		// It points to the option argument
  int ch;
  char *format = "f:h";
  char *temp;

  //Getopt function is used to access the command line arguments. However there can be arguments which may or may not need the parameters after the command
  //Example -f <filename> needs a finename, and therefore we need to give a colon after that sort of argument
  //Ex. f: for h there won't be any argument hence we are not going to do the same for h, hence "f:h"
  while((ch = getopt(argc, argv, format)) != -1)
  {
	  switch(ch)
	  {
	  	  case 'f':
	  		  temp = strdup(optarg);
	  		  strcpy(Makefile, temp);  // here the strdup returns a string and that is later copied using the strcpy
	  		  free(temp);	//need to manually free the pointer
	  		  break;

	  	  case 'h':
	  	  default:
	  		  show_error_message(argv[0]);
	  		  exit(1);
	  }

  }

  argc -= optind;
  if(argc > 1)   //Means that we are giving more than 1 target which is not accepted
  {
	  show_error_message(argv[0]);
	  return -1;   //This line is not needed
  }

  /* Init Targets */
  memset(targets, 0, sizeof(targets));   //initialize all the nodes first, just to avoid the valgrind checks

  /* Parse graph file or die, This is the main function to perform the toplogical sort and hence populate the structure */
  if((nTargetCount = parse(Makefile, targets)) == -1)  //here the parser returns the starting address of the array of the structure. Here we gave the makefile and then it just does the parsing of the makefile and then it has created array of the nodes
	  return -1;


  //Phase1: Warmup-----------------------------------------------------------------------------------------------------
  //Parse the structure elements and print them as mentioned in the Project Writeup
  /* Comment out the following line before Phase2 */
  // show_targets(targets, nTargetCount);
  //End of Warmup------------------------------------------------------------------------------------------------------

  /*
   * Set Targetname
   * If target is not set, set it to default (first target from makefile)
   */
  if(argc == 1)
		strcpy(TargetName, argv[optind]);    // here we have the given target, this acts as a method to begin the building
  else
  	strcpy(TargetName, targets[0].TargetName);  // default part is the first target

  /*
   * Now, the file has been parsed and the targets have been named.
   * You'll now want to check all dependencies (whether they are
   * available targets or files) and then execute the target that
   * was specified on the command line, along with their dependencies,
   * etc. Else if no target is mentioned then build the first target
   * found in Makefile.
   */

  //Phase2: Begins ----------------------------------------------------------------------------------------------------
  /*Your code begins here*/
	int target_index = find_target(TargetName, targets, nTargetCount);
	if(target_index == -1) {
		printf("Target %s does not exist.\n", TargetName);
	} else {
		build(targets[target_index]);
	}
  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/

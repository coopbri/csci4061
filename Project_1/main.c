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
int build(char TargetName[], target_t targets[], int nTargetCount);
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

// Takes in a Target Name, the targets array, and the nTargetCount
// builds the make file.
// Target Status':
// 0 - Has not been error checked.
// 1 - Has been error checked, ready to build.
// 2 - Has been built.
int build(char TargetName[], target_t targets[], int nTargetCount) {
	int target_index, status, timecheck;
	pid_t childpid;
	target_t target, targetDependency;
	char *tokens[32];
	target_index = find_target(TargetName, targets, nTargetCount);

	// Verifies that we have a valid target.
	if(target_index == -1) {
		printf("Invalid make target, %s\n", TargetName);
		return -1;
	}

	target = targets[target_index];
	// Error checking block.
	if(target.Status == 0) {
		// For loop to access target dependencies.
		for(int i = 0; i < target.DependencyCount; i++) {
			// Get the target index for the dependency.
			target_index = find_target(target.DependencyNames[i], targets, nTargetCount);

			// No target in make file, eg. main.c.
			if(target_index == -1) {
				// File does not exist, cannot continue.
				if(does_file_exist(target.DependencyNames[i]) == -1) {
					printf("Target/File: %s, does not exist.\n", target.DependencyNames[i]);
					return -1;
				}
				if(compare_modification_time(TargetName, target.DependencyNames[i]) == 1) {
					printf("make4061: '%s' is up to date.\n", TargetName);
					target.Status = 2;
				} else {
					target.Status = 1;
				}
			}
			else {
				timecheck = compare_modification_time(TargetName, target.DependencyNames[i]);
				targetDependency = targets[target_index];
				//Timestamp modification checks.
				switch(timecheck) {
					case -1:
						target.Status = 1;
						break;
					case 0:
						target.Status = 2;
						printf("make4061: '%s' is up to date.\n", TargetName);
						targetDependency.Status = 2;
						return 0;
					case 1:
						targetDependency.Status = 2;
						break;
					case 2:
						target.Status = 2;
						printf("make4061: '%s' is up to date.\n", TargetName);
						return 0;
				}
				if(build(target.DependencyNames[i], targets, nTargetCount) == -1) {
					return -1;
				}
			}
		}
	}

	// Fork/Exec/Wait block.
	if(target.Status == 1 || target.Status == 0) {
		printf("%s\n", target.Command);
		parse_into_tokens(target.Command, tokens, " ");
		childpid = fork();
		if(childpid > 0) {
			childpid = wait(&status);
			if(WEXITSTATUS(status) != 0) {
				printf("Child exited with error code=%d\n", WEXITSTATUS(status));
				exit(-1);
			}
			target.Status = 2;
			return 0;
		}
		else if(childpid == 0) {
			execvp(tokens[0], tokens);
		}
		else {
			perror("Fork problem");
			exit(-1);
		}
	}
	return 0;
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
	if(build(TargetName, targets, nTargetCount) == -1) {
		return -1;
	}
  /*End of your code*/
  //End of Phase2------------------------------------------------------------------------------------------------------

  return 0;
}
/*-------------------------------------------------------END OF MAIN PROGRAM------------------------------------------*/

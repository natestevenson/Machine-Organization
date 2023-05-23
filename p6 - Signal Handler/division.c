 ////////////////////////////////////////////////////////////////////////////////
 // Main File:        mySigHandler.c
 // This File:        division.c
 // Other Files:      sendsig.c
 // Semester:         CS 354 Spring 2023
 // Instructor:       Deppeler
 //
 // Author:           Nathanael Stevenson
 // Email:            nlstevenson@wisc.edu
 // CS Login:         nathanael
 // GG#:              2
 //
 /////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
 //                   
 //////////////////////////// 80 columns wide ///////////////////////////////////

#include <stdio.h>     
#include <stdlib.h>    
#include <string.h> 
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

//init global counter
int counter = 0;

//sigfpe handler
void handler_SIGFPE(){
    printf("Error: a division by 0 operation was attempted.\n");
    printf("total number of operations completed successfully: %i\n", counter);
    printf("The program will be terminated.\n");
    exit(0);
}

//sigint handler
void handler_SIGINT(){
    printf("\ntotal number of operations completed successfully: %i\n", counter);
    printf("The program will be terminated.\n");
    exit(0);
}

int main() {

    //register handler_SIGFPE
    struct sigaction sigfpe;
    memset(&sigfpe, 0, sizeof(sigfpe));
    sigfpe.sa_handler = handler_SIGFPE;
    //check if handler_SIGFPE successful
    if(sigaction(SIGFPE, &sigfpe, NULL)!=0){
        printf("Error binding SIGFPE handler\n");
        exit(1);
    }

    //register handler_SIGINT
    struct sigaction sigint;
    memset(&sigint, 0, sizeof(sigint));
    sigint.sa_handler = handler_SIGINT;
    //check if handler_SIGINT successful
    if(sigaction(SIGINT, &sigint, NULL)!=0){
        printf("Error binding SIGINT handler\n");
        exit(1);
    }

    //init local variables
    int firstNum;
    int secondNum;
    char buffer[100];

    //run divison loop indefinitely
    while(1){
    //get user input 1
    printf("Enter first integer: ");
    if (fgets(buffer, 100, stdin)== NULL){
	    fprintf(stderr, "Error reading user input. \n");
    }
    firstNum = atoi(buffer);
    //get user input 2
    printf("Enter second integer: ");
    if (fgets(buffer, 100, stdin)== NULL){
        fprintf(stderr, "Error reading user input. \n");
    } 
    secondNum = atoi(buffer);

    //calculate divison
    int divide = firstNum / secondNum;
    int remain = firstNum % secondNum;

    //print result and increment counter
    printf("%i / %i is %i with a remainder of %i\n", firstNum, secondNum, divide, remain);
    counter++;
    }
}

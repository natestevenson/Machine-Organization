 ////////////////////////////////////////////////////////////////////////////////
 // Main File:        mySigHandler.c
 // This File:        mySigHandler.c
 // Other Files:      sendsig.c, division.c
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


#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

//global variables
int seconds = 4;
int sigusr1_tally = 0;

//sigalrm handler
void handler_SIGALRM(){
    pid_t pid = getpid();
    time_t curr;
    //check time successfully updated
    if(time(&curr) == -1){
        printf("Error getting system time");
        exit(1);
    }

    printf("PID: %d CURRENT TIME: %s", pid, ctime(&curr));
    alarm(seconds);
}

//siguser1 handler
void handler_SIGUSR1(){
    printf("SIGUSR1 handled and counted!\n");
    sigusr1_tally++;
}

//signint handler
void handler_SIGINT(){
    printf("\nSIGINT handled.\n");
    printf("SIGUSR1 was handled %d times. Exiting now.\n", sigusr1_tally);
    exit(0);
}

int main(){
    printf("PID and time print every 4 seconds.\n");
    printf("Type Ctrl-C to end the program.\n");
    //start alarm
    alarm(seconds);

    //register handler_SIGALRM
    struct sigaction sigalrm;
    memset(&sigalrm, 0, sizeof(sigalrm));
    sigalrm.sa_handler = handler_SIGALRM;
    //check if handler_SIGALRM successful
    if(sigaction(SIGALRM, &sigalrm, NULL)!=0){
        printf("Error binding SIGALRM handler\n");
         exit(1);
    }

    //register handler_SIGUSR1
    struct sigaction sigusr1;
    memset(&sigusr1, 0, sizeof(sigusr1));
    sigusr1.sa_handler = handler_SIGUSR1;
    //check if handler_SIGUSR1 successful
    if(sigaction(SIGUSR1, &sigusr1, NULL)!=0){
        printf("Error binding SIGUSR1 handler\n");
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

    while(1){};
    return 0;
}

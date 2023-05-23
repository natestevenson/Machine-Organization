////////////////////////////////////////////////////////////////////////////////
 // Main File:        mySigHandler.c
 // This File:        sendsig.c
 // Other Files:      division.c
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
#include <ctype.h>
#include <time.h>

int main(int argc, char *argv[]) {
    //if incorrect number of arguments provided, display usage
    if(argc != 3){
        printf("Usage: sendsig <signal type> <pid>\n");
        exit(0);
}    
    //get signal argument
    char *signal = *(argv+1);
    char signal_arg = *(signal+1);

    //if signal argument is not valid, display usage
    if(signal_arg != 'i' && signal_arg != 'u'){
        printf("Usage: sendsig <signal type> <pid>\n");
        exit(0);
    }

    //get pid argument
    char *pid = *(argv+2);

    //check if pid argument is valid
    int isValidNum = 1;
    char curr;
    int i = 0;
    while(*(pid+i)!='\0'){
        curr = *(pid+i);
        if(!isdigit(curr)){
            isValidNum = 0;
            break;
        }
        i++;
    }
    //if pid argument is not valid, display usage
    if(!isValidNum){
        printf("Usage: sendsig <signal type> <pid>\n");
        exit(0);
    }

    //convert pid argument to int
    int pid_arg = atoi(pid);

    //determine which signal to send
    if(signal_arg == 'i'){
        if(kill(pid_arg, SIGINT) == -1){
            printf("Error: SIGINT failed to send\n");
            exit(1);
        }else{
            //printf("sendsig -i %d\n", pid_arg);
        }
    } else{
        if(kill(pid_arg, SIGUSR1) == -1){
            printf("Error: SIGUSR1 failed to send\n");
            exit(1);
        }else{
            //printf("sendsig -u %d\n", pid_arg);
        }
    }
}

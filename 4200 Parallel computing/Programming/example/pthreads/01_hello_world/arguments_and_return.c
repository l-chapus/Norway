#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


// Shared location to store return values in
int result[2];


// Function we can make a thread out of:
//  * expects the argument to point at an array of 4 integers
//  * expects the result pointer to have space for 2 integers
//  * calculates the 2 output-integers as pairwise sum-of-squares for the input
void *
do_this ( void *arg )
{
    int *values = (int *)arg;
    printf ( "%d %d\n", values[0], values[1] );
    result[0] = values[0]*values[0] + values[1]*values[1];
    result[1] = values[2]*values[2] + values[3]*values[3];
    printf ( "%d %d\n", result[0], result[1] );
    return result;
}


// Main function: sets up arguments, spawns and joins a thread
int
main ( int argc, char **argv )
{
    // Make a handle for our extra thread
    pthread_t worker;

    // Set up our arguments and return value
    int
        *inputs = malloc ( 4*sizeof(int) ), // Passed into pthread_create
        *output = NULL;                     // Will be written by pthread_join

    // Fill in some argument values
    inputs[0] = 1;
    inputs[1] = 2;
    inputs[2] = 3;
    inputs[3] = 4;

    // Launch the thread
    pthread_create ( &worker, NULL, &do_this, inputs );
    printf ( "Started thread\n" );

    // Wait for it to complete
    pthread_join ( worker, (void **) &output );
    printf ( "Joined thread\n" );

    // Print the result
    printf ( "%d %d\n", output[0], output[1] );
    exit ( EXIT_SUCCESS );
}

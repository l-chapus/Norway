#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>


typedef int64_t int_t;


// Default number of threads, can be overwritten with command line argument
int_t n_threads = 2;


// Trivial function to make threads out of
void *
say_hi ( void *arg )
{
    // Cast my thread-id
    int_t tid = (int_t) arg;
    // Say hello
    printf ( "Hello world, I am thread %ld out of %ld\n", tid, n_threads );
    // Stop
    pthread_exit ( EXIT_SUCCESS );
}


// Main function
int
main ( int argc, char **argv )
{
    // Check if we have a request for a non-default number of threads
    if ( argc > 1 )
        n_threads = strtol ( argv[1], NULL, 10 );

    // Make space for that many threads
    pthread_t threads[n_threads];

    // Spawn all the threads, and give them their 'rank' (thread-id)
    for ( int_t i=0; i<n_threads; i++ )
        pthread_create ( &threads[i], NULL, &say_hi, (void *)i );

    // Wait for all the threads to finish
    for ( int_t t=0; t<n_threads; t++ )
        pthread_join ( threads[t], NULL );

    // Quit
    exit ( EXIT_SUCCESS );
}

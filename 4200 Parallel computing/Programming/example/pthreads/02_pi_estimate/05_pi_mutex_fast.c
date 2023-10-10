#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

#define STEPS (1e8)
#define H (1.0/STEPS)

int64_t n_threads = 2;
pthread_mutex_t lock;
double pi = 0.0;

void *
integrate ( void *in )
{
    int64_t tid = (int64_t)in;
    double x = tid*H, pi_local = 0.0;
    for ( size_t i=0; i<STEPS; i+=n_threads )
    {
        x += n_threads*H;
        pi_local += H / (1.0 + x*x);
    }
    pthread_mutex_lock ( &lock );
    pi += pi_local;
    pthread_mutex_unlock ( &lock );
    return NULL;
}

int
main ( int argc, char **argv )
{
    if ( argc > 1 )
        n_threads = strtol ( (const char *)argv[1], NULL, 10 );
    pthread_t threads[n_threads];
    pthread_mutex_init ( &lock, NULL );
    for ( int64_t t=0; t<n_threads; t++ )
        pthread_create ( &threads[t], NULL, &integrate, (void *)t );
    for ( int64_t t=0; t<n_threads; t++ )
        pthread_join ( threads[t], NULL );
    pthread_mutex_destroy ( &lock );
    pi *= 4.0;
    printf ( "Estimated %e, missed by %e\n", pi, fabs(pi-M_PI) );

    exit ( EXIT_SUCCESS );
}

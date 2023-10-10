#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>

#define ITERATIONS 10
#define STEPS (1e8)
#define H (1.0/STEPS)

int64_t n_threads = 2;
int64_t arrived = 0, departed = 0;
pthread_mutex_t lock_pi, lock_arrive, lock_depart;
pthread_cond_t cond_arrive, cond_depart;
double pi = 0.0;

void
signal_barrier ( pthread_mutex_t *lock, pthread_cond_t *cond, int64_t *count )
{
    pthread_mutex_lock ( lock );
    (*count)++;
    if ( (*count) < n_threads )
        while ( pthread_cond_wait ( cond, lock ) != 0 );
    (*count)--;
    if ( (*count) > 0 )
        pthread_cond_signal ( cond );
    pthread_mutex_unlock ( lock );
    return;
}


void *
integrate ( void *in )
{
    for ( size_t iter=0; iter<ITERATIONS; iter++ )
    {
        int64_t tid = (int64_t)in;
        double x = tid*H, pi_local = 0.0;
        for ( size_t i=0; i<STEPS; i+=n_threads )
        {
            x += n_threads*H;
            pi_local += H / (1.0 + x*x);
        }

        pthread_mutex_lock ( &lock_pi );
        pi += pi_local;
        pthread_mutex_unlock ( &lock_pi );

        signal_barrier ( &lock_arrive, &cond_arrive, &arrived );
        if ( tid == 0 )
        {
            pi *= 4.0;
            printf ( "Estimated %e, missed by %e\n", pi, fabs(pi-M_PI) );
            pi = 0.0;
        }
        signal_barrier ( &lock_depart, &cond_depart, &departed );
    }
    return NULL;
}


int
main ( int argc, char **argv )
{
    if ( argc > 1 )
        n_threads = strtol ( (const char *)argv[1], NULL, 10 );
    pthread_t threads[n_threads];
    pthread_mutex_init ( &lock_pi, NULL );
    pthread_mutex_init ( &lock_arrive, NULL );
    pthread_mutex_init ( &lock_depart, NULL );
    pthread_cond_init ( &cond_arrive, NULL );
    pthread_cond_init ( &cond_depart, NULL );
    for ( int64_t t=0; t<n_threads; t++ )
        pthread_create ( &threads[t], NULL, &integrate, (void *)t );
    for ( int64_t t=0; t<n_threads; t++ )
        pthread_join ( threads[t], NULL );
    pthread_mutex_destroy ( &lock_pi );
    pthread_mutex_destroy ( &lock_arrive );
    pthread_mutex_destroy ( &lock_depart );
    pthread_cond_destroy ( &cond_arrive );
    pthread_cond_destroy ( &cond_depart );

    exit ( EXIT_SUCCESS );
}

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h> 
#include <sys/time.h>
#define WALLTIME(t) ((double)(t).tv_sec + 1e-6 * (double)(t).tv_usec)

const int N=1600;

double
    *a = NULL,
    *b = NULL,
    *c = NULL;
#define A(i,j) a[(i)*N+(j)]
#define B(i,j) b[(i)*N+(j)]
#define C(i,j) c[(i)*N+(j)]

struct timeval
    t_start,
    t_end;

int 
main () 
{ 
    // Allocate three NxN matrices
    double
        *a = malloc ( N*N*sizeof(double) ),
        *b = malloc ( N*N*sizeof(double) ),
        *c = malloc ( N*N*sizeof(double) ); 

    // Fill them with some numbers to multiply
    for ( int i=0; i<N; i++ )
    {
        for ( int j=0; j<N; j++ )
        {
            A(i,j) = sin ( 2.0*M_PI*i/(double)N );
            B(i,j) = cos ( 2.0*M_PI*i/(double)N );
        }
    }

    // Make sure the C matrix is all zeros before we begin
    memset ( c, 0, N*N*sizeof(double) );

    gettimeofday ( &t_start, NULL );
    // Naive matrix multiply
    for ( int i=0; i<N; i++ )
        for ( int j=0; j<N; j++ )
            for ( int k=0; k<N; k++ )
                C(i,j) += A(i,k) * B(k,j);
    gettimeofday ( &t_end, NULL );

    // Save the result in a file, so we can compare other implementations
    FILE *out = fopen ( "correct.dat", "w" );
    fwrite ( c, sizeof(double), N*N, out );
    fclose ( out );

    printf ( "Spent %lf seconds in matrix multiply\n",
        WALLTIME(t_end) - WALLTIME(t_start)
    );

    free ( a );
    free ( b );
    free ( c );
    exit ( EXIT_SUCCESS );
} 

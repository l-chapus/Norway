#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h> 
#include <emmintrin.h>
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
    // Allocate three (aligned) NxN matrices
    double
        *a = _mm_malloc ( N*N*sizeof(double), 16 ),
        *b = _mm_malloc ( N*N*sizeof(double), 16 ),
        *c = _mm_malloc ( N*N*sizeof(double), 16 ); 

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
    // Vectorized matrix multiply
    for ( int i=0; i<N; i+=2 )
        for ( int j=0; j<N; j+=2 )
            for ( int k=0; k<N; k+=2 )
            {
                __m128d a00 = _mm_load_pd1 ( &A(i,k)     );
                __m128d a01 = _mm_load_pd1 ( &A(i,k+1)   );
                __m128d a10 = _mm_load_pd1 ( &A(i+1,k)   );
                __m128d a11 = _mm_load_pd1 ( &A(i+1,k+1) );
                __m128d b0x = _mm_load_pd  ( &B(k,j)     );
                __m128d b1x = _mm_load_pd  ( &B(k+1,j)   );
                __m128d c0x = _mm_load_pd  ( &C(i,j)     );
                __m128d c1x = _mm_load_pd  ( &C(i+1,j)   );

                __m128d c0x_update = _mm_add_pd ( _mm_mul_pd(a00,b0x), _mm_mul_pd(a01,b1x) );
                __m128d c1x_update = _mm_add_pd ( _mm_mul_pd(a10,b0x), _mm_mul_pd(a11,b1x) );

                c0x = _mm_add_pd ( c0x, c0x_update );
                c1x = _mm_add_pd ( c1x, c1x_update );
                _mm_store_pd ( &C(i,j),   c0x_update );
                _mm_store_pd ( &C(i+1,j), c1x_update );
            }
    gettimeofday ( &t_end, NULL );

    // Save the result in a file, so we can compare other implementations
    FILE *out = fopen ( "vectorized.dat", "w" );
    fwrite ( c, sizeof(double), N*N, out );
    fclose ( out );

    printf ( "Spent %lf seconds in matrix multiply\n",
        WALLTIME(t_end) - WALLTIME(t_start)
    );

    _mm_free ( a ); 
    _mm_free ( b ); 
    _mm_free ( c ); 
    exit ( EXIT_SUCCESS );
} 

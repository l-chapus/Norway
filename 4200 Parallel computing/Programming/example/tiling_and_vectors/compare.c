#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

const int N=1600;
const double threshold = 1e-12;

int
main ( int argc, char **argv )
{
    if ( argc<3 )
    {
        fprintf ( stderr, "I need two files to compare\n" );
        exit ( EXIT_FAILURE );
    }
    FILE *corr = fopen ( argv[1], "r" );
    FILE *tile = fopen ( argv[2], "r" );
    double maxdiff = 0.0;
    for ( int i=0; i<N; i++ )
        for ( int j=0; j<N; j++ )
        {
            double c, t;
            fread ( &c, sizeof(double), 1, corr );
            fread ( &t, sizeof(double), 1, corr );
            double diff = fabs(c-t);
            maxdiff = MAX(maxdiff,diff);
        }
    if ( maxdiff > threshold )
        printf ( "Greatest difference was %e\n", maxdiff );
    exit ( EXIT_SUCCESS );
}

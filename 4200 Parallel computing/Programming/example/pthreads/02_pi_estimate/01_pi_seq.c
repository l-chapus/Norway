#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STEPS (1e8)
#define H (1.0/STEPS)

int
main ( )
{
    double pi = 0.0, x = 0.0;
    for ( size_t i=0; i<STEPS; i++ )
    {
        x += H;
        pi += H / (1.0 + x*x);
    }
    pi *= 4.0;
    printf ( "Estimated %e, missed by %e\n", pi, fabs(pi-M_PI) );
    exit ( EXIT_SUCCESS );
}

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Using these typedefs means we can change precision for the whole simulation
//  just by altering it in one place
typedef double real_t;
typedef int64_t int_t;

int_t
    N             = 1024,       // Number of points in the simulated domain
    max_iter      = 65536,      // Maximum number of iterations
    snapshot_freq = 128;        // How often to save the simulation state

real_t
    *u[2] = { NULL, NULL },     // Two buffers for displacement (now and next)
    v,                          // Propagation speed for the wave
    dx,                         // Space between two points in our domain
    dt,                         // Time between simulation steps
    x_range[2] = { -1.0, 1.0 }; // "Physical" extent of the domain

/* Convenience macros for indexing:
 *  we want to allocate 1 additional boundary point at each end of the domain,
 *  this way we can index them as U(-1) and U(N)
 */
#define U(i)        u[0][(i)+1]
#define U_next(i)   u[1][(i)+1]


int
main ( int argc, char **argv )
{
    // Allocate 2 extra elements for the displacement arrays, so that we
    //  don't have to make exceptions for first and last points in the domain
    u[0] = malloc ( (N+2) * sizeof(real_t) );
    u[1] = malloc ( (N+2) * sizeof(real_t) );

    // Work out the simulation parameters
    dx = (x_range[1]-x_range[0]) / (real_t) N;
    v = 1.0;
    dt = dx/(4.0*v);    // Set timestep for suitably conservative Courant-number

    // Initial state: start with a big disturbance at one end of the domain
    for ( int_t i=0; i<N; i++ )
    {
        real_t x = i * dx;
        U(i) = pow(exp(-pow(x,2.0)),4.0);
    }

    // Time integration loop
    for ( int_t iter=0; iter<max_iter; iter++ )
    {
        // Periodic boundary condition
        U(-1) = U(N-1);
        U(N)  = U(0);

        // Solve for next time step, using Lax-Friedrichs approximation
        for ( int_t i=0; i<N; i++ )
        {
            U_next(i) =   0.5 * ( U(i-1) + U(i+1) )
                        - dt * v * (U(i+1) - U(i-1)) / (2.0 * dx);
        }

        // Save a picture of the state every so often
        if ( (iter % snapshot_freq) == 0 )
        {
            int_t step = iter / snapshot_freq;
            char filename[256];
            memset ( filename, 0, 256*sizeof(char) );
            sprintf ( filename, "data/%.5ld.dat", step );
            FILE *out = fopen ( filename, "w" );
            fwrite ( &U(0), sizeof(real_t), N, out );
            fclose ( out );
        }

        // Swap the now-step with the next-step and continue
        real_t *tmp = u[0];
        u[0] = u[1];
        u[1] = tmp;
    }

    // Clean up and go home
    free ( u[0] );
    free ( u[1] );
    exit ( EXIT_SUCCESS );
}

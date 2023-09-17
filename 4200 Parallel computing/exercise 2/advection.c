#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Typedefs for floats and integers
typedef double real_t;
typedef int64_t int_t;

int_t
    N             = 512,        // Number of points in each direction
    max_iter      = 131027,     // Maximum number of iterations
    snapshot_freq = 256;        // How often to save the simulation state

real_t
    *u[2] = { NULL, NULL },     // Two buffers for displacement (now and next)
    vy,                         // Propagation speeds for the wave,
    vx,                         //  in y and x directions
    dx,                         // Space between two points in our domain
    dt,                         // Time between simulation steps
    x_range[2] = { -1.0, 1.0 }; // "Physical" extent of the domain
    *global_domain = NULL;      // Only used by rank 0 to collect whole domain

int
    rank,
    size,
    left_neighbor,
    right_neighbor;
int_t
    *local_sizes = NULL;

/* Convenience macros for indexing:
 *  we want to allocate 1 additional boundary point beyond the domain,
 *  in 2D it becomes a whole additional row/column at each end
 */
#define U(i,j)      u[0][((i)+1)*(N+2)+(j)+1]
#define U_next(i,j) u[1][((i)+1)*(N+2)+(j)+1]


int
main ( int argc, char **argv )
{
    domain_init();          // allocate memory and setup all things
    
    // Time integration loop
    for ( int_t iter=0; iter<max_iter; iter++ )
    {

        // Periodic boundary condition
        for ( int_t i=0; i<N; i++ )
        {
            U(i,-1) = U(i,N-1);
            U(i,N)  = U(i,0);
        }
        for ( int_t j=0; j<N; j++ )
        {
            U(-1,j) = U(N-1,j);
            U(N,j)  = U(0,j);
        }

        // Solve for next time step, using Lax-Friedrichs approximation
        for ( int_t i=0; i<N; i++ )
        {
            for ( int_t j=0; j<N; j++ )
            {
                
                U_next(i,j) = 0.25 * (U(i-1,j) + U(i+1,j) + U(i,j-1) + U(i,j+1)) - vy * (dt/(2.0*dx)) * (U(i+1,j) - U(i-1,j)) 
                            - vx * (dt/(2.0*dx)) * (U(i,j+1) - U(i,j-1));
                
               //U_next(i,j) = U(i,j) - vy * (dt/(2.0*dx)) * (U(i-1,j) - U(i+1,j)) - vx * (dt/(2.0*dx)) * U(i,j+1);
               
               //U_next(i,j) = U(i,j) - 390 * (dt/(2.0*dx)) * ( U(i-1,j) - U(i+1,j) + 4 * U(i,j)  - U(i,j+1));
            }
        }

        if ( (iter % snapshot_freq) == 0 )
        {
            int_t step = iter / snapshot_freq;
            char filename[256];
            memset ( filename, 0, 256*sizeof(char) );
            sprintf ( filename, "data/%.5ld.dat", step );
            FILE *out = fopen ( filename, "w" );
            for ( int_t i=0; i<N; i++ )
                fwrite ( &U(i,0), sizeof(real_t), N, out );
            fclose ( out );
            printf ( "Saved state at iteration %ld\n", iter );
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

void
domain_init ( void )
{
    // Allocate 2 extra elements for the displacement arrays, so that we
    //  don't have to make exceptions for first and last points in the domain
    u[0] = malloc ( (N+2) * (N+2) * sizeof(real_t) );
    u[1] = malloc ( (N+2) * (N+2) * sizeof(real_t) );

    // Work out the simulation parameters
    dx = (x_range[1]-x_range[0]) / (real_t) N;
    vx = 1.0;
    vy = 0.0;
    dt = dx/(4.0*sqrt(vy*vy+vx*vx));    // Careful time step value

    // Initial state: start with a big disturbance at one end of the domain
    for ( int_t i=0; i<N; i++ )
    {
        for ( int_t j=0; j<N; j++ )
        {
            real_t
                y = x_range[0] + i * dx,
                x = j * dx;
            real_t
                norm = sqrt ( x*x + y*y );
            U(i,j) = cos(norm);
        }
    }

}
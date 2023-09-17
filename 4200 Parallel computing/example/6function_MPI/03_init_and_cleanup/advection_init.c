#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

// Using these typedefs means we can change precision for the whole simulation
//  just by altering it in one place
typedef double real_t;
typedef int64_t int_t;

int_t
    N             = 8192,        // Number of points in the simulated domain
    max_iter      = 524288,      // Maximum number of iterations
    snapshot_freq = 1024;        // How often to save the simulation state

real_t
    *u[2] = { NULL, NULL },     // Two buffers for displacement (now and next)
    v,                          // Propagation speed for the wave
    dx,                         // Space between two points in our domain
    dt,                         // Time between simulation steps
    x_range[2] = { -1.0, 1.0 }; // "Physical" extent of the domain

int
    rank,
    size;
int_t
    *local_sizes = NULL;


/* Convenience macros for indexing:
 *  we want to allocate 1 additional boundary point at each end of the domain,
 *  this way we can index them as U(-1) and U(N)
 */
#define U(i)        u[0][(i)+1]
#define U_next(i)   u[1][(i)+1]


void domain_init ( void );
void domain_save ( int_t step );
void domain_finalize ( void );


int
main ( int argc, char **argv )
{
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );
    domain_init();      // Allocate arrays and set up starting state
    domain_save ( 0 );  // Save the starting state so that we can see it's ok
    domain_finalize();  // Discard our memory allocations
    MPI_Finalize ();
    exit ( EXIT_SUCCESS );
}


void
domain_init ( void )
{
    // If the number of ranks doesn't evenly divide the domain, give
    // 1 additional point to the first N%size ranks
    local_sizes = malloc ( size * sizeof(int_t) );
    for ( int_t r=0; r<size; r++ )
        local_sizes[r] = (int_t)( N / size ) + ((r<(N%size)) ? 1 : 0);

    // Allocate with 2 extra elements (left/right borders) for each sub-domain
    u[0] = malloc ( (local_sizes[rank]+2) * sizeof(real_t) );
    u[1] = malloc ( (local_sizes[rank]+2) * sizeof(real_t) );
    
    // Work out the simulation parameters
    dx = (x_range[1]-x_range[0]) / (real_t) N;
    v = 1.0;
    dt = dx/(4.0*v);    // Set timestep for suitably conservative Courant-number

    // Find the global index of my own first point
    int_t my_origin = 0;
    for ( int_t i=0; i<rank; i++ )
        my_origin += local_sizes[i];

    // Initial state: start with a big disturbance at one end of the domain
    for ( int_t i=0; i<local_sizes[rank]; i++ )
    {
        real_t x = (my_origin + i) * dx;
        U(i) = pow(exp(-pow(x,2.0)),4.0);
    }
}


void
domain_save ( int_t step )
{
    if ( rank == 0 )
    {
        // Create the file name and open the file
        char filename[256];
        memset ( filename, 0, 256*sizeof(char) );
        sprintf ( filename, "data/%.5ld.dat", step );
        FILE *out = fopen ( filename, "w" );

        real_t *domain_buffer = malloc ( local_sizes[0] * sizeof(real_t) );
        fwrite ( &U(0), sizeof(real_t), local_sizes[0], out );

        int_t origin = local_sizes[0];
        for ( int_t r=1; r<size; r++ )
        {
            MPI_Recv ( 
                domain_buffer, local_sizes[r], MPI_DOUBLE,
                r, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
            fwrite ( domain_buffer, sizeof(real_t), local_sizes[r], out );
            origin += local_sizes[r];
        }

        // Write the current state into the file
        free ( domain_buffer );
        fclose ( out );
    }
    else
    {
        MPI_Send (
            &(U(0)), local_sizes[rank], MPI_DOUBLE, 0, 0, MPI_COMM_WORLD
        );
    }
}


void
domain_finalize ( void )
{
    // Clean up
    free ( u[0] );
    free ( u[1] );
    free ( local_sizes );
}

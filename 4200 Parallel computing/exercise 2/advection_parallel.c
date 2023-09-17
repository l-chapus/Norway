#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

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
    
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    domain_init();          // allocate memory and setup all things
    
    // Time integration loop
    for ( int_t iter=0; iter<max_iter; iter++ )
    {
        //border_exchange();
        
        
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
        

        if ( (iter % snapshot_freq) == 0 )
        {
            int_t step = iter / snapshot_freq;
            time_step();
            domain_save(step);

            printf ( "Saved state at iteration %ld\n", iter );
            
        }

        // Swap the now-step with the next-step and continue
        real_t *tmp = u[0];
        u[0] = u[1];
        u[1] = tmp;
    }

    // Clean up and go home
    domain_finalize();  

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

    // Work out who our neighboring ranks are
    left_neighbor = ( rank + size - 1 ) % size;
    right_neighbor = ( rank + size + 1 ) % size;

    if ( rank == 0 )
        global_domain = malloc ( N * sizeof(real_t) );
    
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

void
domain_finalize ( void )
{
    // Clean up
    free ( u[0] );
    free ( u[1] );
    free ( local_sizes );
    free ( global_domain );
}

void
time_step ( void )
{
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
        for ( int_t i=0; i<N; i++ )
            fwrite ( &U(i,0), sizeof(real_t), N, out );

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
            &(U(0,1)), local_sizes[rank], MPI_DOUBLE, 0, 0, MPI_COMM_WORLD );
        
    }
}
void
border_exchange ( void )
{
    
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

    /*
    // Send my leftmost element to my neighbor on the left
    MPI_Send (
        &(U(0,1)), 1, MPI_DOUBLE, left_neighbor, 0, MPI_COMM_WORLD
    );
    // Receive my right border element from my right neighbor's leftmost
    MPI_Recv (
        &(U(local_sizes[rank],1)), 1, MPI_DOUBLE,
        right_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE
    );
    // Send my rightmost element to my neighbor on the right
    MPI_Send (
        &(U(local_sizes[rank]-1,1)), 1, MPI_DOUBLE, right_neighbor, 0, MPI_COMM_WORLD
    );
    // Receive my left border element from my left neighbor's rightmost
    MPI_Recv (
        &(U(-1,1)), 1, MPI_DOUBLE,
        left_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE
    );
    */
}


/*
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
*/

/*
char filename[256];
memset ( filename, 0, 256*sizeof(char) );
sprintf ( filename, "data/%.5ld.dat", step );
FILE *out = fopen ( filename, "w" );

fclose ( out );


int rank , sendData [N] , receiveData [N] ;
MPI_Comm_rank ( MPI_COMM_WORLD, &rank ) ;
MPI_Sendrecv ( sendData , N, MPI_INT , 1 , 0 , receiveData , N,
MPI_INT , 0 , 0 , MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
MPI_Sendrecv ( sendData , N, MPI_INT , 0 , 0 , receiveData , N,
MPI_INT , 1 , 0 , MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
*/
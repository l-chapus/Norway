#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>

typedef int64_t int_t;
typedef double real_t;


int
main ( int argc, char **argv )
{
    // Number of rectangles to use for the estimate, MPI rank and comm size
    int_t N = 2097152;
    int rank, size;

    // Set up MPI
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );

    // Create an array with space for all the partial results
    real_t result_part = 0.0;

    // Find my interval
    real_t h = 1.0 / (real_t) N;
    int_t chunk = (int_t)(N/size) + ( rank < (N%size) ? 1:0 );
    int_t bounds[2] = { rank*chunk, (rank+1)*chunk };

    // Estimate the area under the curve
    for ( int_t i=bounds[0]; i<bounds[1]; i++ )
    {
        real_t x = (i+1) * h;
        result_part += h / (1+x*x);
    }

    // Share all the results
    real_t total;
    MPI_Allreduce (
        &result_part, &total, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD
    );

    printf ( "Rank %d: Pi =~ %lf\n", rank, 4.0 * total );

    MPI_Finalize ();
}

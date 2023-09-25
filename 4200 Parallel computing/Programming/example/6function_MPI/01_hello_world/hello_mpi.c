#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int
main ( int argc, char **argv )
{
    int size, rank;
    MPI_Init ( &argc, &argv );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );
    printf ( "Hello world, I am rank %d out of %d\n", rank, size );
    MPI_Finalize ();
    exit ( EXIT_SUCCESS );
}

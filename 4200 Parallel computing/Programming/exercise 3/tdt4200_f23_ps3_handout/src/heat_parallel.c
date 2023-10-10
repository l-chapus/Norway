#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#include "../inc/argument_utils.h"

// Convert 'struct timeval' into seconds in double prec. floating point
#define WALLTIME(t) ((double)(t).tv_sec + 1e-6 * (double)(t).tv_usec)

#define MPI_RANK_ROOT  ( rank == 0 )
#define MPI_RANK_FIRST ( MPI_RANK_ROOT )
#define MPI_RANK_LAST  ( rank == size-1 )

typedef int64_t int_t;
typedef double real_t;

int_t
    M,
    N,
    max_iteration,
    snapshot_frequency;

real_t
    *temp[2] = { NULL, NULL },
    *thermal_diffusivity,
    dt,
    *canvas;

    // MPI parameters
int
    size,
    rank,
    n_dim = 2,              // because we work in 2D
    dim_size[2],
    size_subgrid,
    offset_x,
    offset_y,
    coord[2],
    local_N_start,
    local_N_end,
    local_M_start,
    local_M_end;
    

MPI_Comm commCart;

#define T(x,y)                      temp[0][(y) * (N + 2) + (x)]
#define T_next(x,y)                 temp[1][((y) * (N + 2) + (x))]
#define THERMAL_DIFFUSIVITY(x,y)    thermal_diffusivity[(y) * (N + 2) + (x)]


void time_step ( void );
void boundary_condition( void );
void border_exchange( void );
void domain_init ( void );
void domain_save ( int_t iteration );
void domain_finalize ( void );
void border_exchange( void );


void
swap ( real_t** m1, real_t** m2 )
{
    real_t* tmp;
    tmp = *m1;
    *m1 = *m2;
    *m2 = tmp;
}


int
main ( int argc, char **argv )
{
    // TODO 1:
    // - Initialize and finalize MPI.
    // - Create a cartesian communicator.
    // - Parse arguments in the rank 0 processes 
    //   and broadcast to other processes
    
    
    // Start MPI, as per usual
    MPI_Init ( &argc, &argv );
    MPI_Comm_size ( MPI_COMM_WORLD, &size );
    MPI_Comm_rank ( MPI_COMM_WORLD, &rank );

    // determination of the lengh of each dimension by using the prime decomposition of the size and take the 2 numbers in the middle

    int *prime_number;
    int lengh = 0;
    prime_number = (int *)malloc(lengh * sizeof(int));      // make a dynamic array
    
    for(int k=1; k<= size; k++){
        if(size%k == 0){                                    
            lengh++;
            prime_number = (int *)realloc(prime_number, lengh * sizeof(int)); // increase le lengh of the array
            prime_number[lengh-1] = k;                                        // add the value
        }
        
    }
    if(lengh%2 == 0){                                   // if the lengh is even
        dim_size[0] = prime_number[(lengh/2) - 1];      // we take the two number close
        dim_size[1] = prime_number[lengh/2];            // to the center
    }
    else{                                               // else the lengh is odd
        dim_size[0] = prime_number[lengh/2];            // we take the element in the center
        dim_size[1] = prime_number[lengh/2];
    }

    free(prime_number);

    int reorder = 0;
    int periods[n_dim];
    int my_cart_rank, direction, displacement, rank_source, rank_destination;
    periods[0] = 0;
    periods[1] = 0;
    
    MPI_Cart_create ( MPI_COMM_WORLD, n_dim, dim_size, periods, reorder, &commCart);

    MPI_Dims_create(size, n_dim, dim_size);
    MPI_Cart_coords(commCart, rank, n_dim, coord);
    MPI_Cart_rank(commCart, coord, &my_cart_rank);

    if ( MPI_RANK_ROOT )    // parse the argument in the rank 0
    {
        OPTIONS *options = parse_args( argc, argv );
        if ( !options )
        {
            fprintf( stderr, "Argument parsing failed\n" );
            exit(1);
        }

        N = options->N;
        M = options->M;
        max_iteration = options->max_iteration;
        snapshot_frequency = options->snapshot_frequency;
    }

    // Broadcast all the argument needed
    MPI_Bcast ( &N, 1, MPI_INT64_T, 0, MPI_COMM_WORLD );    
    MPI_Bcast ( &M, 1, MPI_INT64_T, 0, MPI_COMM_WORLD );
    MPI_Bcast ( &max_iteration, 1, MPI_INT64_T, 0, MPI_COMM_WORLD );
    MPI_Bcast ( &snapshot_frequency, 1, MPI_INT64_T, 0, MPI_COMM_WORLD );

    domain_init();

    struct timeval t_start, t_end;
    gettimeofday ( &t_start, NULL );

    for ( int_t iteration = 0; iteration <= max_iteration; iteration++ )
    {
        // TODO 6: Implement border exchange.
        // Hint: Creating MPI datatypes for rows and columns might be useful.
        
        border_exchange();

        //boundary_condition();

        time_step();

        if ( iteration % snapshot_frequency == 0 )
        {
            if (rank == 0){
                printf (
                    "Iteration %ld of %ld (%.2lf%% complete)\n",
                    iteration,
                    max_iteration,
                    100.0 * (real_t) iteration / (real_t) max_iteration
                );
            }
            domain_save ( iteration );

        }

        swap( &temp[0], &temp[1] );
    }

    gettimeofday ( &t_end, NULL );
    printf ( "Total elapsed time: %lf seconds\n",
            WALLTIME(t_end) - WALLTIME(t_start)
            );


    domain_finalize();

    MPI_Comm_free( &commCart );
    MPI_Finalize();

    exit ( EXIT_SUCCESS );
}


void
time_step ( void )
{
    real_t c, t, b, l, r, K, new_value;

    // TODO 3: Update the area of iteration so that each
    // process only iterates over its own subgrid.

    // we just have to change the end of the iterator
    for ( int_t x = 1; x <= offset_x; x++ )
    {
        for ( int_t y = 1; y <= offset_y; y++ )
        {
            c = T(x, y);

            t = T(x - 1, y);
            b = T(x + 1, y);
            l = T(x, y - 1);
            r = T(x, y + 1);
            K = THERMAL_DIFFUSIVITY(x, y);

            new_value = c + K * dt * ((l - 2 * c + r) + (b - 2 * c + t));

            T_next(x, y) = new_value;
        }
    }
}

void
border_exchange ( void )
{
    int my_cart_rank, direction, displacement, rank_source, rank_destination;

    // for each process we get the neighbour (it's -2 if there is no neighbour)

    // LEFT - RIGHT
    direction = 1;
    displacement = 1;

    MPI_Cart_shift (commCart, direction, displacement, &rank_source, &rank_destination);

    MPI_Sendrecv(temp[0] + local_M_start,
                 offset_y,
                 MPI_DOUBLE,
                 rank_source,
                 0,
                 temp[0] + local_M_start * (local_N_start + 1),
                 offset_x,
                 MPI_DOUBLE,
                 rank_destination,
                 0,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

    MPI_Sendrecv(temp[0] + local_M_start * local_N_start,
                 offset_x,
                 MPI_DOUBLE,
                 rank_destination,
                 1,
                 temp[0],
                 offset_y,
                 MPI_DOUBLE,
                 rank_source,
                 1,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);


    // TOP - BOTTOM
    direction = 0;
    displacement = 1;

    MPI_Cart_shift (commCart, direction, displacement, &rank_source, &rank_destination);

    MPI_Sendrecv(temp[0] + local_M_start,
                 offset_y,
                 MPI_DOUBLE,
                 rank_source,
                 0,
                 temp[0] + local_M_start * (local_N_start + 1),
                 offset_x,
                 MPI_DOUBLE,
                 rank_destination,
                 0,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

    MPI_Sendrecv(temp[0] + local_M_start * local_N_start,
                 offset_x,
                 MPI_DOUBLE,
                 rank_destination,
                 1,
                 temp[0],
                 offset_y,
                 MPI_DOUBLE,
                 rank_source,
                 1,
                 MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);


    
}

void
boundary_condition ( void )
{
    // TODO 4: Change the application of boundary conditions
    // to match the cartesian topology. 

    for ( int_t x = 1; x <= offset_x-1; x++ )
    {
        T(x, 0) = T(x, 2);
        T(x, offset_y + 1) = T(x, offset_y - 1);
    }
    for ( int_t y =1; y <= offset_y-1 ; y++ )
    {
        T(0, y) = T(2, y);
        T(offset_x + 1, y) = T(offset_x - 1, y);
    }
}


void
domain_init ( void )
{
    // TODO 2:
    // - Find the number of columns and rows in each process' subgrid.
    // - Allocate memory for each process' subgrid.
    // - Find each process' offset to calculate the correct initial values.
    // Hint: you can get useful information from the cartesian communicator.
    // Note: you are allowed to assume that the grid size is divisible by
    // the number of processes.

    
    offset_x = N/dim_size[1];
    offset_y = M/dim_size[0];

    local_N_start = offset_x*coord[1]+1;
    local_N_end = offset_x*(coord[1]+1);
    local_M_start = offset_y*coord[0]+1;
    local_M_end = offset_y*(coord[0]+1);
    
    printf("RANK %d   From Y %d to %d AND From X %d to %d",rank,local_M_start,local_M_end,local_N_start, local_N_end);
    
    canvas = malloc ( (offset_y+2)*(offset_x+2) * sizeof(real_t) ); // this is don't use but when I remove it, I cannot run the programm

    temp[0] = malloc ( (offset_y+2)*(offset_x+2) * sizeof(real_t) );
    temp[1] = malloc ( (offset_y+2)*(offset_x+2) * sizeof(real_t) );
    thermal_diffusivity = malloc ( (offset_y+2)*(offset_x+2) * sizeof(real_t) );

    dt = 0.1;

    for ( int_t x = 1; x <= offset_x; x++ )
    {
        for ( int_t y = 1; y <= offset_y; y++ )
        {
            real_t temperature = 30 + 30 * sin((x + y) / 20.0);
            real_t diffusivity = 0.05 + (30 + 30 * sin((offset_x - x + y) / 20.0)) / 605.0;

            T(x,y) = temperature;
            T_next(x,y) = temperature;
            THERMAL_DIFFUSIVITY(x,y) = diffusivity;
        }
    }
}


void domain_save( int_t iteration ) {
    int_t index = iteration / snapshot_frequency;
    char filename[256];
    memset ( filename, 0, 256*sizeof(char) );
    sprintf ( filename, "data/%.5ld.bin", index );

    MPI_File out;
    MPI_File_open (             // open the file whith the correct name
        MPI_COMM_WORLD,
        filename,
        MPI_MODE_CREATE | MPI_MODE_WRONLY,
        MPI_INFO_NULL,
        &out
    );

    if ( ! out ) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    }

    int_t load_size = offset_y*offset_x;

    MPI_File_write_ordered (    // add each result in the data file in order
        out,
        temp[0],
        load_size,
        MPI_DOUBLE,
        MPI_STATUS_IGNORE
    );

    MPI_File_close ( &out );
}

void
domain_finalize ( void )
{
    free ( canvas );
    free ( temp[0] );
    free ( temp[1] );
    free ( thermal_diffusivity );
}
#include <iostream>
#include <fstream>
#include <chrono>

/*
 * Sort-of inplace Median-Of-Three quicksort test
 */

int64_t
    count (1<<24);


void
mot_qsort ( int64_t *buffer, int64_t lo, int64_t hi )
{
    int64_t n = hi-lo+1;
    if ( n<2 )
        return;
    else if ( n==2 )
    {
        if ( buffer[lo] > buffer[hi] )
        {
            int64_t tmp = buffer[lo];
            buffer[lo] = buffer[hi];
            buffer[hi] = tmp;
        }
        return;
    }

    // Hoare partitioning scheme 
    int64_t
        idx_lower = lo-1,
        idx_upper = hi+1;

    int64_t
        max (std::max(std::max(buffer[lo],buffer[lo+n/2]),buffer[hi])),
        min (std::min(std::min(buffer[lo],buffer[lo+n/2]),buffer[hi])),
        sum (buffer[lo]+buffer[lo+n/2]+buffer[hi]),
        pivot, split;
    pivot = sum - min - max;

    while (true) {
        // Find leftmost element greater than
        // or equal to pivot
        do {
            idx_lower++;
        } while (buffer[idx_lower] < pivot);
 
        // Find rightmost element smaller than
        // or equal to pivot
        do {
            idx_upper--;
        } while (buffer[idx_upper] > pivot);
 
        // If two pointers met.
        if (idx_lower >= idx_upper)
        {
            split = idx_upper;
            break;
        }
 
        int64_t swap = buffer[idx_lower];
        buffer[idx_lower] = buffer[idx_upper];
        buffer[idx_upper] = swap;
    }

    mot_qsort ( buffer, lo, split );
    mot_qsort ( buffer, split+1, hi );
    return;
}


int
main ( int argc, char **argv )
{
    /* Parse the size of the number array, if specified */
    if ( argc > 1 )
        count = std::stoll ( argv[1] );

    /* Read the input */
    int64_t *buffer = new int64_t[count];
    std::ifstream in;
    in.open ( "numbers.dat", std::ios::in );
    if ( !in.is_open() )
    {
        std::cerr << "Could not find the input file 'numbers.dat'" << std::endl;
        std::cerr << " (have you run 'generate_numbers'?)" << std::endl;
        exit ( EXIT_FAILURE );
    }
    in.read ( (char *)buffer, count*sizeof(int64_t) );
    in.close ();

    std::chrono::high_resolution_clock::time_point t_start =
        std::chrono::high_resolution_clock::now();

    /* Sort it */
    mot_qsort ( buffer, 0, count-1 );

    std::chrono::high_resolution_clock::time_point t_end =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed =
        std::chrono::duration_cast<std::chrono::duration<double>>(
            t_end - t_start
        );

    std::cout << "Sorted in " << elapsed.count() << " seconds" << std::endl;

    /* Write result to file */
    std::ofstream out;
    out.open ( "sorted.dat", std::ios::out );
    out.write ( (const char *)buffer, count*sizeof(int64_t) );
    out.close ();

    /* Clean up and quit */
    delete[] buffer;
    exit ( EXIT_SUCCESS );
}

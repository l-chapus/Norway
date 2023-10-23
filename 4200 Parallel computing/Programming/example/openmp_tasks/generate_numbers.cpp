#include <fstream>
#include <random>


int64_t
    count (1<<24);


int
main ( int argc, char **argv )
{
    /* Parse the size of the number array, if specified */
    if ( argc > 1 )
        count = std::stoll ( argv[1] );

    std::random_device rd;      /* Seed */
    std::mt19937 gen(rd());     /* Mersenne-Twister with seed */

    /* Uniform distribution between 0 and maximal int64_t value */
    std::uniform_int_distribution<int64_t> dist (
        0, std::numeric_limits<int64_t>::max()
    );

    /* Write an appropriately large bunch of random numers in a file */
    std::ofstream out;
    out.open ( "numbers.dat", std::ios::out );
    for ( int64_t i=0; i<count; i++ )
    {
        int64_t next = dist(gen);
        out.write ( (const char *)&next, sizeof(int64_t) );
    }
    out.close ();

    exit ( EXIT_SUCCESS );
}

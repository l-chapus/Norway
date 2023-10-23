#include <iostream>
#include <fstream>

int
main ( int argc, char **argv )
{
    if ( argc < 2 )
    {
        std::cerr << "No input file provided" << std::endl;
        exit ( EXIT_FAILURE );
    }

    int64_t prev, next;
    std::ifstream in;
    in.open ( argv[1], std::ios::in );
    in.read ( (char *)&prev, sizeof(int64_t) );
    do {
        in.read ( (char *)&next, sizeof(int64_t) );
        if ( ! in.eof() )
        {
            if ( next < prev )
            {
                std::cerr <<
                    argv[1] << " is NOT sorted in ascending order" <<
                    std::endl;
                exit ( EXIT_FAILURE );
            }
            prev = next;
        }
    } while ( ! in.eof() );
    std::cerr <<
        argv[1] << " IS sorted in ascending order" <<
        std::endl;
    exit ( EXIT_SUCCESS );
}

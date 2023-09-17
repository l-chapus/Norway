#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// Choice of types for integers and floating-point numbers
typedef int64_t int_t;
typedef double real_t;

// Make some symbolic names for image color channels
enum { RED=0, GREEN=1, BLUE=2};

// Dimensions of the image
int_t width, height, maxval;

// We'll convert the color values into floating point format in these buffers
real_t *image[3] = {NULL, NULL, NULL};

// Macros that make it easier to index the memory buffers in 2D
#define R(i,j) image[RED][  (j)*height+(i)]
#define G(i,j) image[GREEN][(j)*height+(i)]
#define B(i,j) image[BLUE][ (j)*height+(i)]


// Here we go:
int
main ()
{
    // Open the input image file for reading
    FILE *input = fopen ( "earth.ppm", "r" );
    // Read the image dimensions
    fscanf ( input, "P6 %ld %ld %ld\n", &width, &height, &maxval );
    // Allocate our 3 buffers for the 3 colors
    image[0] = malloc ( width*height*sizeof(real_t) );
    image[1] = malloc ( width*height*sizeof(real_t) );
    image[2] = malloc ( width*height*sizeof(real_t) );

    // For each pixel, read the actual image data
    for ( int_t i=0; i<height; i++ )
    {
        for ( int_t j=0; j<width; j++ )
        {
            // Red, green, and blue are interleaved, so we read in groups of 3
            uint8_t rgb[3];
            fread ( rgb, sizeof(uint8_t), 3, input );
            // Convert the integer data to floating point, normalize to 1.0
            R(i,j) = rgb[RED] / (real_t)maxval;
            G(i,j) = rgb[GREEN] / (real_t)maxval;
            B(i,j) = rgb[BLUE] / (real_t)maxval;
        }
    }
    // We're done with the input
    fclose ( input );

    // Repeat the blur filter 20 times, so the effect will be obvious
    for ( int_t iter=0; iter<20; iter++ )
    {
        // For each pixel (except the borders)...
        for ( int_t i=1; i<height-1; i++ )
        {
            for ( int_t j=1; j<width-1; j++ )
            {
                // ...each new value is a weighted average of its neighbors
                R(i,j) = 1/16.0 * (
                    1*R(i-1,j-1) + 2*R(i-1,j) + 1*R(i-1,j+1) +
                    2*R(i,j-1)   + 4*R(i,j)   + 2*R(i,j+1) +
                    1*R(i+1,j-1) + 2*R(i+1,j) + 1*R(i+1,j+1)
                );
                G(i,j) = 1/16.0 * (
                    1*G(i-1,j-1) + 2*G(i-1,j) + 1*G(i-1,j+1) +
                    2*G(i,j-1)   + 4*G(i,j)   + 2*G(i,j+1) +
                    1*G(i+1,j-1) + 2*G(i+1,j) + 1*G(i+1,j+1)
                );
                B(i,j) = 1/16.0 * (
                    1*B(i-1,j-1) + 2*B(i-1,j) + 1*B(i-1,j+1) +
                    2*B(i,j-1)   + 4*B(i,j)   + 2*B(i,j+1) +
                    1*B(i+1,j-1) + 2*B(i+1,j) + 1*B(i+1,j+1)
                );
            }
        }
    }

    // Save the image, now that it's nice and blurred
    FILE *output = fopen ( "modified.ppm", "w" );
    // Print the image dimensions into the file
    fprintf ( output, "P6 %ld %ld %ld\n", width, height, maxval );

    // For each pixel, convert to integer in range [0,maxval] and write it
    for ( int_t i=0; i<height; i++ )
    {
        for ( int_t j=0; j<width; j++ )
        {
            uint8_t rgb[3];
            rgb[RED] = R(i,j) * (real_t)maxval;
            rgb[GREEN] = G(i,j) * (real_t)maxval;
            rgb[BLUE] = B(i,j) * (real_t)maxval;
            fwrite( rgb, sizeof(uint8_t), 3, output );
        }
    }
    // We're done with the output file
    fclose ( output );

    // Liberate the memory we allocated, and quit
    free ( image[0] );
    free ( image[1] );
    free ( image[2] );
    exit ( EXIT_SUCCESS );
}

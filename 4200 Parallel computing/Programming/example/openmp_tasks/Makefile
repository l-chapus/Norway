CXXFLAGS+= -O2 -fopenmp
TARGETS=check_sorted generate_numbers mot_quicksort mot_quicksort_openmp
.PHONY: all clean
all: ${TARGETS}
clean:
	-rm -f ${TARGETS} numbers.dat sorted.dat

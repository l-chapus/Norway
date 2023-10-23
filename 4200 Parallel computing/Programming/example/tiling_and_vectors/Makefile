CFLAGS+= -O2
LDLIBS+= -lm
TARGETS=dgemm_naive dgemm_tiled dgemm_vectorized compare
.PHONY: all clean
all: ${TARGETS}
clean:
	-rm -f ${TARGETS} *.dat

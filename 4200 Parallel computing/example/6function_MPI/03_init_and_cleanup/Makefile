CC=mpicc
CFLAGS+= -std=c99 -O2
LDLIBS+= -lm
TARGETS=advection_init
.PHONY: all dirs clean
all: dirs ${TARGETS}
dirs:
	mkdir -p data images
clean:
	-rm -f ${TARGETS} data/*.dat images/*.png

CFLAGS=-std=c99 -pthread -g -O2
TARGETS= \
    01_pi_seq               \
    02_pi_nolock            \
    03_pi_diy_lock          \
    03_pi_diy_deadlock      \
    04_pi_mutex             \
    05_pi_mutex_fast        \
    06_pi_cond_signal       \
    07_pi_cond_broadcast    \
    08_pi_barrier

all: $(TARGETS)
03_pi_diy_lock: 03_pi_diy_lock.c
	cc -std=c99 -pthread -g -o 03_pi_diy_lock 03_pi_diy_lock.c
03_pi_diy_deadlock: 03_pi_diy_lock.c
	cc ${CFLAGS} -o 03_pi_diy_deadlock 03_pi_diy_lock.c
clean:
	-rm -f $(TARGETS)

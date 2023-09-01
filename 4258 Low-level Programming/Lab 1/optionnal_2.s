.global _start

_start:
	// Here your execution starts
	ldr r0, =input

_exit:
	// Branch here for exit
	b .
	
.data
.align
	input: .asciz "Hello World"

.end
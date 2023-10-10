.global _start

_start:
	// Here your execution starts
	mov r1,#0
    mov r2,#1
    mov r3,#0
    
    cmp r2, #2           // Compare r2 avec 2
    bge not_satisfied    // Sauter à "not_satisfied" si r2 >= 2
    cmp r3, #0           // Compare r3 avec 0
    bne not_satisfied 

	b _exit

not_satisfied:
    add r2, r2, #1
    b check_input


_exit:
	// Branch to itelf
	b .
	
.data
.align
	// This section is evaluated before execution to put things into
	// memory that are required for the execution of your application
.end
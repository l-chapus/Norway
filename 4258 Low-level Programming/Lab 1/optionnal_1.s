.global _start

_start:
	// Here your execution starts
    ldr r0, =0xFF201000         // set the address for the UART
    mov r1, #0x20				// code for a space
    mov r2, #0x31               // unit
    mov r3, #0x30               // decade
	
    b loop_decade

loop_decade:
    bl loop_unit

    mov r2, #0x30               // reset the unit
    add r3, r3, #1 

    cmp r3, #0x3a
    beq _exit
    
    b loop_decade    

loop_unit:
    cmp r2, #0x3a
    bne .+8
    bx lr

    str r3, [r0]
    str r2, [r0]				// print the character on the screen
    add r2, r2, #1              // increase r1 by 1 

    str r1, [r0]                // add a space between each number

    b loop_unit


_exit:
	// Branch to itelf
	b .
	
.data
.align
	// This section is evaluated before execution to put things into
	// memory that are required for the execution of your application
.end
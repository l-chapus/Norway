.global _start

_start:
	// Here your execution starts
	ldr r0, =li			// li address
    ldr r2, =d			// d address
	ldrb r1, [r2]  		// d
	mov r2, #0			// lengh of li
	mov r4, #0			
	
	//ldrb r2, [r0]		get li[0]
	//ldrb r6, [r0,#4]		//get li[1]
	//ldrb r3, [r1]  	get d
	bl lengh_li
	sub r2, r2, #1

	mov r3, #0			// index 1 
	mov r4, #0			// index 2
	mov r5, #0			// sum
	mvn r6, #1			// out 1
	mvn r7, #1			// out 2
	ldrb r8, [r0,r3]	// temp 1
	ldrb r9, [r0,r4]	// temp 2
	mov r10, #0			// index 1 bis
	mov r11, #0			// index 2 bis

	b loop_test

	b _exit

lengh_li:
	ldrb r3, [r0,r4]
	cmp r3, #0
	bne .+12
	mov r4, #0
	bx lr

	add r2,r2,#1
	add r4,r4,#4

	b lengh_li	

loop_test:
	cmp r5,r1
	beq find

	cmp r4,r2
	bne .+20
	add r10, r10, #4
	ldrb r9, [r0,r11]
	add r4, r4, #1
	mov r3, r4
	
	add r11, r11, #4
	ldrb r8, [r0,r10]	
	add r3, r3, #1
	
	add r5, r8, r9

	cmp r3,r2
	beq write_text

	b loop_test

find:
	add r6, r6, #1
	add r7, r7, #1

	add r6, r6, r3
	add r7, r7, r4

	b write_text

write_text:
	b _exit

_exit:
	// Branch here for exit
	b .
	
.data
.align
    li:
        .word 1
        .word 3
        .word 4
        .word 7
        .word 9
        .word 12
    d:
        .word 11

.end
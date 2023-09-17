.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	// Here your execution starts
	
	ldr r0, =input
	mov r1, #1      // by default every text is a palindrom
	mov r2, #0		// the default lenght is 0
	ldrb r3, [r0]   // get the first character

	bl check_input       // branch link to check_input
	cmp r2, #2           // Compare r2 with 2
    bge .+8				 // if r2 >= 2 then skip the next instruction 	
	b _exit 			 // exit the app
	

	mov r3, #0			 // first index
	ldrb r4, [r0,#0]     // first character
	sub r2, r2, #1		 // remove 1 from the text lengh to have the last index
	mov r5, r2 			 // last index
	ldrb r6, [r0,r5]	 // last charachter

	b check_palindrom   // branch link to check_input
	
	
check_input:
	// You could use this symbol to check for your input length
	// you can assume that your input string is at least 2 characters 
	// long and ends with a null byte

	cmp r3, #0           	// Compare r3 with 0
    bne not_satisfied 	 	// the text if not finish
	beq satisfied		 	// this is the end of the text

satisfied:
	bx lr					// return to the main branch

not_satisfied:
    add r2, r2, #1			// add 1 to r2
	ldrb r3, [r0,r2]        // update the character
    b check_input			// go back into the loop
	
	
check_palindrom:
	// Here you could check whether input is a palindrom or not

	bl space_removal 		// check and change the index if there is a space
	bl upper_cases 			// change the lower character to upper character

	cmp r4,r6
	bne is_no_palindrom		// if the two characters are not the same we stop the loop

	add r3, r3, #1			// increase the index of the first character
	ldrb r4, [r0,r3]        // update the first character
	sub r5, r5, #1		 	// decrease the index of the last character
	ldrb r6, [r0,r5]		// update the last character

	cmp r3, r5				// compare the two index to know if we have donne the entire text
	bgt is_palindrom		// if we don't go out before that mean this is a palindrom
	ble check_palindrom		// go back into the loop


space_removal:
	cmp r4, #0x20			// #0x20 is the code for a space
	bne .+12				// if it's not equal then skip the 2 next instructions
	add r3, r3, #1			// increase the left index to skip the space
	ldrb r4, [r0,r3] 		// refresh the characte

	cmp r6, #0x20
	bne .+12				// if it's not equal then skip the 2 next instructions
	sub r5, r5, #1			// decrease the right index
	ldrb r6, [r0,r5]		// refresh the character

	cmp r4, #0x20			// in case there are 2 space at the left
	beq space_removal		// recall the function to remoce the space

	cmp r6, #0x20			// in case thare are 2 space at the right
	beq space_removal		// recall the function to remoce the space

	bx lr					// go back to the loop

upper_cases:
	cmp r4, #0x61		
	blt .+8					// if it's not equal then skip the 2 next instructions
	sub r4, r4, #0x20 

	cmp r6, #0x61
	blt .+8					// if it's not equal then skip the 2 next instructions
	sub r6, r6, #0x20

	bx lr

is_palindrom:
	// Switch on only the 5 leftmost LEDs
	// Write 'Palindrom detected' to UART
	
	ldr r0, =0xFF200000   	// set the address for the LED
	mov r1, #0x3E0        	// stock 992 into r1
	str r1, [r0] 		  	// change the value at the address r0 to light the 5 leftmost LEDs

	ldr r0, =0xFF201000   	// set the address for the UART
	ldr r1, =text_palindrom
	mov r3, #0
	b loop_text_writing	  	// go to the loop to write the sentence 'Palindrom detected'

	
is_no_palindrom:
	// Switch on only the 5 rightmost LEDs
	// Write 'Not a palindrom' to UART

	ldr r0, =0xFF200000   	// set the address for the LED
	mov r1, #0x1F         	// stock 1F into r1
	str r1, [r0] 		  	// change the value at the address r0 to light the 5 rightmost LEDs

	ldr r0, =0xFF201000   	// set the address for the UART
	ldr r1, =text_no_palindrom
	mov r3, #0
	b loop_text_writing	  	// go to the loop to write the sentence 'Not a palindrom'

loop_text_writing:
	ldrb r2, [r1,r3]			// load the character
	str r2, [r0]				// print the character on the screen

	add r3, r3, #1				// increase the index
	cmp r2, #0					// compare the character to a null byte
	bne loop_text_writing		// return to the loop 
	beq _exit 		     		// exit the app 

_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "Grav ned den varg"
	text_no_palindrom: .asciz "Not a palindrom"
	text_palindrom: .asciz "Palindrom detected"
.end
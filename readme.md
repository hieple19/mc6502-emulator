Hiep Le - 12/7/2018
MC6502 - an MC6502 Simulator/ Debugger

General Program Structure: 

	opcode.c 	- Read operation codes, corresponding hex and addressing mode frompcode.txt
	readin.c 	- Read binary image. Store in array of bytes
	machine.c 	- Contains functions for all 56 instructions.
				- Process every instruction, update program counter and print data

	main.c 		- I/O with user to choose executable files and simulator commands

Using Program
	
	Program takes in 1 argument
		- The .exc file to read and simulate

	In Command Line: 	./sim <file.exc>
					e.g ./sim fib.exc

	Program: e - step one step
			 p - print data
			 x - exit program
			 c $xxxx - continue to run till address
			 x $xxxx - examine byte at a particular location

References
	
	http://skilldrick.github.io/easy6502/
	https://dwheeler.com/6502/oneelkruns/asm1step.html
	http://www.6502.org/tutorials/6502opcodes.html
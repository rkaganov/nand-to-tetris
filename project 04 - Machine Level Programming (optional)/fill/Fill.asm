	@SCREEN
	D=A
	@addr 
	M=D
(INFINITE_LOOP)	
	@KBD
	D=M
@CLEAR_SCREEN
	D;JLE        
(BLACKEN)
	@addr
	D=M
	@KBD    
	D=D-A
	@INFINITE_LOOP
	D;JGE
	@addr
	M=M+1
	@KBD
	D=D+A 
	A=D
	M=-1
	@INFINITE_LOOP
	0;JMP
(CLEAR_SCREEN)
	@addr
	D=M-1
	@SCREEN
	D=D-A
	@INFINITE_LOOP
	D;JLT
	@addr
	M=M-1
	@SCREEN
	D=D+A
	A=D
	M=0
	@INFINITE_LOOP
	0;JMP
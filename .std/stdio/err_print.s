err_print:	mov rax, 0x01
			mov rdi, 1
			mov rsi, ERROR_MESSAGE
			mov rdx, ERROR_MESSAGE_LEN
			syscall
			
			ret
			
ERROR_MESSAGE:	db "> If you see that message, something in compilation gone wrong, try compile with -v option and watch correctness of program tree"
ERROR_MESSAGE_LEN equ $ - ERROR_MESSAGE

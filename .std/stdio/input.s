input:		sub rsp, 2
			mov byte [rsp], '?'
			mov byte [rsp + 1], ' '			
			
			mov rax, 0x01
			mov rdi, 1
			mov rsi, rsp
			mov rdx, 2
			syscall
			
			add rsp, 2
			
			
			sub rsp, 16			;// creating local buffer for reading
			mov r15, rsp
			mov r14, rsp
			
			mov rax, 0x00
			mov rdi, 0
			mov rsi, r15
			mov rdx, 16
			syscall				;// in RAX stores count of readed symbols
			
			add r14, rax		;// current position
			sub r14, 2
			
			mov rdi, 1			;// power of current symbol in RDX
			
			sub rsp, 8			;// creating two dword variables for mantissa and exhibitor
			mov dword [rsp], 1
			mov dword [rsp + 4], 0
			
			
	.cycle:	xor rax, rax
			mov al, byte [r14]
			
			cmp al, '.'
			je .point
			
			cmp al, ','
			je .point
			
			cmp al, '-'
			je .minus
			
			sub rax, '0'
			
			mul edi
			imul edi, 10
			add dword [rsp + 4], eax
			dec r14
			
			cmp r14, r15
			jge .cycle
			jmp .plus
			
			
	.point:	mov dword [rsp], edi
			dec r14
			jmp .cycle
			
			
	.minus:	fild dword [rsp + 4]
			fild dword [rsp]
			
			fdivp
			fchs
						
			jmp .exit
						
			
	.plus:	fild dword [rsp + 4]
			fild dword [rsp]
			
			fdivp
			
	.exit:	add rsp, 24
			
			ret
			
PREFIX:	db	"? "

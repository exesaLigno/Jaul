print:		fld dword [rsp + 8]
			
			mov rax, 1000000
			push rax
			fild qword [rsp]
			
			fmulp
			frndint
			
			fistp qword [rsp]
			mov rax, qword [rsp]
			
			mov r15, rsp
			dec r15			;// last symbol pointer
			mov r14, 1		;// count of symbols
			mov r13, 6		;// symbols after point
			
			mov r11, 0
			cmp rax, 0
			jnl .skip
			mov r11, 1
			neg rax
			
	.skip:	sub rsp, 32
			
			mov byte [r15], 0xA
			dec r15
			
			mov r12, 0		;// skiping zeros
			
	.cycle:		xor rdx, rdx
				mov rbx, 10
				div rbx
				
				cmp r13, 0
				je .cont2
				
				cmp dl, 0
				je .cont1
				mov r12, 1
				
		.cont1:	cmp r12, 0
				jne .cont2
				
				dec r13
				jmp .cycle
				
		.cont2:	mov r12, 1
				add dl, '0'
				mov byte [r15], dl
				dec r15
				inc r14
				dec r13
				
				cmp r13, 0
				je .point
				
				cmp rax, 0
				jne .cycle
			
			jmp .sign
			
	.point:	mov byte [r15], '.'
			dec r15
			inc r14
			dec r13
			mov r12, 1
			jmp .cycle
			
	.sign:	cmp r11, 0
			je .sys
			mov byte [r15], '-'
			dec r15
			inc r14
			
	.sys:	dec r15
			add r14, 2
			mov byte [r15], '>'
			mov byte [r15 + 1], ' '
			mov rax, 0x01
			mov rdi, 1
			mov rsi, r15
			mov rdx, r14
			syscall
			
			add rsp, 32
			add rsp, 8
			
			ret
	

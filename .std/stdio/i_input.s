input:		pop r15
			sub rsp, 16
			mov r14, rsp	; r14 - buffer start
			
			mov rax, 0x00
			mov rdi, 0
			mov rsi, r14
			mov rdx, 16
			syscall
			
			mov rcx, rax
			dec rcx

			add r14, rcx
			
			mov rbx, 1
			mov rdi, 10
			
			xor rsi, rsi
			
.cycle:		dec r14
			dec rcx
			
			cmp byte [r14], '-'
			je .negative
			
			xor rax, rax
			mov al, byte [r14]
			sub al, '0'
			mul ebx
			add rsi, rax
			
			mov rax, rbx
			mul edi
			mov rbx, rax
			
			cmp rcx, 0
			jne .cycle
			jmp .exit
			
.negative:	neg rsi
			
.exit:		mov rax, rsi

			add rsp, 16
			
			push r15
			
			ret

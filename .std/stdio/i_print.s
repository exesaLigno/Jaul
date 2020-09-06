

print:		pop r15				; saving retcode
			
			pop rax				; number to rax
			push rax			; restoring stack
			
			mov rcx, 1
			mov rbx, 10
			
			mov r14, rsp
			dec r14
			mov byte [r14], 0xA
			dec r14
			
			sub rsp, 16
			
			cmp rax, 0
			jl .negative
			
.cycle1:	xor rdx, rdx
			div ebx
			mov byte [r14], dl
			add byte [r14], '0'
			inc rcx
			dec r14
			cmp rax, 0
			jne .cycle1
			
			jmp .print
			
.negative:	neg rax

.cycle2:	xor rdx, rdx
			div ebx
			mov byte [r14], dl
			add byte [r14], '0'
			inc rcx
			dec r14
			cmp rax, 0
			jne .cycle2
			
			mov byte [r14], '-'
			dec r14
			inc rcx
			
			jmp .print
			
			
			
.print:		mov rax, 0x01
			mov rdi, 1
			mov rsi, r14
			inc rsi
			mov rdx, rcx
			syscall
			
			
			
			add rsp, 16
			
			push r15
			
			ret



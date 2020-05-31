section .text
	global _start

_start:
	pop r13
	mov r15, rsp
	sub rsp, 16
	mov r14, rsp

	push r13
	push r14
	push r15

	call input
	pop r15
	pop r14
	pop r13

	push rax
	mov rax, r14
	add rax, 0
	push rax

	pop rbx
	pop rax
	mov [rbx], rax

	push r13
	push r14
	push r15

	mov rax, [r14 + 0]
	push rax

	call power_of_two
	pop r15
	pop r15
	pop r14
	pop r13

	push rax
	mov rax, r14
	add rax, 8
	push rax

	pop rbx
	pop rax
	mov [rbx], rax

	push r13
	push r14
	push r15

	mov rax, [r14 + 8]
	push rax

	call print
	pop r15
	pop r15
	pop r14
	pop r13

	push rax
	mov rax, 0
	push rax

	pop rax

	add rsp, 16
	push r13
	mov rdi, rax
	mov rax, 60
	syscall


power_of_two:
	pop r13
	mov r15, rsp
	sub rsp, 8
	mov r14, rsp

	mov rax, 1
	push rax

	mov rax, r14
	add rax, 0
	push rax

	pop rbx
	pop rax
	mov [rbx], rax

.cycle0:
	mov rax, 0
	push rax

	mov rax, [r15 + 0]
	push rax

	pop rax
	pop rbx
	xor rcx, rcx
	xor rdx, rdx
	cmp rax, rbx
	lahf
	mov cl, ah
	mov dl, ah
	and cl, 01000000b
	xor cl, 01000000b
	shr cl, 6
	and dl, 10000000b
	xor dl, 10000000b
	shr dl, 7
	and rcx, rdx
	push rcx

	pop rax
	test rax, rax
	jz .exitcycle0
	mov rax, 2
	push rax

	mov rax, [r14 + 0]
	push rax

	pop rax
	pop rbx
	imul ebx
	push rax

	mov rax, r14
	add rax, 0
	push rax

	pop rbx
	pop rax
	mov [rbx], rax

	mov rax, 1
	push rax

	mov rax, [r15 + 0]
	push rax

	pop rax
	pop rbx
	sub rax, rbx
	push rax

	mov rax, r15
	add rax, 0
	push rax

	pop rbx
	pop rax
	mov [rbx], rax

	jmp .cycle0
	.exitcycle0:

	mov rax, [r14 + 0]
	push rax

	pop rax

	add rsp, 8
	push r13
	ret




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

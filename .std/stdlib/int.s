sin:		fld dword [rsp + 8]
			frndint
			fistp dword [rsp + 8]
			xor rax, rax
			mov eax, dword [rsp + 8]
			ret

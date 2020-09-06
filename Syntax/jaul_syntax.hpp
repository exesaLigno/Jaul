/*!
 *	@file jaul_syntax.hpp
 *	@brief Инструкции для компиляции высокоуровневых операций в ассемблерный код
 */

#include "../Headers/ast.hpp"

TOKEN("+", 	ASN::ARITHM_OPERATOR, ASN::PLUS, 	 	 "+", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 8\nfadd\nfstp dword [rsp]")
TOKEN("-", 	ASN::ARITHM_OPERATOR, ASN::MINUS, 	 	 "-", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 8\nfsub\nfstp dword [rsp]")
TOKEN("-", ASN::ARITHM_OPERATOR, ASN::UNARY_MINUS,	 "- (unary)", "fld dword [rsp]\nfchs\nfstp dword [rsp]")
TOKEN("*", 	ASN::ARITHM_OPERATOR, ASN::MULTIPLY, 	 "*", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 8\nfmul\nfstp dword [rsp]")
TOKEN("/", 	ASN::ARITHM_OPERATOR, ASN::DIVIDE, 	 	 "/", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 8\nfdiv\nfstp dword [rsp]")
//TOKEN("^", 	ASN::ARITHM_OPERATOR, ASN::POWER, 	 	 "^", "  N|I power")
TOKEN("%", 	ASN::ARITHM_OPERATOR, ASN::MODULO, 	 	 "%", "pop rax\npop rbx\nidiv rbx\npush rdx")
TOKEN("//", ASN::ARITHM_OPERATOR, ASN::INT_DIVISION, "//", "pop rax\npop rbx\nidiv rbx\npush rax")

TOKEN("==", ASN::CMP_OPERATOR, ASN::EQUAL, 		"==", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 16\nxor rcx, rcx\nfucomip\nsete cl\nffree\npush rcx")
TOKEN("!=", ASN::CMP_OPERATOR, ASN::NOT_EQUAL, 	"!=", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 16\nxor rcx, rcx\nfucomip\nsetne cl\nffree\npush rcx")
TOKEN(">=", ASN::CMP_OPERATOR, ASN::MORE_EQ, 	"More or equal", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 16\nxor rcx, rcx\nfucomip\nsetbe cl\nffree\npush rcx")
TOKEN("<=", ASN::CMP_OPERATOR, ASN::LESS_EQ, 	"Less or equal", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 16\nxor rcx, rcx\nfucomip\nsetae cl\nffree\npush rcx")
TOKEN(">", 	ASN::CMP_OPERATOR, ASN::MORE, 		"More", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 16\nxor rcx, rcx\nfucomip\nsetb cl\nffree\npush rcx")
TOKEN("<", 	ASN::CMP_OPERATOR, ASN::LESS, 		"Less", "fld dword [rsp]\nfld dword [rsp + 8]\nadd rsp, 16\nxor rcx, rcx\nfucomip\nseta cl\nffree\npush rcx")
TOKEN("&&", ASN::CMP_OPERATOR, ASN::AND, 		"And", "pop rax\npop rbx\nand rax, rbx\npush rax")
TOKEN("||", ASN::CMP_OPERATOR, ASN::OR, 		"Or", "pop rax\npop rbx\nor rax, rbx\npush rax")

TOKEN("if", 	ASN::CTRL_OPERATOR, ASN::IF, 					"if", "nop")
TOKEN("else", 	ASN::CTRL_OPERATOR, ASN::ELSE, 					"else", "nop")
TOKEN("repeat", ASN::CTRL_OPERATOR, ASN::FOR, 					"repeat", "nop")
TOKEN("while", 	ASN::CTRL_OPERATOR, ASN::WHILE, 				"while", "nop")
TOKEN("return", ASN::CTRL_OPERATOR, ASN::RETURN, 				"return", "pop rax")
TOKEN("=", 		ASN::CTRL_OPERATOR, ASN::ASSIGNMENT, 			"=", "pop rbx\npop rax\nmov dword [rbx], eax")
//TOKEN("+=", 	ASN::CTRL_OPERATOR, ASN::PLUS_ASSIGNMENT, 		"+=", "pop rbx\npop rax\nadd [rbx], rax")
//TOKEN("-=", 	ASN::CTRL_OPERATOR, ASN::MINUS_ASSIGNMENT, 		"-=", "pop rbx\npop rax\nsub [rbx], rax")
//TOKEN("*=", 	ASN::CTRL_OPERATOR, ASN::MULTIPLY_ASSIGNMENT, 	"*=", "imul [rbx], rax")
//TOKEN("/=", 	ASN::CTRL_OPERATOR, ASN::DIVIDE_ASSIGNMENT, 	"/=", "idiv [rbx], rax")
//TOKEN("^=", 	ASN::CTRL_OPERATOR, ASN::POWER_ASSIGNMENT, 		"^=", "power [rbx], rax")



/*
%macro equal 0
	pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nlahf\nmov cl, ah\nand cl, 01000000b\nshr cl, 6\npush rcx
%endmacro

%macro not_equal 0
	xor rcx, rcx
	cmp rax, rbx
	lahf
	mov cl, ah
	and cl, 01000000b
	xor cl, 01000000b
	shr cl, 6
	push rcx
%endmacro

%macro less 0
	xor rcx, rcx
	cmp rax, rbx
	lahf
	mov cl, ah
	and cl, 10000000b
	shr cl, 7
	push rcx
%endmacro

%macro less_eq 0
	xor rcx, rcx
	xor rdx, rdx
	cmp rax, rbx
	lahf
	mov cl, ah
	mov dl, ah
	and cl, 01000000b
	shr cl, 6
	and dl, 10000000b
	shr dl, 7
	or rcx, rdx
	push rcx
%endmacro

%macro more 0
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
%endmacro

%macro more_eq 0
	xor rcx, rcx
	cmp rax, rbx
	lahf
	mov cl, ah
	and cl, 10000000b
	xor cl, 10000000b
	shr cl, 7
	push rcx
%endmacro
*/

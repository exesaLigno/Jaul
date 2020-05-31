#include "token.hpp"

TOKEN("+", 	Token::ARITHM_OPERATOR, Token::PLUS, 	 	 "+", "\tpop rax\n\tpop rbx\n\tadd rax, rbx\n\tpush rax\n\n", "58 5b 48 01 d8 50")
TOKEN("-", 	Token::ARITHM_OPERATOR, Token::MINUS, 	 	 "-", "\tpop rax\n\tpop rbx\n\tsub rax, rbx\n\tpush rax\n\n", "58 5b 48 29 d8 50")
TOKEN("*", 	Token::ARITHM_OPERATOR, Token::MULTIPLY, 	 "*", "\tpop rax\n\tpop rbx\n\timul ebx\n\tpush rax\n\n", "58 5b f7 eb 50")
//TOKEN("/", 	Token::ARITHM_OPERATOR, Token::DIVIDE, 	 	 "/", "pop rax\npop rbx\nidiv ebx\npush rax\nнассыл в штаны\n\n")
//TOKEN("^", 	Token::ARITHM_OPERATOR, Token::POWER, 	 	 "^", "  N|I power\n\n")
TOKEN("%", 	Token::ARITHM_OPERATOR, Token::MODULO, 	 	 "%", "\tpop rax\n\tpop rbx\n\tidiv rbx\n\tpush rdx\n\n", "58 5b 48 f7 fb 52")
TOKEN("//", Token::ARITHM_OPERATOR, Token::INT_DIVISION, "//", "\tpop rax\n\tpop rbx\n\tidiv rbx\n\tpush rax\n\n", "58 5b 48 f7 fb 50")

TOKEN("==", Token::CMP_OPERATOR, Token::EQUAL, 		"==", "\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tand cl, 01000000b\n\tshr cl, 6\n\tpush rcx\n\n", "58 5b 48 31 c9 48 39 d8 9f 88 e1 80 e1 40 c0 e9 06 51")
TOKEN("!=", Token::CMP_OPERATOR, Token::NOT_EQUAL, 	"!=", "\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tand cl, 01000000b\n\txor cl, 01000000b\n\tshr cl, 6\n\tpush rcx\n\n", "58 5b 48 31 c9 48 39 d8 9f 88 e1 80 e1 40 80 f1 40 c0 e9 06 51")
TOKEN(">=", Token::CMP_OPERATOR, Token::MORE_EQ, 	"More or equal", "\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tand cl, 10000000b\n\txor cl, 10000000b\n\tshr cl, 7\n\tpush rcx\n\n", "58 5b 48 31 c9 48 39 d8 9f 88 e1 80 e1 80 80 f1 80 c0 e9 07 51")
TOKEN("<=", Token::CMP_OPERATOR, Token::LESS_EQ, 	"Less or equal", "\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\txor rdx, rdx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tmov dl, ah\n\tand cl, 01000000b\n\tshr cl, 6\n\tand dl, 10000000b\n\tshr dl, 7\n\tor rcx, rdx\n\tpush rcx\n\n", "58 5b 48 31 c9 48 31 d2 48 39 d8 9f 88 e1 88 e2 80 e1 40 c0 e9 06 80 e2 80 c0 ea 07 48 09 d1 51")
TOKEN(">", 	Token::CMP_OPERATOR, Token::MORE, 		"More", "\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\txor rdx, rdx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tmov dl, ah\n\tand cl, 01000000b\n\txor cl, 01000000b\n\tshr cl, 6\n\tand dl, 10000000b\n\txor dl, 10000000b\n\tshr dl, 7\n\tand rcx, rdx\n\tpush rcx\n\n", "58 5b 48 31 c9 48 31 d2 48 39 d8 9f 88 e1 88 e2 80 e1 40 80 f1 40 c0 e9 06 80 e2 80 80 f2 80 c0 ea 07 48 21 d1 51")
TOKEN("<", 	Token::CMP_OPERATOR, Token::LESS, 		"Less", "\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tand cl, 10000000b\n\tshr cl, 7\n\tpush rcx\n\n", "58 5b 48 31 c9 48 39 d8 9f 88 e1 80 e1 80 c0 e9 07 51")
TOKEN("&&", Token::CMP_OPERATOR, Token::AND, 		"&&", "\tpop rax\n\tpop rbx\n\tand rax, rbx\n\tpush rax\n\n", "58 5b 48 21 d8 50")
TOKEN("||", Token::CMP_OPERATOR, Token::OR, 		"||", "\tpop rax\n\tpop rbx\n\tor rax, rbx\n\tpush rax\n\n", "58 5b 48 09 d8 50")

TOKEN("if", 	Token::CTRL_OPERATOR, Token::IF, 					"if", "\tnop\n", "")
TOKEN("else", 	Token::CTRL_OPERATOR, Token::ELSE, 					"else", "\tnop\n", "")
TOKEN("for", 	Token::CTRL_OPERATOR, Token::FOR, 					"for", "\tnop\n", "")
TOKEN("while", 	Token::CTRL_OPERATOR, Token::WHILE, 				"while", "\tnop\n", "")
TOKEN("return", Token::CTRL_OPERATOR, Token::RETURN, 				"return", "\tpop rax\n\n", "58")
TOKEN("=", 		Token::CTRL_OPERATOR, Token::ASSIGNMENT, 			"=", "\tpop rbx\n\tpop rax\n\tmov [rbx], rax\n\n", "5b 58 48 89 03")
//TOKEN("+=", 	Token::CTRL_OPERATOR, Token::PLUS_ASSIGNMENT, 		"+=", "\tpop rbx\n\tpop rax\n\tadd [rbx], rax\n\n")
//TOKEN("-=", 	Token::CTRL_OPERATOR, Token::MINUS_ASSIGNMENT, 		"-=", "\tpop rbx\n\tpop rax\n\tsub [rbx], rax\n\n")
//TOKEN("*=", 	Token::CTRL_OPERATOR, Token::MULTIPLY_ASSIGNMENT, 	"*=", "imul [rbx], rax\n\n")
//TOKEN("/=", 	Token::CTRL_OPERATOR, Token::DIVIDE_ASSIGNMENT, 	"/=", "idiv [rbx], rax\n\n")
//TOKEN("^=", 	Token::CTRL_OPERATOR, Token::POWER_ASSIGNMENT, 		"^=", "power [rbx], rax\n\n")



/*
%macro equal 0
	\tpop rax\n\tpop rbx\n\txor rcx, rcx\n\tcmp rax, rbx\n\tlahf\n\tmov cl, ah\n\tand cl, 01000000b\n\tshr cl, 6\n\tpush rcx
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

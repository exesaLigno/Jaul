/*!
 *	@file binary.cpp
 *	@brief Исполняемый код класса Binary и вложенных классов
 */


#include "../Headers/binary.hpp"

Binary::Binary()
{}

Binary::~Binary()
{
	if (this -> start != nullptr)
		delete this -> start;

	if (this -> labels != nullptr)
		delete[] this -> labels;
}



void Binary::importAST(ASN* node)
{
	if (node -> type == ASN::FUNC)								// Компиляция кода функции
	{
		this -> pushBack(Token::FUNCTION_LABEL, node -> svalue);	// Создание узла с названием функции
		this -> pushBack("sub rsp, %d", (node -> ivalue) * 8);		// Выделение места под локальные переменные
		this -> pushBack("mov r15, rsp");							// Сохранение отступа переменных в r15
		
		importAST(node -> left);									// Компиляция тела функции
		
		this -> pushBack("add rsp, %d", (node -> ivalue) * 8);		// Освобождение места от локальных переменных
		
		if (!strcmp(node -> svalue, "_start"))						// Добавление выхода из программы
		{
			this -> pushBack("mov rdi, %d", 0);
			this -> pushBack("mov rax, 60");
			this -> pushBack("syscall");
		}
		
		else
			this -> pushBack("ret");								// Добавление возврата из функции
	}
	
	else if (node -> type == ASN::FUNCCALL)						// Компиляция вызова функции
	{
		this -> pushBack("push r15");								// Сохранение положения локальных перменных
		
		ASN* declaration = node -> bounded_node != nullptr ? node -> bounded_node : node;
		
		ASN* call_parameter = node -> right;
		ASN* decl_parameter = declaration -> right;
		
		int params_count = 0;
		
		while (call_parameter != nullptr)
		{
			call_parameter = call_parameter -> left;
			params_count++;
		}
		
		call_parameter = node -> right;
		
		for (int counter = 0; counter < params_count - 1; counter++)
		{
			call_parameter = call_parameter -> left;
			decl_parameter = decl_parameter -> left;
		}
				
		for (int counter = 0; counter < params_count; counter++)			// После импорта интовской конструкции результат закономерно останется в стэке,
		{																	// после флоатовской - в FPU. Его необходимо выгрузить в оперативку и очистить FPU
			this -> importAST(call_parameter -> right);						// Если оказалось, что int необходимо перевести во float, просто кормим его FPU и выгружем обратно
			
			if (call_parameter -> right -> data_type == ASN::FLOAT)
			{
				this -> pushBack("sub rsp, %d", 8);
				this -> pushBack("fstp dword [rsp]");
			}
			
			else if (call_parameter -> right -> data_type == ASN::INT and decl_parameter -> right -> data_type == ASN::FLOAT)
			{
				this -> pushBack("fild dword [rsp]");
				this -> pushBack("fstp dword [rsp]");
			}
			
			call_parameter = call_parameter -> parent;
			decl_parameter = decl_parameter -> parent;
		}
		
		this -> pushBack("call %s", node -> svalue);				// Вызов функции
		
		this -> pushBack("add rsp, %d", params_count * 8);			// Очистка места, занятого параметрами
		
		this -> pushBack("pop r15");								// Восстановление положения локальных переменных
		
		if (node -> data_type == ASN::INT)							// Перемещение результата выполнения из rax в необходимое расположение
			this -> pushBack("push rax");							// (обычный стэк или стэк регистров FPU)
																	// В последнем случае переносить результат не надо, он и так находится в регистре FPU
		else if (node -> data_type == ASN::FLOAT);
	}
	
	
	
	
	else if (node -> type == ASN::VARIABLE)
	{
		if (node -> LValue)
		{
			this -> pushBack("lea rax, [r15 + %d]", (node -> ivalue) * 8);
			this -> pushBack("push rax");
		}
		
		else if (node -> data_type == ASN::INT)
			this -> pushBack("push qword [r15 + %d]", (node -> ivalue) * 8);
		
		else if (node -> data_type == ASN::FLOAT)
			this -> pushBack("fld dword [r15 + %d]", (node -> ivalue) * 8);
		
		else
			this -> pushBack("no-type variable");
	}
	
	
	

	else if (node -> type == ASN::CONSTANT and node -> data_type == ASN::INT)
	{
		this -> pushBack("mov rax, %d", node -> ivalue);
		this -> pushBack("push rax");
	}

	else if (node -> type == ASN::CONSTANT and node -> data_type == ASN::FLOAT)
	{
		this -> pushBack("mov rax, __float32__(%f)", node -> fvalue);
		this -> pushBack("push rax");
		this -> pushBack("fld dword [rsp]");
		this -> pushBack("add rsp, 8");
	}
	
	
	

	else if (node -> type == ASN::CTRL_OPERATOR)
	{
		if (node -> ivalue == ASN::ASSIGNMENT)
		{
			if (node -> right)
				this -> importAST(node -> right);
			
			if (node -> left)
				this -> importAST(node -> left);
			
			if (node -> data_type == ASN::INT)
				this -> pushBack("pop rax\npop rbx\nmov dword [rax], ebx");
			
			else if (node -> data_type == ASN::FLOAT)
				this -> pushBack("pop rax\nfstp dword [rax]");
		}
		
		
		else if (node -> ivalue == ASN::RETURN)
		{
			if (node -> right)
				this -> importAST(node -> right);
			
			if (node -> data_type == ASN::INT)
				this -> pushBack("pop rax");
		}
		
		
		else if (node -> ivalue == ASN::WHILE)
		{
			this -> pushBack(Token::LOCAL_LABEL, ".cycle%d", node -> vartype);
			
			if (node -> right)
				importAST(node -> right);
			
			this -> pushBack("pop rax");
			this -> pushBack("test rax, rax");
			this -> pushBack("jz .exitcycle%d", node -> vartype);
			
			if (node -> left)
				importAST(node -> left);
			
			this -> pushBack("jmp .cycle%d", node -> vartype);
			
			this -> pushBack(Token::LOCAL_LABEL, ".exitcycle%d", node -> vartype);
		}
		
		
		else if (node -> ivalue == ASN::FOR)	// TODO make for compilation
		{
			this -> pushBack("for cycle");
		}
		
		
		else if (node -> ivalue == ASN::IF)
		{
			if (node -> right)
				importAST(node -> right);
			
			this -> pushBack("pop rax");
			this -> pushBack("test rax, rax");
			this -> pushBack("jz .endif%d", node -> vartype);
			
			if (node -> left)
				importAST(node -> left);
			
			this -> pushBack("jmp .exitif%d", node -> vartype);
			
			this -> pushBack(Token::LOCAL_LABEL, ".endif%d", node -> vartype);
			
			
			// Else compilation
			if (node -> parent -> left)
			{
				if (node -> parent -> left -> right -> type == ASN::CTRL_OPERATOR and node -> parent -> left -> right -> ivalue == ASN::ELSE)
					importAST(node -> parent -> left -> right -> left);
			}
			
			this -> pushBack(Token::LOCAL_LABEL, ".exitif%d", node -> vartype);
		}
	}
	
	
	
	
	else if (node -> type == ASN::CMP_OPERATOR)
	{
		int data_type = node -> left -> data_type == ASN::INT and node -> right -> data_type == ASN::INT ? ASN::INT : ASN::FLOAT;
		
		this -> importAST(node -> right);
		if (data_type == ASN::FLOAT and node -> right -> data_type == ASN::INT)
		{
			this -> pushBack("fild dword [rsp]");
			this -> pushBack("add rsp, %d", 8);
		}
		
		if (node -> left -> type == ASN::FUNCCALL and data_type == ASN::FLOAT)
		{
			this -> pushBack("sub rsp, %d", 8);
			this -> pushBack("fstp dword [rsp]");
		}
		
		this -> importAST(node -> left);
		
		if (data_type == ASN::FLOAT and node -> left -> data_type == ASN::INT)
		{
			this -> pushBack("fild dword [rsp]");
			this -> pushBack("add rsp, %d", 8);
		}
		
		if (node -> left -> type == ASN::FUNCCALL and data_type == ASN::FLOAT)
		{
			this -> pushBack("fld dword [rsp]");
			this -> pushBack("fxch");
			this -> pushBack("add rsp, %d", 8);
		}
		
		
		if (node -> data_type == ASN::FLOAT)
		{
			if (node -> ivalue == ASN::EQUAL)
				this -> pushBack("xor rcx, rcx\nfucomip\nsete cl\nffree\npush rcx");
			
			else if (node -> ivalue == ASN::NOT_EQUAL)
				this -> pushBack("xor rcx, rcx\nfucomip\nsetne cl\nffree\npush rcx");
			
			else if (node -> ivalue == ASN::MORE)
				this -> pushBack("xor rcx, rcx\nfucomip\nseta cl\nffree\npush rcx");
			
			else if (node -> ivalue == ASN::LESS)
				this -> pushBack("xor rcx, rcx\nfucomip\nsetb cl\nffree\npush rcx");
			
			else if (node -> ivalue == ASN::MORE_EQ)
				this -> pushBack("xor rcx, rcx\nfucomip\nsetae cl\nffree\npush rcx");
			
			else if (node -> ivalue == ASN::LESS_EQ)
				this -> pushBack("xor rcx, rcx\nfucomip\nsetbe cl\nffree\npush rcx");
			
			else if (node -> ivalue == ASN::AND)
				this -> pushBack("pop rax\npop rbx\nand rax, rbx\npush rax");
			
			else if (node -> ivalue == ASN::OR)
				this -> pushBack("pop rax\npop rbx\nor rax, rbx\npush rax");
		}
		
		if (node -> data_type == ASN::INT)
		{
			if (node -> ivalue == ASN::EQUAL)
				this -> pushBack("pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nsete cl\npush rcx");
			
			else if (node -> ivalue == ASN::NOT_EQUAL)
				this -> pushBack("pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nsetne cl\npush rcx");
			
			else if (node -> ivalue == ASN::MORE)
				this -> pushBack("pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nsetg cl\npush rcx");
			
			else if (node -> ivalue == ASN::LESS)
				this -> pushBack("pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nsetl cl\npush rcx");
			
			else if (node -> ivalue == ASN::MORE_EQ)
				this -> pushBack("pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nsetge cl\npush rcx");
			
			else if (node -> ivalue == ASN::LESS_EQ)
				this -> pushBack("pop rax\npop rbx\nxor rcx, rcx\ncmp rax, rbx\nsetle cl\npush rcx");
			
			else if (node -> ivalue == ASN::AND)
				this -> pushBack("pop rax\npop rbx\nand rax, rbx\npush rax");
			
			else if (node -> ivalue == ASN::OR)
				this -> pushBack("pop rax\npop rbx\nor rax, rbx\npush rax");
		}
	}
	
	
	
	
	else if (node -> type == ASN::ARITHM_OPERATOR)
	{
		if (node -> right)
		{
			importAST(node -> right);
			if (node -> data_type == ASN::FLOAT and node -> right -> data_type == ASN::INT)
			{
				this -> pushBack("fild dword [rsp]");
				this -> pushBack("add rsp, %d", 8);
			}
		}
		
		if (node -> left)
		{
			if (node -> left -> type == ASN::FUNCCALL and node -> data_type == ASN::FLOAT)
			{
				this -> pushBack("sub rsp, %d", 8);
				this -> pushBack("fstp dword [rsp]");
			}
			
			this -> importAST(node -> left);
			
			if (node -> data_type == ASN::FLOAT and node -> left -> data_type == ASN::INT)
			{
				this -> pushBack("fild dword [rsp]");
				this -> pushBack("add rsp, %d", 8);
			}
			
			if (node -> left -> type == ASN::FUNCCALL and node -> data_type == ASN::FLOAT)
			{
				this -> pushBack("fld dword [rsp]");
				this -> pushBack("fxch");
				this -> pushBack("add rsp, %d", 8);
			}
		}
		
		if (node -> data_type == ASN::FLOAT)
		{
			if (node -> ivalue == ASN::PLUS)
				this -> pushBack("fadd");
			
			else if (node -> ivalue == ASN::MINUS)
				this -> pushBack("fsubr");
			
			else if (node -> ivalue == ASN::UNARY_MINUS)
				this -> pushBack("fchs");
			
			else if (node -> ivalue == ASN::MULTIPLY)
				this -> pushBack("fmul");
			
			else if (node -> ivalue == ASN::DIVIDE)
				this -> pushBack("fdivr");
		}
		
		else if (node -> data_type == ASN::INT)
		{
			if (node -> ivalue == ASN::PLUS)
				this -> pushBack("pop rax\n"
								 "pop rbx\n"
								 "add rax, rbx\n"
								 "push rax");
			
			else if (node -> ivalue == ASN::MINUS)
				this -> pushBack("pop rax\npop rbx\nsub rax, rbx\npush rax");
			
			else if (node -> ivalue == ASN::UNARY_MINUS)
				this -> pushBack("xor rax, rax\npop rbx\nsub rax, rbx\npush rax");
			
			else if (node -> ivalue == ASN::MULTIPLY)
				this -> pushBack("pop rax\npop rbx\nimul ebx\npush rax");
			
			else if (node -> ivalue == ASN::INT_DIVISION)
				this -> pushBack("pop rax\npop rbx\nxor rdx, rdx\nidiv rbx\npush rax");
			
			else if (node -> ivalue == ASN::MODULO)
				this -> pushBack("pop rax\npop rbx\nxor rdx, rdx\nidiv rbx\npush rdx");
		}
	}
	
	else
	{
		if (node -> right)
			this -> importAST(node -> right);
		
		if (node -> left)
			this -> importAST(node -> left);
	}
}



void Binary::importAST(AST* ast)
{
	this -> importAST(ast -> head);
// 	ASN* current_def = ast -> head -> left;
// 
// 	while (current_def)
// 	{
// 		importDef(current_def -> right);
// 		current_def = current_def -> left;
// 	}
}

void Binary::importDef(ASN* node)
{
	this -> pushBack(Token::FUNCTION_LABEL, node -> svalue);
	this -> pushBack("sub rsp, %d", (node -> ivalue) * 8);
	this -> pushBack("mov r15, rsp");

	importBody(node -> left);

	this -> pushBack("add rsp, %d", (node -> ivalue) * 8);

	if (!strcmp(node -> svalue, "_start"))
	{
		this -> pushBack("mov rdi, %d", 0);
		this -> pushBack("mov rax, 60");
		this -> pushBack("syscall");
	}

	else
		this -> pushBack("ret");
}

void Binary::importBody(ASN* node)
{
	int type = node -> type;
	int optype = node -> ivalue;

	if (type == ASN::FUNCCALL)
	{
		this -> pushBack("push r15");

		if (node -> right)
			importParameters(node -> right);

		this -> pushBack("call %s", node -> svalue);

		ASN* param = node -> right;

		int params_count = 0;
		while (param)
		{
			params_count++;
			param = param -> left;
		}
		this -> pushBack("add rsp, %d", params_count * 8);

		this -> pushBack("pop r15");

		if (node -> parent -> type != ASN::LINE)	// pushing retcode only if it necessary
			this -> pushBack("push rax");
	}

	else if (optype == ASN::WHILE and type == ASN::CTRL_OPERATOR)
	{
		this -> pushBack(Token::LOCAL_LABEL, ".cycle%d", node -> vartype);

		if (node -> right)
			importBody(node -> right);

		this -> pushBack("pop rax");
		this -> pushBack("test rax, rax");
		this -> pushBack("jz .exitcycle%d", node -> vartype);

		if (node -> left)
			importBody(node -> left);

		this -> pushBack("jmp .cycle%d", node -> vartype);

		this -> pushBack(Token::LOCAL_LABEL, ".exitcycle%d", node -> vartype);
	}

	else if (optype == ASN::FOR and type == ASN::CTRL_OPERATOR)
	{
		this -> pushBack("For compilation");
	}

	else if (optype == ASN::IF and type == ASN::CTRL_OPERATOR)
	{
		if (node -> right)
			importBody(node -> right);

		this -> pushBack("pop rax");
		this -> pushBack("test rax, rax");
		this -> pushBack("jz .endif%d", node -> vartype);

		if (node -> left)
			importBody(node -> left);

		this -> pushBack("jmp .exitif%d", node -> vartype);

		this -> pushBack(Token::LOCAL_LABEL, ".endif%d", node -> vartype);


		// Else compilation
		if (node -> parent -> left)
		{
			if (node -> parent -> left -> right -> type == ASN::CTRL_OPERATOR and node -> parent -> left -> right -> ivalue == ASN::ELSE)
				importBody(node -> parent -> left -> right -> left);
		}

		this -> pushBack(Token::LOCAL_LABEL, ".exitif%d", node -> vartype);
	}

	else if (optype == ASN::ELSE and type == ASN::CTRL_OPERATOR)
		this -> pushBack("nop");
	
	else if (optype == ASN::FOR and type == ASN::CTRL_OPERATOR)
	{
		
	}

	else
	{
		if (node -> right)
			importBody(node -> right);

		if (node -> left)
			importBody(node -> left);

		importNode(node);
	}
}

void Binary::importParameters(ASN* node)
{
	if (node -> left)
		importParameters(node -> left);

	if (node -> right)
		importBody(node -> right);
}

void Binary::importNode(ASN* node)
{
	if (node -> type == ASN::ARITHM_OPERATOR or node -> type == ASN::CMP_OPERATOR or node -> type == ASN::CTRL_OPERATOR)
	{
		#define TOKEN(string, token_type, token_number, dump, nasm_code)				\
				case token_number:														\
				{																		\
					this -> pushBack(nasm_code);										\
					break;																\
				}

		switch (node -> ivalue)
		{
			#include "../Syntax/jaul_syntax.hpp"

			default:
			{
				this -> pushBack("nop");
				break;
			}
		}

		#undef TOKEN
	}

	else if (node -> type == ASN::VARIABLE)
	{
		if (node -> LValue)
		{
			this -> pushBack("mov rax, r15");
			this -> pushBack("add rax, %d", (node -> ivalue) * 8);
			this -> pushBack("push rax");
		}
		
		else
		{
			this -> pushBack("mov eax, dword [r15 + %d]", (node -> ivalue) * 8);
			this -> pushBack("push rax");
		}
	}

	else if (node -> type == ASN::CONSTANT and node -> data_type == ASN::INT)
	{
		this -> pushBack("mov rax, %d", node -> ivalue);
		this -> pushBack("push rax");
	}

		else if (node -> type == ASN::CONSTANT and node -> data_type == ASN::FLOAT)
	{
		this -> pushBack("mov rax, __float32__(%f)", node -> fvalue);
		this -> pushBack("push rax");
	}
}



void Binary::importNasm(const char* nasm_code)
{
	printf("\x1b[1;31mNASM import is not implemented yet!\x1b[0m\n");
}

void Binary::importObj(const char* object_code, long int object_code_length)
{	
	long int counter = 0;
	
	while (counter < object_code_length - 1)
	{
		int funcname_length = *((int*) (object_code + counter));
		counter += sizeof(int);
		
		char* funcname = new char[funcname_length + 1]{0};
		strncpy(funcname, object_code + counter, funcname_length);
		counter += funcname_length;
		
		this -> pushBack(Token::FUNCTION_LABEL, funcname);
		
		int function_length = *((long int*) (object_code + counter));
		counter += sizeof(long int);
		
		char* function = new char[function_length]{0};
		memcpy(function, object_code + counter, function_length);
		counter += function_length;
		
		this -> pushBytes(function, function_length);
		
		delete[] funcname;
		delete[] function;
	}
}


void Binary::pushBytes(const char* function, long int function_length)
{
	Token* new_token = new Token;
	new_token -> container = this;
	
	if (not this -> start)
	{
		this -> start = new_token;
		this -> end = new_token;
	}
	
	else
	{
		this -> end -> next = new_token;
		new_token -> prev = this -> end;
		this -> end = new_token;
	}
	
	new_token -> type = Token::BYTE_CODE;
	new_token -> setBytes(function, function_length);
}


/// TODO local labels rewriting in format "global_label.local_label"
void Binary::pushBack(int type, const char* text, int ivalue, float fvalue, char cvalue, const char* svalue)
{
	Token* new_token = new Token;
	new_token -> container = this;

	if (not this -> start)
	{
		this -> start = new_token;
		this -> end = new_token;
	}

	else
	{
		this -> end -> next = new_token;
		new_token -> prev = this -> end;
		this -> end = new_token;
	}
	
	new_token -> type = type;
	if (type == Token::LOCAL_LABEL or type == Token::GLOBAL_LABEL or type == Token::FUNCTION_LABEL)
		(this -> labels_count)++;

	new_token -> setText(text);
	new_token -> ivalue = ivalue;
	new_token -> fvalue = fvalue;
	new_token -> cvalue = cvalue;
	new_token -> setSValue(svalue);
	new_token -> prepare();
}

void Binary::pushBack(const char* nasm_code)
{
	int indent = 0;

	while (*(nasm_code + indent) != '\0')
	{
		if (*(nasm_code + indent) == '\n')
			indent++;

		const char* endline = strchr(nasm_code + indent, '\n');
		const char* endcode = strchr(nasm_code + indent, '\0');

		int length = 0;

		if (endline)
			length = endline - (nasm_code + indent);

		else
			length = endcode - (nasm_code + indent);

		char* nasm_token = new char[length + 1]{0};
		strncpy(nasm_token, nasm_code + indent, length);

		this -> pushBack(Token::NASM_CODE, nasm_token, 0, 0, 0, 0);

		delete[] nasm_token;

		indent += length;
	}
}

void Binary::pushBack(const char* nasm_code, int ivalue)
{
	this -> pushBack(Token::NASM_CODE, nasm_code, ivalue, 0, 0, 0);
}

void Binary::pushBack(const char* nasm_code, float fvalue)
{
	this -> pushBack(Token::NASM_CODE, nasm_code, 0, fvalue, 0, 0);
}

void Binary::pushBack(const char* nasm_code, char cvalue)
{
	this -> pushBack(Token::NASM_CODE, nasm_code, 0, 0, cvalue, 0);
}

void Binary::pushBack(const char* nasm_code, const char* svalue)
{
	this -> pushBack(Token::NASM_CODE, nasm_code, 0, 0, 0, svalue);
}

void Binary::pushBack(int type, const char* text)
{
	this -> pushBack(type, text, 0, 0, 0, 0);
}

void Binary::pushBack(int type, const char* text, int ivalue)
{
	this -> pushBack(type, text, ivalue, 0, 0, 0);
}



void Binary::compile()
{
	Token* current = this -> start;

	while (current)
	{
		current -> compile();

		current = current -> next;
	}
}



void Binary::storeLabels()
{
	this -> labels = new Token*[this -> labels_count]{0};
	int counter = 0;

	Token* current = this -> start;

	while (current)
	{
		if (current -> type == Token::FUNCTION_LABEL or current -> type == Token::GLOBAL_LABEL or current -> type == Token::LOCAL_LABEL)
		{
			(this -> labels)[counter++] = current;
		
			for (int i = 0; i < counter - 1; i++)
			{
				if (!strcmp((this -> labels)[i] -> text, current -> text))
				{
					printf("\x1b[1;31merror:\x1b[0m function \"%s\" redefined\n", current -> text);
					this -> status = FUNCTION_REDEFINED;
				}
			}
		}
		
		current = current -> next;
	}
}



#include "binary.optimizations.cpp"



void Binary::setLabels()
{
	Token* current = this -> start;

	while (current)
	{
		if ((current -> type == Token::NASM_CODE or current -> type == Token::BOTH) and ((current -> text)[0] == 'j' or !strncmp(current -> text, "call", 4)))
		{
			long int label_position = getLabelPosition(current -> svalue);
			if (label_position == -1)
			{
				printf("\x1b[1;31merror:\x1b[0m function \"%s\" is not declared in this scope\n", current -> svalue);
				this -> status = FUNCTION_NOT_EXIST;
			}
			
			else
			{
				long int delta = label_position - ((current -> first_byte_position) + (current -> bytes_count));
				memcpy((current -> bytes) + ((current -> bytes_count) - (current -> parameter_length)), &delta, current -> parameter_length);
			}
		}
		
		current = current -> next;
	}
}



long int Binary::getLabelPosition(const char* label_name)
{
	long int position = -1;
	for (int counter = 0; counter < this -> labels_count; counter++)
	{
		if (!strcmp(label_name, (this -> labels)[counter] -> text))
		{
			position = (this -> labels)[counter] -> first_byte_position;
			break;
		}
	}

	return position;
}



int Binary::exportString(char* destination)
{
	Token* current = this -> start;
	long int global_counter = 0;
	
	while (current)
	{
		for (int local_counter = 0; local_counter < current -> bytes_count; local_counter++, global_counter++)
			destination[global_counter] = (current -> bytes)[local_counter];
		
		current = current -> next;
	}
	
	return 0;
}




int Binary::exportNasm(const char* filename)
{
	FILE* export_file = fopen(filename, "a");

	Token* current = this -> start; // i love you

	while (current)
	{
		if (current -> type != Token::BYTE_CODE)
		{
			if (current -> type == Token::FUNCTION_LABEL)
				fprintf(export_file, "\n\n");

			if (current -> type == Token::NASM_CODE)
				fprintf(export_file, "\t");


			if (strstr(current -> text, "%d") != nullptr)
				fprintf(export_file, current -> text, current -> ivalue);

			else if (strstr(current -> text, "%f") != nullptr)
				fprintf(export_file, current -> text, current -> fvalue);

			else if (strstr(current -> text, "%c") != nullptr)
				fprintf(export_file, current -> text, current -> cvalue);

			else if (strstr(current -> text, "%s") != nullptr)
				fprintf(export_file, current -> text, current -> svalue);

			else
				fprintf(export_file, "%s", current -> text);


			if (current -> type == Token::LOCAL_LABEL or current -> type == Token::GLOBAL_LABEL or current -> type == Token::FUNCTION_LABEL)
				fprintf(export_file, ":\n");

			if (current -> type == Token::NASM_CODE)
				fprintf(export_file, "\n");
		}

		current = current -> next;
	}

	fclose(export_file);

	return 0;
}



int Binary::exportObj(const char* filename)
{
	FILE* obj_file = fopen(filename, "w");
	
	Token* current = this -> start;
	
	while (current)
	{
		current = current -> next;
	}
	
	fclose(obj_file);
	
	return 0;
}



int Binary::exportVirtualExecutable(const char* filename)
{
	return 0;
}



int Binary::exportExecutable(const char* filename)
{
	FILE* executable = fopen(filename, "w");
	
	char* compiled = new char[this -> size]{0};
	this -> exportString(compiled);
	
	ELF elf(compiled, this -> size);
	elf.makeHeader();
	
	fwrite(elf.text, sizeof(char), elf.text_length, executable);
	
	fclose(executable);
	
	char* cmd = new char[strlen(filename) + strlen("chmod +x ") + 1];
	sprintf(cmd, "chmod +x %s", filename);
	system(cmd);
	
	return 0;
}



int Binary::exportHex(const char* filename)
{
	FILE* hex = fopen(filename, "w");

	Token* current = this -> start;

	fprintf(hex, "Hex dump of programm\nSize = %ld bytes\n\n", this -> size);

	fprintf(hex, " position |           byte  code           |     nasm  code\n");
	fprintf(hex, "          |                                |\n");

	while (current)
	{
		if (current -> text)
		{
			if (current -> type == Token::LOCAL_LABEL or current -> type == Token::GLOBAL_LABEL or current -> type == Token::FUNCTION_LABEL)
			{
				fprintf(hex, "  %7llu |                                | \n", current -> first_byte_position);
				fprintf(hex, "->%7llu | ", current -> first_byte_position);
			}
			else if (!strncmp(current -> text, "call ", 5) or !strncmp(current -> text, "jmp ", 4) or
					 !strncmp(current -> text, "jz ", 3))
				fprintf(hex, "<-%7llu | ", current -> first_byte_position);
			else
				fprintf(hex, "  %7llu | ", current -> first_byte_position);
		}
		
		else
			fprintf(hex, "  %7llu | ", current -> first_byte_position);
		
		
		
		if (current -> bytes)
		{
			for (int counter = 0; counter < current -> bytes_count; counter++)
			{
				fprintf(hex, "%02x ", (unsigned char) (current -> bytes)[counter]);
				if (counter % 10 == 9)
					fprintf(hex, " |\n  %7llu | ", current -> first_byte_position + counter + 1);
			}
		}

		for (int counter = 0; counter < (10 - (current -> bytes_count) % 10); counter++)
			fprintf(hex, "   ");
		
		fprintf(hex, " | ");

		if (current -> text)
		{
			if (current -> type == Token::NASM_CODE)
				fprintf(hex, "    ");

			if (strstr(current -> text, "%d") != nullptr)
				fprintf(hex, current -> text, current -> ivalue);

			else if (strstr(current -> text, "%f") != nullptr)
				fprintf(hex, current -> text, current -> fvalue);

			else if (strstr(current -> text, "%c") != nullptr)
				fprintf(hex, current -> text, current -> cvalue);

			else if (strstr(current -> text, "%s") != nullptr)
				fprintf(hex, current -> text, current -> svalue);

			else
				fprintf(hex, "%s", current -> text);

			if (current -> type == Token::LOCAL_LABEL or current -> type == Token::GLOBAL_LABEL or current -> type == Token::FUNCTION_LABEL)
				fprintf(hex, ":");
		}
		
		fprintf(hex, "\n");

		current = current -> next;
	}

	fclose(hex);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////


Binary::Token::Token()
{}

Binary::Token::~Token()
{
	//printf("deleting on byte %llu\n", this -> first_byte_position);
	if (this -> text != nullptr)
		delete[] this -> text;
	//DEBUG
	if (this -> bytes != nullptr)
		delete[] this -> bytes;
	//DEBUG
	if (this -> svalue != nullptr)
		delete[] this -> svalue;
	
	//printf("%p\n", this -> next);
	if (this -> next)
		delete this -> next;
	//printf("%p\n", this -> next);
}



void Binary::Token::setText(const char* text)
{
	this -> text = new char[strlen(text) + 1]{0};
	strcpy(this -> text, text);
}

void Binary::Token::setBytes(const char* bytes, long int bytes_count)
{
	this -> bytes = new char[bytes_count]{0};
	this -> bytes_count = bytes_count;
	memcpy(this -> bytes, bytes, bytes_count);
}

void Binary::Token::setSValue(const char* svalue)
{
	if (svalue)
	{
		this -> svalue = new char[strlen(svalue) + 1]{0};
		strcpy(this -> svalue, svalue);
	}
}

int Binary::Token::decompile()
{
	assert("Translating from binary to asm is not implemented yet\n");
	return 0;
}

int Binary::Token::prepare()
{
	if (this -> type == BOTH or this -> type == BYTE_CODE)
		return 0;
	
	else if (this -> type == LOCAL_LABEL or this -> type == GLOBAL_LABEL or this -> type == FUNCTION_LABEL)
	{
		if (strstr(this -> text, "%d") != nullptr)
		{
			char* parsed = new char[strlen(this -> text) + 8]{0};
			sprintf(parsed, this -> text, this -> ivalue);
			delete[] this -> text;
			this -> text = parsed;
		}
	}

	else if ((this -> text)[0] == 'j')
	{
		char* label_name = strchr(this -> text, ' ') + 1;
		if (not !strncmp(label_name, "%s", 2))
		{
			char* new_label_name = new char[strlen(label_name) + 6]{0};

			if (strstr(label_name, "%d"))
				sprintf(new_label_name, label_name, this -> ivalue);

			else
				strcpy(new_label_name, label_name);

			this -> svalue = new_label_name;

			label_name[0] = '%';
			label_name[1] = 's';
			label_name[2] = '\0';
		}
	}

	else
	{
		int i = 0;
		while ((strchr("0123456789", (this -> text)[i]) == nullptr or (i == 0 or (i > 0 and strchr(" +-*[({\t", (this -> text)[i - 1]) == nullptr))) and (this -> text)[i] != '\0')
			i++;

		char* num_string = new char[10]{0};
		int j = 0;
		bool number_founded = false;

		int number_start = i;

		while (strchr("0123456789.", (this -> text)[i]) != nullptr and (this -> text)[i] != '\0')
		{
			number_founded = true;
			num_string[j] = (this -> text)[i];
			i++, j++;
		}

		int number_end = i;

		if (strchr(" +-*])}\t\n\0", (this -> text)[i]) == nullptr)
			number_founded = false;

		if (number_founded)
		{
			this -> ivalue = atoi(num_string);
			(this -> text)[number_start] = '\0';

			char* new_text = new char[strlen(this -> text) + strlen((this -> text) + number_end) + strlen(num_string) + 1]{0};

			strcpy(new_text, this -> text);
			strcat(new_text, "%d");
			strcat(new_text, (this -> text) + number_end);

			delete[] this -> text;
			this -> text = new_text;
		}

		delete[] num_string;
	}

	return 0;
}

int Binary::Token::compile()
{
	if (this -> type == LOCAL_LABEL or this -> type == GLOBAL_LABEL or this -> type == FUNCTION_LABEL or this -> type == BOTH or not this -> text);

	#define ASM(asm_code, data_length, byte_code_length, ...)			 \
			else if (!strcmp(this -> text, asm_code))					  \
			{															   \
				unsigned char byte_code[] = __VA_ARGS__;					\
				this -> bytes = new char[byte_code_length];					 \
				this -> bytes_count = byte_code_length;						  \
				memcpy(this -> bytes, (char*) byte_code, byte_code_length);    \
				this -> type = BOTH;											\
				this -> parameter_length = data_length;							 \
			}																	  \

	#include "../Syntax/asm_syntax.hpp"

	#undef ASM

	else
		printf("\x1b[1;31merror:\x1b[0m can'not compile asm code: \x1b[1m%s\x1b[0m\n", this -> text);

	if (this -> prev)
		this -> first_byte_position = this -> prev -> first_byte_position + this -> prev -> bytes_count;

	this -> container -> size += this -> bytes_count;

	if (this -> text != nullptr)
	{
		if (strstr(this -> text, "%d"))
			memcpy((this -> bytes) + ((this -> bytes_count) - (this -> parameter_length)), &(this -> ivalue), this -> parameter_length);
		
		else if (strstr(this -> text, "%f"))
			memcpy((this -> bytes) + ((this -> bytes_count) - (this -> parameter_length)), &(this -> fvalue), this -> parameter_length);
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////




/*!
 *	@file source.cpp
 *	@brief Исполняемый код класса Source
 */


#include "../Headers/source.hpp"



#define skip_spaces(where)	while (*where == ' ')	\
								(where)++;			\


Source::Source()
{
	this -> source_type = DEFINITION;
	this -> ast = new AST;
}

Source::Source(const char* name)
{
	this -> name = new char[strlen(name) + 1]{0};
	strcpy(this -> name, name);
	
	this -> source_type = this -> getType();

	if (this -> source_type == ERRTYPE)
	{
		printf("\x1b[1;31merror:\x1b[0m file \x1b[1m\'%s\'\x1b[0m has an unsupported extension\n", name);
		this -> status = UNSUPPORTED_FILE_EXTENSION;
	}
	
	this -> ast = new AST;
}

Source::~Source()
{
	if (this -> name != nullptr)
		delete[] this -> name;
		
	if (this -> text != nullptr)
		delete[] this -> text;
	
	if (this -> ast != nullptr)
		delete this -> ast;
}

bool Source::open()
{
	if (this -> source_type == DEFINITION)
	{
		this -> status = OPENNING_PROHIBITED;
		return false;
	}
		
	FILE* source_file = fopen(this -> name, "r");	// Trying to open file
	
	if (not source_file)	// Trying to find file in std library path
	{
		// TODO config reading from Jaul.conf
		char* std_name = new char[strlen(this -> name) + strlen(".std/") + 1]{0};
		strcpy(std_name, ".std/");
		strcat(std_name, this -> name);
		source_file = fopen(std_name, "r");
		delete[] std_name;
	}
	
	if (not source_file)
	{
		printf("\x1b[1;31merror\x1b[0m: file \x1b[1m\'%s\'\x1b[0m not exist\n", this -> name);
		this -> status = FILE_NOT_EXIST;
		return false;
	}
	
	fseek(source_file, 0, SEEK_END);
	this -> text_length = ftell(source_file);
	rewind(source_file);

	this -> text = new char[this -> text_length + 1]{0};
	
	this -> text_pointer = this -> text;

	this -> text_length = fread(this -> text, sizeof(char), this -> text_length, source_file);
	
	fclose(source_file);
	
	return true;
}

short int Source::getType()
{
	const char* extension = strrchr(name, '.');
	
	if (extension == nullptr)
		return ERRTYPE;
	
	else if (!strcmp(extension, ".j"))
		return JAUL_SOURCE;
		
	else if (!strcmp(extension, ".s"))
		return JASM_SOURCE;
		
	else if (!strcmp(extension, ".jo"))
		return JAUL_OBJ;
		
	else if (!strcmp(extension, ".jv"))
		return VIRTUAL_EXECUTABLE;
		
	else
		return ERRTYPE;
}


void Source::print()
{
	if (this -> name)
	{
		char* filename = strrchr(this -> name, '/') != nullptr ? strrchr(this -> name, '/') + 1 : this -> name;
		
		printf("\x1b[1;32m%s\x1b[0m ", filename);
		
		if (this -> name != filename)
		{
			printf ("\x1b[2mfrom ");
			char* path = this -> name;
			while (path != filename)
				putchar(*(path++));
		}
		
		printf("\x1b[0m %s\n", 
			   this -> source_type == JAUL_SOURCE ? "\x1b[34m(JAUL source code)\x1b[0m" : 
			   this -> source_type == JASM_SOURCE ? "\x1b[34m(assembler source code)\x1b[0m" : 
			   this -> source_type == JAUL_OBJ ? "\x1b[33m(JAUL object file)\x1b[0m" : 
			   this -> source_type == VIRTUAL_EXECUTABLE ? "\x1b[33m(JAUL virtual executable)\x1b[0m" : 
			   this -> source_type == DEFINITION ? "\x1b[35mdefinition\x1b[0m" : "\x1b[31m(incorrect file type)\x1b[0m");
	}
		
	if (this -> text and (this -> source_type == JAUL_SOURCE or this -> source_type == JASM_SOURCE))
	{
		printf("-----------------\n%s-----------------\n\n", this -> text);
	}
}



/*----------------------\
| This fragment			|
| need to rewrited		|
| with new logic		|
\----------------------*/
void Source::makeAST()	
{
	
	if (this -> source_type != JAUL_SOURCE)
		return;
	
	int initial_indent = 0;
	
	ASN* entry = ast -> createNode(ASN::ENTRY);
	
	ASN* result = parseBlock(initial_indent, &(this -> text_pointer));
	
	entry -> leftConnect(result);
}


void Source::optimizeAST()
{
	int change_count = 0;
	change_count += inlineFunctions(this -> ast -> head);
	change_count += foldConstants(this -> ast -> head);
	change_count += substituteStatic(this -> ast -> head);
	change_count += foldConstants(this -> ast -> head);
	change_count += deleteUnused(this -> ast -> head);
	change_count += optimizeBuiltInFunctions(this -> ast -> head);
	
	if (change_count != 0)
		this -> optimizeAST();
}


#include "source.optimizations.cpp"		///< Optimization functions stores in separate file to simplifize code


void Source::prepareAST()
{
	this -> splitFunctions();
	this -> enumerateMembers();
}


int Source::calculateIndent(char* text)
{
	int indent = 0;
	
	while (*text == '\t')
	{
		indent++;
		text++;
	}
		
	return indent;
}


bool Source::nextLine(char** _text)
{
	while (**_text != '\0')
	{		
		if (**_text == '\n')
		{
			(*_text)++;
			(this -> current_line)++;
		}
			
		char* line_start = *_text;
		
		bool empty = true;
		
		while (**_text != '\n' and **_text != '\0')
		{
			if (**_text != '\t' and **_text != ' ')
			{
				empty = false;
				break;
			}
			
			(*_text)++;
		}
		
		if (not empty)
		{
			*_text = line_start;
			break;
		}
	}
	

	//char* 
	
	if (**_text == '\0')
		return false;
		
	else
		return true;
}


ASN* Source::parseBlock(int indent, char** _text)
{
	ASN* head = 0;
	ASN* previous = 0;
	
	while (**_text != '\0')
	{
		bool line_founded = nextLine(_text);
		
		int current_indent = calculateIndent(*_text);
		
		if (current_indent > indent)
			printf("\x1b[1;31mIncorrect indent!\x1b[0m\n");

		if (current_indent < indent)
			break;
		
		if (line_founded)
		{
			ASN* newline = ast -> createNode(ASN::LINE);
			newline -> ivalue = this -> current_line;
			newline -> svalue = new char[strlen(this -> name) + 1];
			strcpy(newline -> svalue, this -> name);
			
			char* newline_start = *_text;
			while (*newline_start == '\t')
				newline_start++;
			
			char* newline_end = strchr(newline_start, '\n') != nullptr ? strchr(newline_start, '\n') : strchr(newline_start, '\0');
			
			newline -> line_content = new char[newline_end - newline_start + 1]{};
			strncpy(newline -> line_content, newline_start, newline_end - newline_start + 1);
			
			if (previous)
			{
				previous -> leftConnect(newline);
				previous = newline;
			}
			
			else
			{
				previous = newline;
				head = newline;
			}
			
			ASN* parsed = parseLine(indent, _text);
			
			newline -> rightConnect(parsed);
		}
	}
	
	return head;
}


ASN* Source::parseLine(int indent, char** _text)
{
	while (**_text == '\t')
		(*_text)++;
	
	char* endline = strchr(*_text, '\n');
	if (endline == NULL)
		endline = strchr(*_text, '\0');
		
	int length = endline - *_text;

	char* line = new char[length + 1]{0};
	strncpy(line, *_text, length);
	
	char* _line = line;
	
	ASN* node = getInclude(indent, &_line);
	
	(*_text) += length;
	
	if ((node -> type == ASN::CTRL_OPERATOR and (node -> ivalue == ASN::WHILE or node -> ivalue == ASN::FOR or node -> ivalue == ASN::IF or node -> ivalue == ASN::ELSE)) or node -> type == ASN::FUNC)
	{
		ASN* internal = parseBlock(indent + 1, _text);
		node -> leftConnect(internal);
	}
	
	if (line)
		delete[] line;
	
	
	return node;
}


ASN* Source::getInclude(int indent, char** _line)
{
	
	if (!strncmp(*_line, "include ", 8))
	{
		
		*_line += 8;
		ASN* include = new ASN(ASN::INCLUDE, _line);
		
		char* source_prefix_end = strrchr(this -> name, '/');
		if (source_prefix_end != nullptr)
		{
			int source_prefix_length = source_prefix_end - this -> name + 1;
			char* fixed_name = new char[source_prefix_length + strlen(include -> svalue) + 1]{0};
			strncpy(fixed_name, this -> name, source_prefix_length);
			strcat(fixed_name, include -> svalue);
			
			delete[] include -> svalue;
			include -> svalue = fixed_name;
		}
		
		if (strrchr(include -> svalue, '.') == nullptr)	// TODO add searching algorithm
		{
			char* jaul_library_name = new char[strlen(include -> svalue) + 3];
			sprintf(jaul_library_name, "%s.j", include -> svalue);			
			
			delete[] include -> svalue;
			include -> svalue = jaul_library_name;
		}
		
		Source library(include -> svalue);
		library.open();
		
		if (library.status != OK)
		{
			printf("%s:%d: \x1b[1;31merror:\x1b[0m including file \x1b[1m\"%s\"\x1b[0m not exist.\n", this -> name, this -> current_line, library.name);
			return include;
		}
			
		else
		{
			library.makeAST();
			include -> leftConnect(library.ast -> head -> left);
			include -> rightConnect(library.ast -> head -> right);
			library.ast -> head -> left = 0;
			library.ast -> head -> right = 0;
		}
		
		return include;
	}
	
	else
	{
		
		return getDef(indent, _line);
		
	}
	
}


ASN* Source::getDef(int indent, char** _line)
{
	if (!strncmp(*_line, "def ", 4))
	{
		(*_line) += 4;
		
		ASN* def = ast -> createNode(ASN::FUNC, _line);
		
		(*_line)++;
		
		ASN* itemize = getItemize(indent, _line);
		
		ASN* item = itemize;
		
		while (item)
		{
			item -> right -> LValue = true;
			item = item -> left;
		}
		
		def -> rightConnect(itemize);
		
		return def;
	}
	
	else
	{
		
		return getAssignment(indent, _line);
		
	}
}


ASN* Source::getAssignment(int indent, char** _line)	// get assignment
{	
	skip_spaces(*_line);
	ASN* left_branch = getOperators(indent, _line);
	
	skip_spaces(*_line);
	if (**_line == '=' and *(*_line + 1) != '=')
	{		
		ASN* operand = ast -> createNode(_line);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		ASN* right_branch = getOperators(indent, _line);
			
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


ASN* Source::getOperators(int indent, char** _line)
{	
	ASN* left_branch = 0;

	if (!strncmp(*_line, "while", 5) or !strncmp(*_line, "if", 2) or !strncmp(*_line, "return", 6))
	{
		
		ASN* operand = ast -> createNode(_line);
		
		skip_spaces(*_line);
		ASN* right_branch = getLogic(indent, _line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	else if (!strncmp(*_line, "repeat", 6))
	{
		ASN* operand = ast -> createNode(_line);
		
		skip_spaces(*_line);
		ASN* right_branch = ast -> createNode(ASN::CTRL_OPERATOR);
		right_branch -> ivalue = ASN::ASSIGNMENT;
		ASN* right_branch_value = getNumVarFunc(indent, _line);
		
		skip_spaces(*_line);
		ASN* right_branch_variable = getNumVarFunc(indent, _line);
		
		right_branch -> rightConnect(right_branch_value);
		right_branch -> leftConnect(right_branch_variable);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	else if (!strncmp(*_line, "else", 4))
	{
		
		ASN* operand = ast -> createNode(_line);
		
		skip_spaces(*_line);
		left_branch = operand;		
	}
	
	else
	{
		
		skip_spaces(*_line);
		left_branch = getLogic(indent, _line);
		
		skip_spaces(*_line);
		
	}
	
	return left_branch;
}


ASN* Source::getLogic(int indent, char** _line)
{
	skip_spaces(*_line);
	
	ASN* left_branch = getCmp(indent, _line);
	
	
	skip_spaces(*_line);
	while (!strncmp(*_line, "||", 2) or !strncmp(*_line, "&&", 2))
	{		
		ASN* operand = ast -> createNode(_line);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		ASN* right_branch = getCmp(indent, _line);
			
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


ASN* Source::getCmp(int indent, char** _line)
{
	
	skip_spaces(*_line);
	ASN* left_branch = getAddSub(indent, _line);
	
	skip_spaces(*_line);
	while (!strncmp(*_line, "==", 2) or !strncmp(*_line, "!=", 2) or 
		!strncmp(*_line, ">=", 2) or !strncmp(*_line, "<=", 2) or
		!strncmp(*_line, ">", 1) or !strncmp(*_line, "<", 1))
	{
		ASN* operand = ast -> createNode(_line);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		ASN* right_branch = getAddSub(indent, _line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


ASN* Source::getAddSub(int indent, char** _line)
{
	
	skip_spaces(*_line);
	ASN* left_branch = getMulDiv(indent, _line);
	
	skip_spaces(*_line);
	
	while (**_line == '+' or **_line == '-')
	{
		if (not left_branch)
		{
			left_branch = ast -> createNode(ASN::CONSTANT);
			left_branch -> data_type = ASN::FLOAT;
			left_branch -> fvalue = 0;
		}
		
		ASN* operand = ast -> createNode(_line);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		ASN* right_branch = getMulDiv(indent, _line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


ASN* Source::getMulDiv(int indent, char** _line)
{
	
	skip_spaces(*_line);
	ASN* left_branch = getPow(indent, _line);
	
	skip_spaces(*_line);
	while (**_line == '*' or **_line == '/' or **_line == '%')
	{
		ASN* operand = ast -> createNode(_line);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		ASN* right_branch = getPow(indent, _line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


ASN* Source::getPow(int indent, char** _line)
{
	
	skip_spaces(*_line);
	ASN* left_branch = getNumVarFunc(indent, _line);
	
	skip_spaces(*_line);
	if (**_line == '^')
	{
		ASN* operand = ast -> createNode(_line);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		ASN* right_branch = getNumVarFunc(indent, _line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}

	
ASN* Source::getNumVarFunc(int indent, char** _line)	// get numbers, variables and functions
{
	
	skip_spaces(*_line);
	
	if (**_line == '(')
	{
		(*_line)++;
		
		ASN* block = getLogic(indent, _line);
		
		if (**_line != ')')
		{
			char operand[20] = {};
			for (int i = 0; strchr(" \n\t\0", (*_line)[i]) == nullptr; i++)
				operand[i] = (*_line)[i];
			printf("\x1b[1;31merror:\x1b[0m <%s:%d>: enclosing bracket expected, but \x1b[1m\'%s\'\x1b[0m met\n", this -> name, this -> current_line, operand);
			this -> status = SYNTAX_ERROR;
		}
		
		(*_line)++;
		return block;
	}
	
	skip_spaces(*_line);
	
	if (**_line == '-')
		return 0;
	
	ASN* parsed = ast -> createNode(_line);
	
	if (parsed == nullptr)
	{
		printf("\x1b[1;31merror:\x1b[0m <%s:%d>: supposed operand, but endline met\n", this -> name, this -> current_line);
		this -> status = SYNTAX_ERROR;
	}
	
	else if (parsed -> type != ASN::FUNCCALL and parsed -> type != ASN::VARIABLE and parsed -> type != ASN::CONSTANT)
	{
		printf("\x1b[1;31merror:\x1b[0m <%s:%d>: supposed operand, but operator ", this -> name, this -> current_line);
		parsed -> print();
		printf(" met\n");
		this -> status = SYNTAX_ERROR;
	}
	
	else if (parsed -> type == ASN::CONSTANT and parsed -> data_type == ASN::INT)	///< All numbers switching to floats to simplifize constant folding and show old performance
	{
		parsed -> data_type = ASN::FLOAT;
		parsed -> fvalue = parsed -> ivalue;
		parsed -> ivalue = 0;
	}
	
	skip_spaces(*_line);
	if (**_line == '(')
	{
		(*_line)++;
		ASN* itemize = getItemize(indent, _line);
		
		if (itemize)
			parsed -> rightConnect(itemize);
		
		if (**_line != ')')
		{
			char operand[20] = {};
			for (int i = 0; strchr(" \n\t\0", (*_line)[i]) == nullptr; i++)
				operand[i] = (*_line)[i];
			printf("\x1b[1;31merror:\x1b[0m <%s:%d>: enclosing bracket expected, but \x1b[1m\'%s\'\x1b[0m met\n", this -> name, this -> current_line, operand);
			this -> status = SYNTAX_ERROR;
		}
		
		(*_line)++;
	}
	
	return parsed;
}


ASN* Source::getItemize(int indent, char** _line)
{
	
	skip_spaces(*_line);
	if (**_line == ')')
		return 0;
	
	ASN* item = ast -> createNode(ASN::ITEM);
	
	item -> rightConnect(getLogic(indent, _line));
	
	skip_spaces(*_line);
	
	if (**_line == ',')
	{
		
		(*_line)++;
		skip_spaces(*_line);
		item -> leftConnect(getItemize(indent, _line));
		
	}
	
	return item;
}




void Source::splitFunctions()
{
	if (this -> source_type != JAUL_SOURCE)
		return;
	
	ASN* last_function = 0;

	ASN* line = this -> ast -> head -> left;
	
	while (line)
	{
		if (line -> right -> type == ASN::FUNC)
		{
			ASN* def = line;
			def -> type = ASN::DEF;
			
			(line -> parent) -> leftConnect(line -> left);
			line = line -> left;
			
			def -> left = 0;
			
			if (last_function)
				last_function -> leftConnect(def);
				
			else
				this -> ast -> head -> rightConnect(def);
			
			last_function = def;
		}
		
		
		else if (line -> right -> type == ASN::INCLUDE)
		{
			ASN* deleting = line;
			
			ASN* new_lines = line -> right -> left;
			ASN* new_functions = line -> right -> right;
			line -> right -> left = 0;
			line -> right -> right = 0;
			
			line -> parent -> leftConnect(new_lines);
			
			if (last_function)
				last_function -> leftConnect(new_functions);
				
			else
			{
				this -> ast -> head -> rightConnect(new_functions);
				last_function = new_functions;
			}
			
			if (last_function)
			{
				while (last_function -> left)
					last_function = last_function -> left;
			}
			
			if (new_lines)
			{
				line = new_lines;
				while (line -> left)
					line = line -> left;
				
				line -> leftConnect(deleting -> left);
			}
			
			else
				deleting -> parent -> leftConnect(deleting -> left);
				
			deleting -> left = 0;
			delete deleting;
			
			line = new_lines;
		}
		
		else
			line = line -> left;
	}
	
	ASN* tmp = this -> ast -> head -> left;
	this -> ast -> head -> left = this -> ast -> head -> right;
	this -> ast -> head -> right = tmp;
	
	if (this -> ast -> head -> right)
	{
		ASN* _start = this -> ast -> head -> right;
		this -> ast -> head -> right = 0;
		
		ASN* _start_function_node = this -> ast -> createNode(ASN::_START);
		ASN* _start_def_node = this -> ast -> createNode(ASN::DEF);
		
		_start_def_node -> leftConnect(this -> ast -> head -> left);
		_start_def_node -> rightConnect(_start_function_node);
		_start_function_node -> leftConnect(_start);
		
		this -> ast -> head -> leftConnect(_start_def_node);
	}
}


void Source::enumerateMembers()
{
	ASN* def = this -> ast -> head -> left;
	
	while (def)
	{
		ASN* function = def -> right;
		
		ASN* parameter = function -> right;		
		
		while (parameter)
		{
			setSide(function, parameter -> right -> svalue, ASN::PARAMETER);
			parameter = parameter -> left;
		}
		
		int varcounter = 0;
		while (true)
		{
			char* unnumerated_variable = getUnnumeratedVariable(function, ASN::LOCAL);
			if (not unnumerated_variable)
				break;
			
			enumerateVariables(function, unnumerated_variable, varcounter);
			
			varcounter++;
		}
		
		function -> ivalue = varcounter;
		
		varcounter++;
		
		parameter = function -> right;
		
		while (parameter)
		{			
			enumerateVariables(function, parameter -> right -> svalue, varcounter);
			parameter = parameter -> left;
			varcounter++;
		}
		
		def = def -> left;
	}
	
	int number = 0;
	enumerateBranching(this -> ast -> head, &number);
}



void Source::setFloats()
{
	this -> setFloats(this -> ast -> head);
}


void Source::setFloats(ASN* node)
{
	if (node -> left)
		this -> setFloats(node -> left);
	
	if (node -> right)
		this -> setFloats(node -> right);
	
	if (node -> type == ASN::VARIABLE or node -> type == ASN::ARITHM_OPERATOR or node -> type == ASN::CMP_OPERATOR or
		node -> type == ASN::FUNC or node -> type == ASN::FUNCCALL or
		(node -> type == ASN::CTRL_OPERATOR and (node -> ivalue == ASN::RETURN or node -> ivalue == ASN::ASSIGNMENT)))
		node -> data_type = ASN::FLOAT;
	
	if (node -> type == ASN::FUNCCALL and (!strcmp(node -> svalue, "i_input") or !strcmp(node -> svalue, "i_print")))
	{
		char* old_name = node -> svalue;
		node -> svalue = new char[6]{};
		strcpy(node -> svalue, old_name + 2);
		delete[] old_name;
	}
	
	else if (node -> type == ASN::FUNCCALL and (!strcmp(node -> svalue, "int") or !strcmp(node -> svalue, "float")))
	{
		ASN* parameter = node -> right -> right;
		node -> right -> right = nullptr;
		
		if (node -> parent -> left == node)
			node -> parent -> leftConnect(parameter);
		
		else
			node -> parent -> rightConnect(parameter);
		
		delete node;
	}
}



void Source::setTypes()
{
 	this -> initParameters(this -> ast -> head);
	
	this -> setTypes(this -> ast -> head);
	
 	int modified_count = this -> verifyDeclarationCoincidence();
	
	if (this -> status == SYNTAX_ERROR)
		return;
		
	if (modified_count > 0)
	{
		this -> flushTypes(this -> ast -> head);
		this -> setTypes();
	}
}



int Source::setTypes(ASN* node)
{
	int left_type = ASN::NO_TYPE;
	int right_type = ASN::NO_TYPE;
	
	if (node -> type == ASN::FUNC and node -> data_type != ASN::NO_TYPE)
		return node -> data_type;
	
	if (node -> right)
		right_type = this -> setTypes(node -> right);
	
	if (node -> left)
		left_type = this -> setTypes(node -> left);
	
	if (node -> type == ASN::CONSTANT and node -> data_type == ASN::FLOAT)
	{
		int rounded = round(node -> fvalue);
		float delta = node -> fvalue - rounded;
		if (delta <= APPROXIMATION and delta >= -APPROXIMATION)
		{
			node -> fvalue = 0;
			node -> data_type = ASN::INT;
			node -> ivalue = rounded;
		}
	}
	
	else if (node -> type == ASN::FUNCCALL)
	{
		if (!strcmp(node -> svalue, "input") or !strcmp(node -> svalue, "float")
			or !strcmp(node -> svalue, "sin") or !strcmp(node -> svalue, "cos")
			or !strcmp(node -> svalue, "ctg") or !strcmp(node -> svalue, "tg")
			or !strcmp(node -> svalue, "sqr") or !strcmp(node -> svalue, "sqrt")
			or !strcmp(node -> svalue, "abs"))
			node -> data_type = ASN::FLOAT;
		
		else if (!strcmp(node -> svalue, "i_input") or !strcmp(node -> svalue, "int"))
			node -> data_type = ASN::INT;
		
		else if (!strcmp(node -> svalue, "i_print") or !strcmp(node -> svalue, "print"))
		{
			delete[] node -> svalue;
			node -> svalue = new char[strlen("err_print") + 1]{};
			
			if (node -> right -> right -> data_type == ASN::INT)
				strcpy(node -> svalue, "i_print");
			
			else if (node -> right -> right -> data_type == ASN::FLOAT)
				strcpy(node -> svalue, "print");
			
			else
				strcpy(node -> svalue, "err_print");
		}
		
		else
			node -> data_type = this -> getFunctionType(node);
	}
	
	else if (node -> type == ASN::ARITHM_OPERATOR or node -> type == ASN::CMP_OPERATOR)
	{
		if (((left_type == ASN::INT and right_type == ASN::INT)
			or node -> ivalue == ASN::INT_DIVISION or (node -> ivalue >= ASN::EQUAL and node -> ivalue <= ASN::LESS_EQ)) 
			and node -> ivalue != ASN::DIVIDE)
			node -> data_type = ASN::INT;
		
		else if (left_type == ASN::FLOAT or right_type == ASN::FLOAT
			or node -> ivalue == ASN::DIVIDE)
			node -> data_type = ASN::FLOAT;
	}
	
	else if (node -> type == ASN::CTRL_OPERATOR and node -> ivalue == ASN::RETURN)
	{
		node -> data_type = node -> right -> data_type;
		this -> getFunction(node) -> data_type = node -> data_type;
	}
	
	if (node -> type == ASN::CTRL_OPERATOR and node -> ivalue == ASN::ASSIGNMENT)
	{
		if (node -> data_type == ASN::NO_TYPE)
			node -> data_type = node -> right -> data_type;
				
		if (node -> left -> data_type != ASN::NO_TYPE and node -> left -> data_type != node -> data_type)
		{
			ASN* line = getLine(node);
			printf("\x1b[1;31merror:\x1b[0m <%s:%d>: casting from \x1b[1m%s\x1b[0m to \x1b[1m%s\x1b[0m in branching is prohibited by the language standart\n", this -> name, line -> ivalue,
				   node -> data_type == ASN::INT ? "int" : node -> data_type == ASN::FLOAT ? "float" : "n/t",
				   node -> left -> data_type == ASN::INT ? "int" : node -> left -> data_type == ASN::FLOAT ? "float" : "n/t");
			printf("        -> %s\n", line -> line_content);
			
			//this -> status = SYNTAX_ERROR;
		}
		
		else
			node -> left -> data_type = node -> data_type;
		
		if (node -> data_type != ASN::NO_TYPE)
			setVariablesTypes(node -> parent -> left, node -> left, DEFAULT);
	}
	
	if (node -> type == ASN::VARIABLE and node -> vartype == ASN::PARAMETER and node -> LValue == true and node -> parent -> type == ASN::ITEM)
	{
		ASN* function = node;
		while (function -> type != ASN::FUNC)
			function = function -> parent;
		
		setVariablesTypes(function -> left, node, DEFAULT);
	}
	
	return node -> data_type;
}


int Source::setVariablesTypes(ASN* node, ASN* variable, int mode)
{	
	if (node == nullptr)
		return 0;
	
	if (node -> type == ASN::VARIABLE and node -> LValue == true and !strcmp(node -> svalue, variable -> svalue) and mode == DEFAULT)
		return -1;
	
	if (node -> right)
	{
		int status = this -> setVariablesTypes(node -> right, variable, mode);
		
		if (status == -1)
			return -1;
	}
	
	if (node -> type == ASN::VARIABLE and (node -> LValue == false or mode == FORCE) and !strcmp(node -> svalue, variable -> svalue))
	{
		node -> data_type = variable -> data_type;
	}
	
	if (node -> left)
	{
		int status = 0;
		if (node -> type == ASN::CTRL_OPERATOR and 
			(node -> ivalue == ASN::IF or node -> ivalue == ASN::ELSE or node -> ivalue == ASN::WHILE or node -> ivalue == ASN::FOR))
			this -> setVariablesTypes(node -> left, variable, FORCE);
		
		else
			status = this -> setVariablesTypes(node -> left, variable, mode);
		
		if (status == -1)
			return -1;
	}
	
	return 0;
}



void Source::initParameters(ASN* node)
{
	if (node -> left)
		this -> initParameters(node -> left);
	
	if (node -> right)
		this -> initParameters(node -> right);
	
	if (node -> type == ASN::VARIABLE and node -> vartype == ASN::PARAMETER and node -> LValue == true and node -> data_type == ASN::NO_TYPE and node -> parent -> type == ASN::ITEM)
		node -> data_type = ASN::INT;
}



int Source::verifyDeclarationCoincidence()
{
	int redeclarations_count = 0;
	
	ASN* current_def = this -> ast -> head -> left;
	
	while (current_def)
	{
		redeclarations_count += this -> verifyDeclarationCoincidence(this -> ast -> head, current_def -> right);
		current_def = current_def -> left;
	}
	
	return redeclarations_count;
}


int Source::verifyDeclarationCoincidence(ASN* node, ASN* declaration)
{
	int redeclarations_count = 0;
	
	if (node -> left)
		redeclarations_count += this -> verifyDeclarationCoincidence(node -> left, declaration);
	
	if (node -> right)
		redeclarations_count += this -> verifyDeclarationCoincidence(node -> right, declaration);
	
	if (node -> type == ASN::FUNCCALL and !strcmp(node -> svalue, declaration -> svalue))
	{
		node -> bounded_node = declaration;
		
		ASN* decl_parameter = declaration -> right;
		int decl_parameter_count = 0;
		
		ASN* call_parameter = node -> right;
		int call_parameter_count = 0;
		
		while (decl_parameter and call_parameter)
		{
			if (call_parameter -> right -> data_type == ASN::FLOAT and decl_parameter -> right -> data_type == ASN::INT)
			{
				decl_parameter -> right -> data_type = ASN::FLOAT;
				redeclarations_count++;
			}
			
			decl_parameter = decl_parameter -> left;
			call_parameter = call_parameter -> left;
			decl_parameter_count++;
			call_parameter_count++;
		}
		
		while (decl_parameter)
		{
			decl_parameter = decl_parameter -> left;
			decl_parameter_count++;
		}
		
		while (call_parameter)
		{
			call_parameter = call_parameter -> left;
			call_parameter_count++;
		}
		
		if (call_parameter_count != decl_parameter_count)
		{
			printf("\x1b[1;31merror:\x1b[0m <%s:%d>: function \x1b[1m%s\x1b[0m assumes %d parameters, but %d given\n", getLine(node) -> svalue, 
					getLine(node) -> ivalue, declaration -> svalue, decl_parameter_count, call_parameter_count);
			printf("\t-> %s\n", getLine(node) -> line_content);
		
			this -> status = SYNTAX_ERROR;
		}
		
	}
	
	return redeclarations_count;
}



void Source::flushTypes(ASN* node)
{
	if (node -> left)
		this -> flushTypes(node -> left);
	
	if (node -> right)
		this -> flushTypes(node -> right);
	
	if (not 
		(node -> type == ASN::CONSTANT or 
		(node -> type == ASN::VARIABLE and node -> vartype == ASN::PARAMETER and node -> LValue == true and node -> parent -> type == ASN::ITEM)))
		node -> data_type = ASN::NO_TYPE;
}




int Source::getFunctionType(ASN* node)
{
	ASN* current_def = this -> ast -> head -> left;
	
	while (current_def)
	{
		if (!strcmp(node -> svalue, current_def -> right -> svalue))
		{
			if (current_def -> right -> data_type != ASN::NO_TYPE)
				return current_def -> right -> data_type;
			
			else
			{
				this -> setTypes(current_def -> right);
				return current_def -> right -> data_type;
			}
			
			break;
		}
		current_def = current_def -> left;
	}
	
	return ASN::NO_TYPE;
}




void Source::enumerateBranching(ASN* node, int* number)
{
	if (node -> type == ASN::CTRL_OPERATOR and (node -> ivalue == ASN::IF or node -> ivalue == ASN::WHILE or node -> ivalue == ASN::FOR))
	{
		node -> vartype = *number;
		(*number)++;
	}
	
	if (node -> right)
		enumerateBranching(node -> right, number);
		
	if (node -> left)
		enumerateBranching(node -> left, number);
}



void Source::setSide(ASN* node, const char* varname, int vartype)
{
	if (varname == nullptr)
		return;
	
	if (node -> type == ASN::VARIABLE and node -> svalue)
	{
		if (!strcmp(node -> svalue, varname))
		{
			node -> vartype = vartype;
		}
	}
	
	if (node -> right)
		setSide(node -> right, varname, vartype);
	
	if (node -> left)
		setSide(node -> left, varname, vartype);
	
	return;
}



char* Source::getUnnumeratedVariable(ASN* node, int vartype)
{
	char* unnumerated_variable = nullptr;
	
	if (node -> type == ASN::VARIABLE and node -> ivalue == 0 and node -> vartype == vartype and (not node -> enumerated))
	{
		unnumerated_variable = node -> svalue;
	}
		
	if (node -> right and not unnumerated_variable)
		unnumerated_variable = getUnnumeratedVariable(node -> right, vartype);
		
	if (node -> left and not unnumerated_variable)
		unnumerated_variable = getUnnumeratedVariable(node -> left, vartype);	
		
	return unnumerated_variable;
}



void Source::enumerateVariables(ASN* node, const char* varname, int varnumber)
{
	if (varname == nullptr)
		return;
	
	if (node -> type == ASN::VARIABLE and node -> svalue)
	{
		if (!strcmp(node -> svalue, varname))
		{
			node -> ivalue = varnumber;
			node -> enumerated = true;
		}
	}
	
	if (node -> right)
		enumerateVariables(node -> right, varname, varnumber);
	
	if (node -> left)
		enumerateVariables(node -> left, varname, varnumber);
	
	return;
}



ASN* Source::getLine(ASN* node)
{
	while (node -> type != ASN::LINE and node -> parent != nullptr)
		node = node -> parent;
	
	if (node -> type == ASN::LINE)
		return node;
	
	else
		return nullptr;
}


ASN* Source::getFunction(ASN* node)
{
	while (node -> type != ASN::FUNC and node -> parent != nullptr)
		node = node -> parent;
	
	if (node -> type == ASN::FUNC)
		return node;
	
	else
		return nullptr;
}



void Source::dumpAST(const char* title)
{
	ast -> dumper(this -> name, AST::DELETE_TXT, title);
	printf("\x1b[1m%s:\x1b[0m Programm tree dump saved as \x1b[1;32m<%s.png>\x1b[0m\n", this -> name, this -> name);
}




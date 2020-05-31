#include "../Headers/programm.hpp"


Programm::Programm()
{
	text = 0;
	text_length = 0;
	programm_tree.setType(DATA_POINTER);
}


Programm::~Programm()
{
	if (this -> text)
		delete[] text;
}




void Programm::readSource(const Settings* settings)
{
	char* source_path = settings -> source_path;
	
	if (strcmp(source_path + (strlen(source_path) - 5), ".jaul"))
	{
		printf("\x1b[1;31mError\x1b[0m: file \x1b[1m\"%s\"\x1b[0m extension not supported.\n", source_path);
		exit(WRONG_EXTENSION);
	}
	
	FILE* source_file = fopen(source_path, "r");
	
	if (not source_file)
	{
		printf("\x1b[1;31mError\x1b[0m: file \x1b[1m\"%s\"\x1b[0m not exist.\n", source_path);
		exit(FILE_NOT_EXIST);
	}
	
	fseek(source_file, 0, SEEK_END);
	this -> text_length = ftell(source_file);
	rewind(source_file);

	this -> text = new char[this -> text_length + 1]{0};

	this -> text_length = fread(this -> text, sizeof(char), this -> text_length, source_file);
	
	fclose(source_file);

	if (not settings -> silent)
		printf("\n\x1b[1;32m%lld\x1b[0;32m symbols readed from \x1b[1;32m%s\x1b[0m\n------------ \x1b[1;32mSOURCE CODE\x1b[0m ------------\n%s\n------------- \x1b[1;32mEND SOURCE\x1b[0m ------------\n\n", this -> text_length, source_path, this -> text);
}


void Programm::preprocessor(const Settings* settings)
{
	while (importLibraries());

	setDefinitions();

	deleteComments();
	
	if (not settings -> silent)
		printf("\n\x1b[1;32m%lld\x1b[0;32m symbols preprocessed\n------------ \x1b[1;32mSOURCE CODE\x1b[0m ------------\n%s\n------------- \x1b[1;32mEND SOURCE\x1b[0m ------------\n\n", this -> text_length, this -> text);
}


int Programm::importLibraries()
{
	int imports_count = strcount(this -> text, "@import");

	if (imports_count == 0)
		return 0;

	Library* libraries = new Library[imports_count];

	int new_length = this -> text_length;
	char* _text = this -> text;

	for (int counter = 0; counter < imports_count; counter++)
	{
		_text = strstr(_text, "@import") + 8;
		(libraries[counter]).libname_length = std::min(strchr(_text, ' '), strchr(_text, '\n')) - _text;
		(libraries[counter]).libname = new char[(libraries[counter]).libname_length + 1]{0};
		strncpy((libraries[counter]).libname, _text, (libraries[counter]).libname_length);
		(libraries[counter]).readLibrary();
		new_length = new_length - (8 + (libraries[counter]).libname_length) + (libraries[counter]).libtext_length;
	}

	this -> text_length = new_length;
	char* old_text = this -> text;
	char* _old_text = old_text;
	this -> text = new char[this -> text_length + 1]{0};
	_text = this -> text;
	
	int lib_counter = 0;
	
	while (*_old_text)
	{
		if (!strncmp(_old_text, "@import", 7))
		{
			strcpy(_text, (libraries[lib_counter]).libtext);
			_text += (libraries[lib_counter]).libtext_length;
			_old_text += (8 + (libraries[lib_counter]).libname_length);
			lib_counter++;
		}
		
		else
		{
			*_text = *_old_text;
			_text++;
			_old_text++;
		}
	}
	
	delete[] old_text;
	
	delete[] libraries;			// TODO add recursive including check
	
	return imports_count;
}


int Programm::setDefinitions()		// TODO fix replacing middle of word (word borders checking)
{
	int definitions_count = strcount(this -> text, "@define");
	if (definitions_count == 0)
		return 0;
		
	Definition* definitions = new Definition[definitions_count];
	
	int new_length = this -> text_length;
	char* _text = this -> text;
	
	for (int counter = 0; counter < definitions_count; counter++)
	{
		_text = strstr(_text, "@define") + 8;
		(definitions[counter]).defname_length = std::min(strchr(_text, ' '), strchr(_text, '\n')) - _text;
		
		(definitions[counter]).defname = new char[(definitions[counter]).defname_length + 1]{0};
		strncpy((definitions[counter]).defname, _text, (definitions[counter]).defname_length);
		
		_text += (definitions[counter]).defname_length + 1;
		(definitions[counter]).defstatement_length = strchr(_text, '\n') - _text;
		
		(definitions[counter]).defstatement = new char[(definitions[counter]).defstatement_length + 1]{0};
		strncpy((definitions[counter]).defstatement, _text, (definitions[counter]).defstatement_length);
		
		new_length = new_length + (((definitions[counter]).defstatement_length - (definitions[counter]).defname_length) \
									* (strcount(this -> text, (definitions[counter]).defname) - 1) - \
									((definitions[counter]).defstatement_length + (definitions[counter]).defname_length) + 9);
	}
	
	this -> text_length = new_length;
	char* old_text = this -> text;
	char* _old_text = old_text;
	this -> text = new char[this -> text_length + 1]{0};
	_text = this -> text;

	while (*_old_text)
	{
		if (!strncmp(_old_text, "@define", 7))
		{
			while(*_old_text != '\n')
				_old_text++;
		}
		
		bool defstatement_placed = false;
		for (int counter = 0; counter < definitions_count; counter++)
		{
			if (!strncmp(_old_text, definitions[counter].defname, definitions[counter].defname_length))
			{
				strncpy(_text, definitions[counter].defstatement, definitions[counter].defstatement_length);
				_text += definitions[counter].defstatement_length;
				_old_text += definitions[counter].defname_length;
				defstatement_placed = true;
				break;
			}
		}
		
		if (not defstatement_placed)
		{
			*_text = *_old_text;
			_text++;
			_old_text++;
		}
	}
	
	delete[] old_text;

	delete[] definitions;
	
	return definitions_count;
}


int Programm::deleteComments()
{
	char* _text = this -> text;
	char* _old_text = this -> text;
	
	bool in_comment = false;
	while (*_old_text)
	{
		if (*_old_text == '#')
			in_comment = true;
		
		if (*_old_text == '\n')
			in_comment = false;
			
		if (in_comment)
			_old_text++;
			
		else
		{
			*_text = *_old_text;
			_text++;
			_old_text++;
		}
	}
	
	*_text = '\0';
	
	return 0;
}


void Programm::makeTree(const Settings* settings)
{
	int initial_indent = 0;
	char* _text = this -> text;
	Token* token = new Token(Token::ENTRY);
	PNode* entry = (this -> programm_tree).createNode(token);
	PNode* result = parseBlock(initial_indent, &_text);
	entry -> leftConnect(result);
	this -> rebuildTree();
	if (not settings -> silent)
	{
		(this -> programm_tree).dumper(settings -> source_path, DELETE_TXT + DETAILED, colorize);
		printf("Programm tree dump saved as \x1b[1;32m<%s.png>\x1b[0m\n", settings -> source_path);
	}
}


const char* colorize(Token* token)
{
	if (token -> type == Token::ENTRY)
		return "green";
		
	else if (token -> type == Token::LINE or token -> type == Token::ITEM or token -> type == Token::DEF)
		return "lightgray";
		
	else if (token -> type == Token::FUNC)
		return "green";
	
	else if (token -> type == Token::ARITHM_OPERATOR)
		return "pink";
		
	else if (token -> type == Token::CMP_OPERATOR)
		return "\"#f8aeff\"";
		
	else if (token -> type == Token::CTRL_OPERATOR)
		return "orange";
		
	else if (token -> type == Token::INT)
		return "\"#D083FF\"";
		
	else if (token -> type == Token::CHAR)
		return "\"#B3FFEF\"";
		
	else if (token -> type == Token::FLOAT)
		return "\"#FF89A9\"";
		
	else if (token -> type == Token::STRING)
		return "\"#9FFF88\"";
		
	else if (token -> type == Token::VARIABLE)
		return "\"#64ADFF\"";
		
	else if (token -> type == Token::FUNCCALL)
		return "\"#0B8AFF\"";
		
	else
		return "lightblue";
}


PNode* parseBlock(int indent, char** _text)
{
	PNode* head = 0;
	PNode* previous = 0;

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
			Token* token = new Token(Token::LINE);
			PNode* newline = new PNode(token);
			
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
			
			PNode* parsed = parseLine(indent, _text);
			newline -> rightConnect(parsed);
		}
	}

	return head;
}


bool nextLine(char** _text)
{
	while (**_text != '\0')
	{		
		if (**_text == '\n')
			(*_text)++;
			
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
	
	if (**_text == '\0')
		return false;
		
	else
		return true;
}


int calculateIndent(char* text)
{
	int indent = 0;
	
	while (*text == '\t')
	{
		indent++;
		text++;
	}
		
	return indent;
}


PNode* parseLine(int indent, char** _text)
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
	
	PNode* node = 0;
	
	if (!strncmp(line, "def ", 4))
		node = getDef(&_line);
	
	else
		node = getAssignment(&_line);

	
	(*_text) += length;
		
	if ((node -> data -> type == Token::CTRL_OPERATOR and (node -> data -> ivalue == Token::WHILE or node -> data -> ivalue == Token::IF or node -> data -> ivalue == Token::ELSE)) or node -> data -> type == Token::FUNC)
	{
		PNode* internal = parseBlock(indent + 1, _text);
		node -> leftConnect(internal);
	}
	
	if (line)
		delete[] line;

	return node;
}


PNode* getDef(char** _line)
{
	(*_line) += 4;
	
	Token* token = new Token(Token::FUNC, _line);
	
	PNode* def = new PNode(token);
	
	(*_line)++;
	def -> rightConnect(getItemize(_line));
	
	return def;
}


PNode* getAssignment(char** _line)	// get assignment
{
	skip_spaces(*_line);
	PNode* left_branch = getOperators(_line);
	
	skip_spaces(*_line);
	if (**_line == '=' and *(*_line + 1) != '=')
	{
		Token* token = new Token(_line);
		
		PNode* operand = new PNode(token);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		PNode* right_branch = getOperators(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


PNode* getOperators(char** _line)
{
	PNode* left_branch = 0;

	if (!strncmp(*_line, "while", 5) or !strncmp(*_line, "if", 2) or !strncmp(*_line, "for", 4) or !strncmp(*_line, "return", 6))
	{
		Token* token = new Token(_line);
		
		PNode* operand = new PNode(token);
		
		skip_spaces(*_line);
		PNode* right_branch = getLogic(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	else if (!strncmp(*_line, "else", 4))
	{
		Token* token = new Token(_line);
		
		PNode* operand = new PNode(token);
		
		skip_spaces(*_line);
		left_branch = operand;		
	}
	
	else
	{
		skip_spaces(*_line);
		left_branch = getLogic(_line);
		
		skip_spaces(*_line);
	}
	
	return left_branch;
}


PNode* getLogic(char** _line)
{
	skip_spaces(*_line);
	PNode* left_branch = getCmp(_line);
	
	skip_spaces(*_line);
	while (!strncmp(*_line, "||", 2) or !strncmp(*_line, "&&", 2))
	{
		Token* token = new Token(_line);
		
		PNode* operand = new PNode(token);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		PNode* right_branch = getCmp(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


PNode* getCmp(char** _line)
{
	skip_spaces(*_line);
	PNode* left_branch = getAddSub(_line);
	
	skip_spaces(*_line);
	while (!strncmp(*_line, "==", 2) or !strncmp(*_line, "!=", 2) or 
		!strncmp(*_line, ">=", 2) or !strncmp(*_line, "<=", 2) or
		!strncmp(*_line, ">", 1) or !strncmp(*_line, "<", 1))
	{
		Token* token = new Token(_line);

		PNode* operand = new PNode(token);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		PNode* right_branch = getAddSub(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


PNode* getAddSub(char** _line)
{
	skip_spaces(*_line);
	PNode* left_branch = getMulDiv(_line);
	
	skip_spaces(*_line);
	
	while (**_line == '+' or **_line == '-')
	{
		if (not left_branch)
		{
			Token* token = new Token;
			token -> type = Token::INT;
			token -> ivalue = 0;
			
			left_branch = new PNode(token);
		}
		
		Token* token = new Token(_line);
		
		PNode* operand = new PNode(token);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		PNode* right_branch = getMulDiv(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


PNode* getMulDiv(char** _line)
{
	skip_spaces(*_line);
	PNode* left_branch = getPow(_line);
	
	skip_spaces(*_line);
	while (**_line == '*' or **_line == '/' or **_line == '%')
	{
		Token* token = new Token(_line);
		
		PNode* operand = new PNode(token);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		PNode* right_branch = getPow(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}


PNode* getPow(char** _line)
{
	skip_spaces(*_line);
	PNode* left_branch = getNumVarFunc(_line);
	
	skip_spaces(*_line);
	if (**_line == '^')
	{
		Token* token = new Token(_line);

		PNode* operand = new PNode(token);
		operand -> leftConnect(left_branch);
		
		skip_spaces(*_line);
		PNode* right_branch = getNumVarFunc(_line);
		
		operand -> rightConnect(right_branch);
		left_branch = operand;
	}
	
	return left_branch;
}

	
PNode* getNumVarFunc(char** _line)	// get numbers, variables and functions
{
	skip_spaces(*_line);
	
	if (**_line == '(')
	{
		(*_line)++;
		PNode* block = getLogic(_line);
		if (**_line != ')')
			std::cout << "Error!\n";
		(*_line)++;
		return block;
	}
		
	skip_spaces(*_line);
	
	if (**_line == '-')
		return 0;
		
	Token* token = new Token(_line);
	
	PNode* parsed = new PNode(token);
	
	skip_spaces(*_line);
	if (**_line == '(')
	{
		(*_line)++;
		PNode* itemize = getItemize(_line);
		if (itemize)
			parsed -> rightConnect(itemize);
		if (**_line != ')')
			std::cout << "Error!\n";
		(*_line)++;
	}
	
	return parsed;
}


PNode* getItemize(char** _line)
{
	skip_spaces(*_line);
	if (**_line == ')')
		return 0;
	
	Token* token = new Token(Token::ITEM);
	PNode* item = new PNode(token);
	item -> rightConnect(getLogic(_line));
	
	skip_spaces(*_line);
	if (**_line == ',')
	{
		(*_line)++;
		item -> leftConnect(getItemize(_line));
	}
	
	return item;
}




void Programm::optimizeTree(const Settings* settings)
{
	std::cout << "\x1b[1;31mTree optimization is not implemented yet!\x1b[0m\n";
}



void Programm::rebuildTree()
{
	PNode* last_function = 0;
	PNode* current_line = (this -> programm_tree).head;
	
	PNode* deleting = 0;
	while (current_line -> left)
	{
		current_line = current_line -> left;
		
		if (deleting)
		{
			deleting -> left = 0;
			deleting -> right = 0;
			delete deleting;
			deleting = 0;
		}
		
		if (current_line -> right -> data -> type == Token::FUNC)
		{
			PNode* function = current_line -> right;
			deleting = current_line;

			if (current_line -> left)
				current_line -> parent -> leftConnect(current_line -> left);
			else
				current_line -> parent -> left = 0;
				
			Token* def = new Token(Token::DEF);
			PNode* def_node = new PNode(def);
			def_node -> rightConnect(function);

			if (not last_function)
			{
				(this -> programm_tree).head -> rightConnect(def_node);
				last_function = def_node;
			}
			
			else
			{
				last_function -> leftConnect(def_node);
				last_function = def_node;
			}
		}
	}
	
	if ((this -> programm_tree).head -> left == 0)
	{
		std::cout << "\x1b[1mThis is not an executable programm\nAborting...\n\x1b[0m";
		exit(1);
	}
	
	PNode* _start = (this -> programm_tree).head -> left;
	(this -> programm_tree).head -> left = 0;
	
	PNode* defs = (this -> programm_tree).head -> right;
	(this -> programm_tree).head -> right = 0;

	Token* _start_def_token = new Token(Token::DEF);
	Token* _start_func_token = new Token(Token::_START);

	PNode* _start_def = new PNode(_start_def_token);
	PNode* _start_func = new PNode(_start_func_token);

	(this -> programm_tree).head -> leftConnect(_start_def);

	_start_def -> leftConnect(defs);

	_start_def -> rightConnect(_start_func);

	_start_func -> leftConnect(_start);
	
	PNode* def = (this -> programm_tree).head -> left;
	
	while (def)
	{
		PNode* function = def -> right;
		
		PNode* parameter = function -> right;		
		
		int varcounter = -1;
		while (parameter)
		{			
			varcounter++;
			setVariables(function, parameter -> right -> data -> svalue, PARAMETER, varcounter);
			parameter = parameter -> left;
		}
		
		
		varcounter = -1;
		while (true)
		{
			char* unnumerated_variable = getUnnumeratedVariable(function);
			if (not unnumerated_variable)
				break;
				
			varcounter++;
			
			setVariables(function, unnumerated_variable, LOCAL, varcounter);
		}
		function -> data -> ivalue = varcounter + 1;
		def = def -> left;
	}
	
	int number = 0;
	enumerateBranching((this -> programm_tree).head, &number);
}



void enumerateBranching(PNode* node, int* number)
{
	if (node -> data -> type == Token::CTRL_OPERATOR and (node -> data -> ivalue == Token::IF or node -> data -> ivalue == Token::WHILE or node -> data -> ivalue == Token::FOR))
	{
		node -> data -> vartype = *number;
		(*number)++;
	}
	
	if (node -> right)
		enumerateBranching(node -> right, number);
		
	if (node -> left)
		enumerateBranching(node -> left, number);
}



char* getUnnumeratedVariable(PNode* node)
{
	char* unnumerated_variable = 0;
	
	if (node -> data -> type == Token::VARIABLE and node -> data -> ivalue == 0 and node -> data -> vartype == 0)
		unnumerated_variable = node -> data -> svalue;
		
	if (node -> right and not unnumerated_variable)
		unnumerated_variable = getUnnumeratedVariable(node -> right);
		
	if (node -> left and not unnumerated_variable)
		unnumerated_variable = getUnnumeratedVariable(node -> left);	
		
	return unnumerated_variable;
}



void setVariables(PNode* node, const char* varname, int vartype, int varnumber)
{
	if (varname == 0)
		return;
	
	if (node -> data -> type == Token::VARIABLE and node -> data -> svalue)
	{
		if (!strcmp(node -> data -> svalue, varname))
		{
			node -> data -> vartype = vartype;
			node -> data -> ivalue = varnumber;
		}
	}
	
	if (node -> right)
		setVariables(node -> right, varname, vartype, varnumber);
	
	if (node -> left)
		setVariables(node -> left, varname, vartype, varnumber);
	
	return;
}



void Programm::makeNasm(const Settings* settings)
{
	bool nasm_compilation = false;
	if (settings -> nasm_listing)
		nasm_compilation = true;
		
	delete[] this -> text;
	this -> text = 0;
	this -> text_length = 0;
	
	PNode* current_node = (this -> programm_tree).head -> left;
	
	if (nasm_compilation)
		this -> add("section .text\n\tglobal _start\n\n");
	
	while (current_node)
	{
		compileDef(current_node -> right, nasm_compilation);
		current_node = current_node -> left;
	}
	
	char* print_text = 0;
	char* input_text = 0;
	
	if (nasm_compilation)
	{
		print_text = readFile("AsmLibraries/print.s");
		input_text = readFile("AsmLibraries/input.s");
		
		this -> add(print_text);
		this -> add(input_text);
	}
	
	else
	{
		print_text = readFile("AsmLibraries/print");
		input_text = readFile("AsmLibraries/input");
		
		setLabelPosition("print", this -> text_length);
		this -> addBin(print_text);
		
		setLabelPosition("input", this -> text_length);
		this -> addBin(input_text);
	}
	
	if (print_text)
		delete[] print_text;
		
	if (input_text)
		delete[] input_text;
		
	if (not settings -> silent)
		printf("\nCompiled \x1b[1m%d\x1b[0m bytes from \x1b[1;32m<%s>\x1b[0m\n\n", this -> text_length, settings -> source_path);
	
	if (not nasm_compilation)
		this -> makeHeader();
}



void Programm::compileDef(PNode* node, bool nasm_compilation)
{
	if (nasm_compilation)
	{
		this -> add(node -> data -> svalue);
		this -> add(":\n");
		
		this -> add("\tpop r13\n");
		this -> add("\tmov r15, rsp\n");			// r15 - parameters
		this -> add("\tsub rsp, ");
		this -> add((node -> data -> ivalue) * 8);
		this -> add("\n");
		
		this -> add("\tmov r14, rsp\n\n");			// r14 - local variables
		
		makeBody(node -> left, nasm_compilation);
		
		this -> add("\tadd rsp, ");
		this -> add((node -> data -> ivalue) * 8);
		this -> add("\n");
		
		this -> add("\tpush r13\n");
		if (!strcmp(node -> data -> svalue, "_start"))
			this -> add("\tmov rdi, rax\n\tmov rax, 60\n\tsyscall\n\n\n");
			
		else
			this -> add("\tret\n\n\n");
	}
	
	else
	{
		this -> setLabelPosition(node -> data -> svalue, this -> text_length);
		
		this -> addBin("41 5d 49 89 e7 48 83 ec");
		this -> addBin(1, (node -> data -> ivalue) * 8);
		this -> addBin("49 89 e6");
		
		makeBody(node -> left, nasm_compilation);
		
		this -> addBin("48 83 c4");
		this -> addBin(1, (node -> data -> ivalue) * 8);
		this -> addBin("41 55");
		
		if (!strcmp(node -> data -> svalue, "_start"))
			this -> addBin("48 89 c7 b8 3c 00 00 00 0f 05");
			
		else
			this -> addBin("c3");
	}
}



void Programm::makeBody(PNode* node, bool nasm_compilation)
{
	int type = node -> data -> type;
	int optype = node -> data -> ivalue;
	
	if (type == Token::FUNCCALL)
	{
		if (nasm_compilation)
			this -> add("\tpush r13\n\tpush r14\n\tpush r15\n\n");
		else
			this -> addBin("41 55 41 56 41 57");
		
		if (node -> right)
			pushParameters(node -> right, nasm_compilation);
		
		if (nasm_compilation)
		{
			this -> add("\tcall ");
			this -> add(node -> data -> svalue);
			this -> add("\n");
		}
		
		else
		{
			this -> addBin("e8"); 
			int label_position = getLabelPosition(node -> data -> svalue, this -> text_length);
			this -> addBin(4, label_position);
		}
		
		PNode* param = node -> right;
		
		while (param)
		{
			if (nasm_compilation)
				this -> add("\tpop r15\n");
				
			else
				this -> addBin("41 5f");
				
			param = param -> left;
		}
		
		if (nasm_compilation)
			this -> add("\tpop r15\n\tpop r14\n\tpop r13\n\n\tpush rax\n");
		else
			this -> addBin("41 5f 41 5e 41 5d 50");
	}
	
	else if (optype == Token::WHILE and type == Token::CTRL_OPERATOR)
	{
		if (nasm_compilation)
		{
			this -> add(".cycle");
			this -> add(node -> data -> vartype);
			this -> add(":\n");
		}
		
		else
		{
			char* label_name = makeLable(".cycle", node -> data -> vartype);
			setLabelPosition(label_name, this -> text_length);
			delete[] label_name;
		}
		
		if (node -> right)
			makeBody(node -> right, nasm_compilation);
			
		if (nasm_compilation)
		{
			this -> add("\tpop rax\n\ttest rax, rax\n\tjz .exitcycle");
			this -> add(node -> data -> vartype);
			this -> add("\n");
		}
		
		else
		{
			this -> addBin("58 48 85 c0 0f 84");
			char* label_name = makeLable(".exitcycle", node -> data -> vartype);
			int label_position = getLabelPosition(label_name, this -> text_length);
			delete[] label_name;
			// storing jmp position label
			this -> addBin(4, label_position);
		}
		
		if (node -> left)
			makeBody(node -> left, nasm_compilation);
		
		if (nasm_compilation)
		{
			this -> add("\tjmp .cycle");
			this -> add(node -> data -> vartype);
			this -> add("\n");
			
			this -> add("\t.exitcycle");
			this -> add(node -> data -> vartype);
			this -> add(":\n\n");
		}
		
		else
		{
			this -> addBin("e9");
			char* label_name = makeLable(".cycle", node -> data -> vartype);
			int label_position = getLabelPosition(label_name, this -> text_length);
			delete[] label_name;
			this -> addBin(4, label_position);
			
			label_name = makeLable(".exitcycle", node -> data -> vartype);
			setLabelPosition(label_name, this -> text_length);
			delete[] label_name;
		}
	}
	
	else if (optype == Token::FOR and type == Token::CTRL_OPERATOR)
	{
		this -> add("\t\x1b[1;31mFor compilation\x1b[0m\n");
	}
	
	else if (optype == Token::IF and type == Token::CTRL_OPERATOR)
	{
		if (node -> right)
			makeBody(node -> right, nasm_compilation);
			
		if (nasm_compilation)
		{
			this -> add("\tpop rax\n\ttest rax, rax\n\tjz .endif");
			this -> add(node -> data -> vartype);
			this -> add("\n\n");
		}
		
		else
		{
			this -> addBin("58 48 85 c0 0f 84");
			char* label_name = makeLable(".endif", node -> data -> vartype);
			int label_position = getLabelPosition(label_name, this -> text_length);
			delete[] label_name;
			// storing jmp position label
			this -> addBin(4, label_position);
		}
		
		if (node -> left)
			makeBody(node -> left, nasm_compilation);
			
		if (nasm_compilation)
		{
			this -> add("\tjmp .exitif");
			this -> add(node -> data -> vartype);
			this -> add("\n\n");
			
			this -> add("\t.endif");
			this -> add(node -> data -> vartype);
			this -> add(":\n");
		}
		
		else
		{
			this -> addBin("e9");
			char* label_name = makeLable(".exitif", node -> data -> vartype);
			int label_position = getLabelPosition(label_name, this -> text_length);
			delete[] label_name;
			this -> addBin(4, label_position);
			
			label_name = makeLable(".endif", node -> data -> vartype);
			setLabelPosition(label_name, this -> text_length);
			delete[] label_name;
		}
		
		// Else compilation
		if (node -> parent -> left)
		{
			if (node -> parent -> left -> right -> data -> type == Token::CTRL_OPERATOR and node -> parent -> left -> right -> data -> ivalue == Token::ELSE)
				makeBody(node -> parent -> left -> right -> left, nasm_compilation);
		}
		
		if (nasm_compilation)
		{
			this -> add("\t.exitif");
			this -> add(node -> data -> vartype);
			this -> add(":\n");
		}
		
		else
		{
			char* label_name = makeLable(".exitif", node -> data -> vartype);
			setLabelPosition(label_name, this -> text_length);
			delete[] label_name;
		}
	}
	
	else if (optype == Token::ELSE and type == Token::CTRL_OPERATOR)
	{
		if (nasm_compilation)
			this -> add("\tnop\n");
			
		else
			this -> addBin("90");
	}
		
	else
	{
		if (node -> right)
			makeBody(node -> right, nasm_compilation);
		
		if (node -> left)
			makeBody(node -> left, nasm_compilation);
		
		compile(node, nasm_compilation);
	}
}



void Programm::pushParameters(PNode* node, bool nasm_compilation)
{
	if (node -> left)
		pushParameters(node -> left, nasm_compilation);
		
	if (node -> right)
		makeBody(node -> right, nasm_compilation);
}



void Programm::compile(PNode* node, bool nasm_compilation)
{
	if (nasm_compilation)
	{
		if (node -> data -> type == Token::ARITHM_OPERATOR or node -> data -> type == Token::CMP_OPERATOR or node -> data -> type == Token::CTRL_OPERATOR)
		{
			#define TOKEN(string, token_type, token_number, dump, nasm_code, bin_code)		\
					case token_number:														\
					{																		\
						this -> add(nasm_code);												\
						break;																\
					}	
			
			switch (node -> data -> ivalue)
			{
				#include "../Headers/syntax.hpp"
				
				default:
				{
					this -> add("\x1b[1;31mUnknown operator\x1b[0m\n");
					break;
				}
			}
			
			#undef TOKEN
		}
		
		else if (node -> data -> type == Token::VARIABLE)
		{
			if (node -> data -> LValue)
			{
				if (node -> data -> vartype == LOCAL)
				{
					this -> add("\tmov rax, r14\n\tadd rax, ");
					this -> add((node -> data -> ivalue) * 8);
					this -> add("\n\tpush rax\n\n");
				}
				else if (node -> data -> vartype == PARAMETER)
				{
					this -> add("\tmov rax, r15\n\tadd rax, ");
					this -> add((node -> data -> ivalue) * 8);
					this -> add("\n\tpush rax\n\n");
				}
			}
			else
			{
				if (node -> data -> vartype == LOCAL)
				{
					this -> add("\tmov rax, [r14 + ");
					this -> add((node -> data -> ivalue) * 8);
					this -> add("]\n\tpush rax\n\n");
				}
				else if (node -> data -> vartype == PARAMETER)
				{
					this -> add("\tmov rax, [r15 + ");
					this -> add((node -> data -> ivalue) * 8);
					this -> add("]\n\tpush rax\n\n");
				}
			}
		}
			
		else if (node -> data -> type == Token::INT)
		{
			this -> add("\tmov rax, ");
			this -> add(node -> data -> ivalue);
			this -> add("\n\tpush rax\n\n");
		}
			
		else if (node -> data -> type == Token::FLOAT)
		{
			this -> add("\tmov rax, ");
			this -> add(node -> data -> fvalue);
			this -> add("\n\tpush rax\n\n");
		}
		
		else if (node -> data -> type == Token::CHAR)
		{
			this -> add("\tmov rax, \'");
			this -> add(node -> data -> cvalue);
			this -> add("\'\n\tpush rax\n\n");
		}
			
		else if (node -> data -> type == Token::STRING)
			this -> add("\tString\n\n");
	}
	
	else
	{
		if (node -> data -> type == Token::ARITHM_OPERATOR or node -> data -> type == Token::CMP_OPERATOR or node -> data -> type == Token::CTRL_OPERATOR)
		{
			#define TOKEN(string, token_type, token_number, dump, nasm_code, bin_code)		\
					case token_number:														\
					{																		\
						this -> addBin(bin_code);												\
						break;																\
					}	
			
			switch (node -> data -> ivalue)
			{
				#include "../Headers/syntax.hpp"
				
				default:
				{
					this -> add("\x1b[1;31mUnknown operator\x1b[0m\n");
					break;
				}
			}
			
			#undef TOKEN
		}
		
		else if (node -> data -> type == Token::VARIABLE)
		{
			if (node -> data -> LValue)
			{
				if (node -> data -> vartype == LOCAL)
				{
					this -> addBin("4c 89 f0 48 83 c0");
					this -> addBin(1, (node -> data -> ivalue) * 8);
					this -> addBin("50");
				}
				else if (node -> data -> vartype == PARAMETER)
				{
					this -> addBin("4c 89 f8 48 83 c0");
					this -> addBin(1, (node -> data -> ivalue) * 8);
					this -> addBin("50");
				}
			}
			else
			{
				if (node -> data -> vartype == LOCAL)
				{
					this -> addBin("49 8b 46");
					this -> addBin(1, (node -> data -> ivalue) * 8);
					this -> addBin("50");
				}
				else if (node -> data -> vartype == PARAMETER)
				{
					this -> addBin("49 8b 47");
					this -> addBin(1, (node -> data -> ivalue) * 8);
					this -> addBin("50");
				}
			}
		}
			
		else if (node -> data -> type == Token::INT)
		{
			this -> addBin("b8");
			this -> addBin(4, node -> data -> ivalue);
			this -> addBin("50");
		}
			
		else if (node -> data -> type == Token::FLOAT)
		{
			this -> addBin("b8");
			this -> addBin(4, node -> data -> fvalue);
			this -> addBin("50");
		}
		
		else if (node -> data -> type == Token::CHAR)
		{
			this -> addBin("b8");
			this -> addBin(4, node -> data -> cvalue);
			this -> addBin("50");
		}
			
		else if (node -> data -> type == Token::STRING)
			this -> add("\tString\n\n");
	}
}



void Programm::setLabelPosition(const char* label_name, long long int position)
{
	int label_number = labels_storage.findLabel(label_name);
	labels_storage.setPosition(label_number, position);
	
	int references_count = labels_storage.getReferencesCount(label_number);
	long long int* references = labels_storage.getReferences(label_number);
	
	for (int i = 0; i < references_count; i++)
	{
		substituteNumber(references[i], 4, (position - references[i] - 4));
	}
	
	labels_storage.clearReferences(label_number);
}



long long int Programm::getLabelPosition(const char* label_name, long long int position)
{
	int label_number = labels_storage.findLabel(label_name);
	long long int adress = labels_storage.getPosition(label_number);
	
	if (not adress)
	{
		labels_storage.addReference(label_number, position);
		return 0;
	}
	
	return (adress - position - 4);
}




void Programm::optimizeNasm(const Settings* settings)
{
	std::cout << "\x1b[1;31mNasm optimization is not implemented yet!\x1b[0m\n";
}


void Programm::makeHeader()
{
	char* compiled_text = this -> text;
	long long int compiled_text_length = this -> text_length;
	
	this -> text = 0;
	this -> text_length = 0;
	
	this -> addBin("7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 02 00 3e 00 01 00 00 00 00 10 40 00 00 00 00 00 40 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 38 00 02 00 40 00 03 00 02 00 01 00 00 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 40 00 00 00 00 00 b0 00 00 00 00 00 00 00 b0 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 01 00 00 00 05 00 00 00 00 10 00 00 00 00 00 00 00 10 40 00 00 00 00 00 00 10 40 00 00 00 00 00"); 
	this -> addBin(8, compiled_text_length);
	this -> addBin(8, compiled_text_length);
	this -> addBin("00 10 00 00 00 00 00 00");
	
	for (int counter = 0; counter < 3920; counter++)
		this -> addBin("00");
		
	long long int new_text_length = compiled_text_length + (this -> text_length);
	char* new_text = new char[new_text_length];
	
	for (long long int i = 0; i < this -> text_length; i++)
		new_text[i] = (this -> text)[i];
		
	for (long long int i = 0; i < compiled_text_length; i++)
		new_text[(this -> text_length) + i] = compiled_text[i];
		
	delete[] this -> text;
	
	this -> text = new_text;
	this -> text_length = new_text_length;
	
	long long int shstrtab_indent = this -> text_length;
	
	this -> addBin("00 2e 73 68 73 74 72 74 61 62 00 2e 74 65 78 74");
	
	while (this -> text_length % 8 != 0)
		this -> addBin("00");
		
	long long int section_header_offset = this -> text_length;
		
	this -> addBin("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	this -> addBin("0b 00 00 00 01 00 00 00 06 00 00 00 00 00 00 00 00 10 40 00 00 00 00 00 00 10 00 00 00 00 00 00");
	this -> addBin(8, compiled_text_length);
	this -> addBin("00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	this -> addBin("01 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	this -> addBin(8, shstrtab_indent);//52 10 00 00    00 00 00 00    
	this -> addBin("11 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00");
	
	this -> substituteNumber(40, 8, section_header_offset);
		
	delete[] compiled_text;
}


void Programm::substituteNumber(long long int position, int bytes_count, int number)
{
	unsigned int quotient = number;
	char balance = 0;
	int counter = 0;
	while (counter < bytes_count)
	{
		balance = quotient % 256;
		quotient = quotient / 256;		
		(this -> text)[position + counter] = balance;
		
		counter++;
	}
	return;
}



void Programm::write(const Settings* settings)
{
	char* filename = 0;
	if (settings -> output_path)
	{
		filename = new char[strlen(settings -> output_path) + 2]{0};
		strcpy(filename, settings -> output_path);
	}
	
	else
	{
		if (settings -> only_preprocess)
		{
			filename = new char[strlen(settings -> source_path) + 4]{0};
			strncpy(filename, settings -> source_path, strlen(settings -> source_path));
			filename[strlen(settings -> source_path)] = 'p';
		}
		
		else if (settings -> nasm_listing)
		{
			filename = new char[strlen(settings -> source_path) + 4]{0};
			strncpy(filename, settings -> source_path, strlen(settings -> source_path) - 4);
			filename[strlen(settings -> source_path) - 4] = 's';
		}
		
		else
		{
			filename = new char[strlen(settings -> source_path) + 4]{0};
			strncpy(filename, settings -> source_path, strlen(settings -> source_path) - 4);
			strcpy(filename + strlen(settings -> source_path) - 4, "out");
		}
	}

	if (settings -> only_preprocess or settings -> nasm_listing)
	{
		FILE* file = fopen(filename, "w");

		fprintf(file, "%s", this -> text);
		
		fclose(file);
	}
	
	else
	{
		FILE* file = fopen(filename, "wb");
		fwrite(this -> text, sizeof(char), this -> text_length, file);
		fclose(file);
		
		char* cmd = new char[strlen(settings -> source_path) + 4 + 10]{0};
		strcpy(cmd, "chmod +x ");
		strcat(cmd, filename);
		system(cmd);
		delete[] cmd;
	}
	
	if (not settings -> silent)
		printf("Compiled programm saved as \x1b[1;32m<%s>\x1b[0m\n", filename);
		
	delete[] filename;
}


void Programm::add(const char* string)
{
	int add_len = strlen(string);
	int new_len = this -> text_length + add_len;
	char* new_text = new char[new_len + 1]{0};
	
	if (this -> text)
	{
		strncpy(new_text, this -> text, this -> text_length);
		strncat(new_text, string, add_len);
		delete[] this -> text;
	}
	
	else
		strncpy(new_text, string, add_len);
		
	this -> text_length = new_len;
	this -> text = new_text;
}


void Programm::add(const int number)
{
	char* string = new char[50]{0};
	sprintf(string, "%d", number);
	
	int add_len = strlen(string);
	int new_len = this -> text_length + add_len;
	char* new_text = new char[new_len + 1]{0};
	
	if (this -> text)
	{
		strncpy(new_text, this -> text, this -> text_length);
		strncat(new_text, string, add_len);
		delete[] this -> text;
	}
	
	else
		strncpy(new_text, string, add_len);
		
	this -> text_length = new_len;
	this -> text = new_text;
	
	delete[] string;
}


void Programm::add(const float number)
{
	char* string = new char[50]{0};
	sprintf(string, "%f", number);
	
	int add_len = strlen(string);
	int new_len = this -> text_length + add_len;
	char* new_text = new char[new_len + 1]{0};
	
	if (this -> text)
	{
		strncpy(new_text, this -> text, this -> text_length);
		strncat(new_text, string, add_len);
		delete[] this -> text;
	}
	
	else
		strncpy(new_text, string, add_len);
		
	this -> text_length = new_len;
	this -> text = new_text;
	
	delete[] string;
}




void Programm::addBin(const char* string)
{
	int counter = 0;
	char symbol = 0;
	while (true)
	{
		int number = 0;
		if (string[counter] >= '0' and string[counter] <= '9')
			number = string[counter] - '0';
		else if (string[counter] >= 'a' and string[counter] <= 'f')
			number = string[counter] - 'a' + 10;
			
		if (counter % 3 == 0)
			symbol = 16 * number;
			
		else if (counter % 3 == 1)
			symbol += number;
			
		else
		{
			long long int new_length = this -> text_length + 1;
			char* new_text = new char[new_length];
			
			for (int i = 0; i < this -> text_length; i++)
				new_text[i] = (this -> text)[i];
			new_text[this -> text_length] = symbol;
			
			if (this -> text)
				delete[] this -> text;
			
			this -> text = new_text;
			this -> text_length = new_length;
		}
		
		if (string[counter] == '\0')
			break;
		
		counter++;
	}
	return;
}


void Programm::addBin(const int bytes_count, const int number)
{
	unsigned int quotient = number;
	char balance = 0;
	int counter = 0;
	while (counter < bytes_count)
	{
		balance = quotient % 256;
		quotient = quotient / 256;
		counter++;
		
		long long int new_length = this -> text_length + 1;
		char* new_text = new char[new_length];
		
		for (int i = 0; i < this -> text_length; i++)
			new_text[i] = (this -> text)[i];
		new_text[this -> text_length] = balance;
		
		if (this -> text)
			delete[] this -> text;
		
		this -> text = new_text;
		this -> text_length = new_length;
		
	}
	return;
}


void Programm::addBin(const float number)
{
	return;
}




int strcount(char* str, const char* expression)
{
	int counter = 0;
	char* _str = str;
	bool flag = true;
	while (flag)
	{
		_str = strstr(_str, expression);
		if (_str == nullptr)
			flag = false;
		else
		{
			counter++;
			_str += 1;
		}
	}
	
	return counter;
}

bool isSpace(char symbol)
{
	return (symbol == ' ' or symbol == '\n' or symbol == '\t' or symbol == '\0');
}


char* makeLable(const char* string, int number)
{
	char* label = new char[strlen(string) + 7];
	sprintf(label, "%s%d", string, number);
	return label;
}


char* readFile(const char* filename)
{
	FILE* source_file = fopen(filename, "r");
	
	fseek(source_file, 0, SEEK_END);
	long long text_length = ftell(source_file);
	rewind(source_file);

	char* text = new char[text_length + 1]{0};

	text_length = fread(text, sizeof(char), text_length, source_file);
	
	fclose(source_file);
	
	return text;
}







// end;

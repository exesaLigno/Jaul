/*!
 *	@file ast.cpp
 *	@brief Исполняемый код класса AbstractSyntaxTree
 */


#include "../Headers/ast.hpp"


AbstractSyntaxTree::Node::Node(char** text)
{
	int first_sym = 0;
	if (isLetter(**text))
		first_sym = LETTER;
		
	else if (isNumber(**text))
		first_sym = NUMBER;
		
	else if (isOperand(**text))
		first_sym = SPECSYMBOL;
	
	else
		return;
		
	int token_len = 0;
	char* token_start = *text;
	bool in_token = true;
	
	while (in_token)
	{
		token_len++;
		(*text)++;
		
		if (((first_sym == LETTER or first_sym == NUMBER) and (isOperand(**text) or isEnd(**text))) or	\
		   (first_sym == SPECSYMBOL and (not isOperand(**text))))
			in_token = false;
	}
	
	char* token = new char[token_len + 1]{0};
	strncpy(token, token_start, token_len);
	
	if(false);
	
	#define TOKEN(string, token_type, token_number, dump, nasm_code)		\
			else if (!strcmp(token, string))								\
			{																\
				this -> type = token_type;									\
				this -> ivalue = token_number;								\
				delete[] token;												\
			}																\
			
	#include "../Syntax/jaul_syntax.hpp"
	
	#undef TOKEN
	
	else if (isInteger(token))
	{
		this -> type = CONSTANT;
		this -> data_type = INT;
		this -> ivalue = atoi(token);
		delete[] token;
	}
	
	else if (isFloat(token))
	{
		this -> type = CONSTANT;
		this -> data_type = FLOAT;
		this -> fvalue = atof(token);
		delete[] token;
	}

	else
	{
		char* test_next = *text;
		
		while (*test_next == ' ')
			(test_next)++;
			
		if (*test_next == '(')
			this -> type = FUNCCALL;
			
		else
			this -> type = VARIABLE;
			
		if (*test_next == '=' and *(test_next + 1) != '=')	
			this -> LValue = true;
			
		this -> svalue = token;
	}
}


AbstractSyntaxTree::Node::Node(int type)
{
	this -> type = type;
	
	if (type == _START)
	{
		this -> type = FUNC;
		this -> svalue = new char[10];
		strcpy(this -> svalue, "_start");
	}
}


AbstractSyntaxTree::Node::Node(int type, char** text)
{
	this -> type = type;
	
	int first_sym = 0;
	if (isLetter(**text))
		first_sym = LETTER;
		
	if (isNumber(**text))
		first_sym = NUMBER;
		
	if (isOperand(**text))
		first_sym = SPECSYMBOL;
		
	int token_len = 0;
	char* token_start = *text;
	bool in_token = true;
	
	while (in_token)
	{
		token_len++;
		(*text)++;
		
		if (((first_sym == LETTER or first_sym == NUMBER) and (isOperand(**text) or isEnd(**text))) or	\
		   (first_sym == SPECSYMBOL and (not isOperand(**text))))
			in_token = false;
	}
	
	char* token = new char[token_len + 1]{0};
	strncpy(token, token_start, token_len);
	
	this -> svalue = token;
}


AbstractSyntaxTree::Node::~Node()
{
	if (this -> left)
		delete this -> left;

	if (this -> right)
		delete this -> right;
		
	if (this -> svalue)
		delete[] this -> svalue;
	
	if (this -> line_content)
		delete[] this -> line_content;
		
	if (this -> container)
		(this -> container -> nodes_count)--;
}


void AbstractSyntaxTree::Node::flush()
{
	this -> type = 0;
	this -> ivalue = 0;
	this -> fvalue = 0;
	this -> cvalue = 0;
	
	if (this -> svalue)
	{
		delete this -> svalue;
		this -> svalue = nullptr;
	}
	
	this -> LValue = false;
	this -> vartype = 0;
}



void AbstractSyntaxTree::Node::leftConnect(Node* left)
{
	this -> left = left;
	if (left)
	{
		left -> parent = this;
		if (this -> container)
		{
			if (this -> left -> container != this -> container)
			{
				this -> container -> nodes_count += 1;
				this -> left -> container = this -> container;
			}
			left -> fixContainer();
		}
	}
}


void AbstractSyntaxTree::Node::rightConnect(Node* right)
{
	this -> right = right;
	if (right)
	{
		right -> parent = this;
		if (this -> container)
		{
			if (this -> right -> container != this -> container)
			{
				this -> container -> nodes_count += 1;
				this -> right -> container = this -> container;
			}
			right -> fixContainer();
		}
	}
}



void AbstractSyntaxTree::Node::fixContainer()
{
	if (this -> right)
	{
		if (this -> right -> container != this -> container)
		{
			this -> container -> nodes_count += 1;
			this -> right -> container = this -> container;
		}
		this -> right -> fixContainer();
	}
	
	if (this -> left)
	{
		if (this -> left -> container != this -> container)
		{
			this -> container -> nodes_count += 1;
			this -> left -> container = this -> container;
		}
		this -> left -> fixContainer();
	}
}



const char* AbstractSyntaxTree::Node::colorize()
{
	if (this -> type == ENTRY or this -> type == INCLUDE or this -> type == FUNC)
		return "green";
		
	else if (this -> type == LINE or this -> type == ITEM or this -> type == DEF)
		return "lightgray";
	
	else if (this -> type == ARITHM_OPERATOR)
		return "pink";
		
	else if (this -> type == CMP_OPERATOR)
		return "\"#F8AEFF\"";
		
	else if (this -> type == CTRL_OPERATOR)
		return "orange";
		
	else if (this -> type == CONSTANT and this -> data_type == INT)
		return "\"#D083FF\"";

	else if (this -> type == CONSTANT and this -> data_type == FLOAT)
		return "\"#FF89A9\"";

	else if (this -> type == VARIABLE)
		return "\"#64ADFF\"";
		
	else if (this -> type == FUNCCALL)
		return "\"#0B8AFF\"";
		
	else
		return "white";
}


void AbstractSyntaxTree::Node::print()
{
	if (this -> type == UNKNOWN)
	{
		printf("unknown operator");
		
		if (this -> svalue)
			printf(" (%s)", this -> svalue);
	}

	else if (this -> type == ENTRY)
		printf("entry");
	
	else if (this -> type == LINE)
		printf("<%s:%d>", this -> svalue, this -> ivalue);

	else if (this -> type == INCLUDE)
		printf("include of %s", this -> svalue);

	else if (this -> type == FUNC)
		printf("function %s", this -> svalue);
	
	else if (this -> type == ITEM)
		printf("item");
	
	#define TOKEN(string, token_type, token_number, dump, nasm_code)			\
	else if (this -> type == token_type and this -> ivalue == token_number)		\
		printf("%s", dump);														\

	#include "../Syntax/jaul_syntax.hpp"

	#undef TOKEN

	else if (this -> type == INT)
		printf("%d", this -> ivalue);
	
	else if (this -> type == FLOAT)
		printf("%g", this -> fvalue);
	
	else if (this -> type == VARIABLE)
	{
		if (this -> vartype == LOCAL)
			printf("local variable %s", this -> svalue);
		
		else if (this -> vartype == PARAMETER)
			printf("global variable %s", this -> svalue);
	}
	
	else if (this -> type == FUNCCALL)
		printf("call of function %s", this -> svalue);
}



void AbstractSyntaxTree::Node::write(std::ofstream& out)
{
	if (this -> type == UNKNOWN)
	{
		out << "unknown";
		
		if (this -> svalue)
			out << " | " << this -> svalue;
	}
		
	else if (this -> type == ENTRY)
		out << "entry";
		
	else if (this -> type == LINE)
	{
		out << "line " << this -> ivalue;
		out << " | " << this -> svalue;
	}
	
	else if (this -> type == INCLUDE)
	{
		out << "include";
		out << " | " << this -> svalue;
	}
		
	else if (this -> type == FUNC)
		out << "function | " << this -> svalue << " | { Mem | " << this -> ivalue << " } ";
		
	else if (this -> type == ITEM)
		out << "item";

	#define TOKEN(string, token_type, token_number, dump, nasm_code)					\
			else if (this -> type == token_type and this -> ivalue == token_number)		\
				out << "OPERATOR | " << dump;											\
			
	#include "../Syntax/jaul_syntax.hpp"
	
	#undef TOKEN

	else if (this -> type == CONSTANT and this -> data_type == INT)
		out << this -> ivalue;
		
	else if (this -> type == CONSTANT and this -> data_type == FLOAT)
		out << this -> fvalue;
		
	else if (this -> type == VARIABLE)
	{
		if (this -> LValue)
			out << "L-";
		out << "variable | " << this -> svalue;
			
		if (this -> vartype == LOCAL)
			out << " | { Local | " << this -> ivalue << " }";
			
		else if (this -> vartype == PARAMETER)
			out << " | { Parameter  | " << this -> ivalue << " }";
	}
		
	else if (this -> type == FUNCCALL)
		out << "funccall | " << this -> svalue;
		
	else if (this -> type == DEF)
	{
		out << "def";
		out << " | line " << this -> ivalue;
		if (this -> svalue)
			out << " | " << this -> svalue;
	}
	
	else
	{
		out << "err type: " << this -> type;
		
		if (this -> svalue)
			out << " | " << this -> svalue;
	}
	
	if (this -> data_type == INT)
		out << " | integer ";
	
	else if (this -> data_type == FLOAT)
		out << " | float ";
}


int AbstractSyntaxTree::dumper(const char* filename, int mode, const char* title)
{
	char* cmd = new char[2 * strlen(filename) + 1 + 8 + 29];
	strcpy(cmd, filename);
	strcat(cmd, ".txt");
    std::ofstream file;
    file.open(cmd);
    file << "digraph G{\n";
	file << "graph [label=\"" << title << "\", labelloc=top, fontsize=30]";
    file << "root [shape = box, style = filled, fillcolor = orange, color = black, label = \" Root = " << this -> head << "\"]\n";
    file << "count [shape = box, style = filled, fillcolor = orange, color = black, label = \" Count of nodes = " << this -> nodes_count << "\"]\n";
    AbstractSyntaxTree::Node* element = this -> head;
    
    file << (long int) element << " [shape = record, style = filled, fillcolor = " << element -> colorize() << ", color = black, label = \" {";
    
    if ((mode / DETAILED) % 10 == 1)
	{
    	file << "<adr> Address: " << element << " | Data: ";
	}
    
	element -> write(file);
    
    if ((mode / DETAILED) % 10 == 1)
	{
    	file << " |{<left> " << element -> left << " | <right> " << element -> right << "}";
	}
    	
    file << "}\"]\n";
    
    file << "root -> " << (long int) element << "\n [color = black]";	// element as int
	
    element -> dumper(file, mode);
    file << "}";
    file.close();
    
    strcpy(cmd, "dot -Tpng ");
    strcat(cmd, filename);
    strcat(cmd, ".txt -o ");
    strcat(cmd, filename);
    strcat(cmd, ".png");
    int result = system(cmd);
    
    if ((mode / DELETE_TXT) % 10 == 1)
    {
    	strcpy(cmd, "rm ");
    	strcat(cmd, filename);
    	strcat(cmd, ".txt");
    	result = system(cmd);
    }
    	
    if ((mode / SHOW) % 10 == 1)
   	{
    	strcpy(cmd, "eog ");
    	strcat(cmd, filename);
    	strcat(cmd, ".png");
    	result = system(cmd);
    }
    	
    if ((mode / DELETE_PNG) % 10 == 1)
    {
    	strcpy(cmd, "rm ");
    	strcat(cmd, filename);
    	strcat(cmd, ".png");
    	result = system(cmd);
    }
    	
    delete[] cmd;
    	
    return 0;
}



int AbstractSyntaxTree::Node::dumper(std::ofstream& file, int mode)
{
	
    if (this -> left)
    {
        file << (long int) this -> left << " [shape = record, style = filled, fillcolor = " << this -> left -> colorize() << ", color = black, label = \" {";
        
        if ((mode / DETAILED) % 10 == 1)
        	file << "<adr> Address: " << this -> left << " | Data: ";
        
		this -> left -> write(file);
        
        if ((mode / DETAILED) % 10 == 1)
        	file << " | <prev> Prev: " << this << " |{<left> " << this -> left -> left << " | <right> " << this -> left -> right << "}";
        
        file << "}\"]\n";
        
        if ((mode / DETAILED) % 10 == 1)
        {
        	file << (long int) this << ":<left> -> " << (long int) this -> left << " [color = black]\n";
        	file << (long int) this -> left << ":<prev> -> " << (long int) this << " [color = gray]\n";
        }
        	
        else
        	file << (long int) this << " -> " << (long int) this -> left << " [color = black]\n";
		
        this -> left -> dumper(file, mode);
    }

    if (this -> right)
    {
		file << (long int) this -> right << " [shape = record, style = filled, fillcolor = " << this -> right -> colorize() << ", color = black, label = \" {";
        
        if ((mode / DETAILED) % 10 == 1)
        	file << "<adr> Address: " << this -> right << " | Data: ";
        
		this -> right -> write(file);
        
        if ((mode / DETAILED) % 10 == 1)
        	file << " | <prev> Prev: " << this << " |{<left> " << this -> right -> left << " | <right> " << this -> right -> right << "}";
        
        file << "}\"]\n";
        
        if ((mode / DETAILED) % 10 == 1)
        {
        	file << (long int) this << ":<right> -> " << (long int) this -> right << " [color = black]\n";
        	file << (long int) this -> right << ":<prev> -> " << (long int) this << " [color = gray]\n";
        }
        
        else
        	file << (long int) this << " -> " << (long int) this -> right << " [color = black]\n";
		
        this -> right -> dumper(file, mode);
    }

    return 0;
}






AbstractSyntaxTree::AbstractSyntaxTree()
{
	this -> head = 0;
	this -> nodes_count = 0;
}



AbstractSyntaxTree::~AbstractSyntaxTree()
{
	if (this -> head)
		delete this -> head;
}


typename AbstractSyntaxTree::Node* AbstractSyntaxTree::createNode(char** text)
{
	Node* new_node = new Node(text);
	if (new_node -> type == ASN::UNKNOWN)
	{
		delete new_node;
		return nullptr;
	}
	
	this -> nodes_count++;
	new_node -> container = this;
	if (not this -> head)
		this -> head = new_node;
	
	return new_node;
}

typename AbstractSyntaxTree::Node* AbstractSyntaxTree::createNode(int type, char** text)
{
	Node* new_node = new Node(type, text);

	if (new_node -> type == ASN::UNKNOWN)
	{
		
		delete new_node;
		return nullptr;
	}
	
	this -> nodes_count++;
	new_node -> container = this;
	if (not this -> head)
		this -> head = new_node;
	
	return new_node;
}

typename AbstractSyntaxTree::Node* AbstractSyntaxTree::createNode(int type)
{
	Node* new_node = new Node(type);
	if (new_node -> type == ASN::UNKNOWN)
	{
		delete new_node;
		return nullptr;
	}
	
	this -> nodes_count++;
	new_node -> container = this;
	if (not this -> head)
		this -> head = new_node;
	
	return new_node;
}




bool isLetter(char symbol)
{
	return ((symbol >= 'A' and symbol <= 'Z') or (symbol >= 'a' and symbol <= 'z') or symbol == '.' or symbol == '_' or symbol == '\'' or symbol == '\"');
}

bool isNumber(char symbol)
{
	return ((symbol >= '0' and symbol <= '9'));
}

bool isOperand(char symbol)
{
	return (symbol == '>' or symbol == '<' or symbol == '=' or symbol == '!' or \
			symbol == '+' or symbol == '-' or symbol == '*' or symbol == '/' or \
			symbol == '%' or symbol == '|' or symbol == '&' or symbol == '^');
}

bool isEnd(char symbol)
{
	return (symbol == ' ' or symbol == '\t' or symbol == '\n' or symbol == '\0' or 
			symbol == '(' or symbol == ')' or symbol == ',');
}



bool isInteger(char* string)
{
	bool result = true;
	
	while (*string != '\0')
	{
		if (not (isNumber(*string)))
			result = false;
		string++;
	}
	
	return result;
}

bool isFloat(char* string)
{
	bool result = true;
	
	while (*string != '\0')
	{
		if (not (isNumber(*string) or *string == '.'))
			result = false;
		string++;
	}
	
	return result;
}

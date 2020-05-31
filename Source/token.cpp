#include "../Headers/token.hpp"

Token::Token()
{}

Token::Token(int type)
{
	this -> type = type;
	this -> ivalue = 0;
	this -> fvalue = 0;
	this -> cvalue = 0;
	this -> svalue = 0;
	this -> vartype = 0;
	this -> LValue = false;
	if (type == Token::_START)
	{
		this -> type = Token::FUNC;
		this -> svalue = new char[10];
		strcpy(this -> svalue, "_start");
	}
}

Token::Token(char** text)
{
	this -> type = UNKNOWN;
	this -> ivalue = 0;
	this -> fvalue = 0;
	this -> cvalue = 0;
	this -> svalue = 0;
	this -> vartype = 0;
	this -> LValue = false;
	
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
	
	if(0)
		std::cout << "\x1b[1;31mbullshit\n\x1b[0m";

	#define TOKEN(string, token_type, token_number, dump, nasm_code, bin_code)		\
			else if (!strcmp(token, string))										\
			{																		\
				this -> type = token_type;											\
				this -> ivalue = token_number;										\
				delete[] token;														\
			}																		\
			
	#include "../Headers/syntax.hpp"
	
	#undef TOKEN
	
	else if (isInteger(token))
	{
		this -> type = INT;
		this -> ivalue = atoi(token);
		delete[] token;
	}
	
	else if (isFloat(token))
	{
		this -> type = FLOAT;
		this -> fvalue = atof(token);
		delete[] token;
	}
	
	else if (isChar(token))
	{
		this -> type = CHAR;
		this -> cvalue = token[1];
		delete[] token;
	}
	
	else if (isString(token))
	{
		this -> type = STRING;
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


Token::Token(int type, char** text)
{
	this -> type = type;
	this -> ivalue = 0;
	this -> fvalue = 0;
	this -> cvalue = 0;
	this -> svalue = 0;
	this -> vartype = 0;
	this -> LValue = false;
	
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




std::ostream& operator<< (std::ostream &out, const Token &token)
{
	if (token.type == Token::UNKNOWN)
		out << "unknown | " << token.svalue;
		
	else if (token.type == Token::ENTRY)
		out << "entry";
		
	else if (token.type == Token::LINE)
		out << "line";
		
	else if (token.type == Token::FUNC)
		out << "function | " << token.svalue << " | { Mem | " << token.ivalue << " } ";
		
	else if (token.type == Token::ITEM)
		out << "item";
	
	#define TOKEN(string, token_type, token_number, dump, nasm_code, bin_code)					\
			else if (token.type == token_type and token.ivalue == token_number)					\
				out << "OPERATOR | " << dump;													\
			
	#include "../Headers/syntax.hpp"
	
	#undef TOKEN
	
	else if (token.type == Token::INT)
		out << "int | " << token.ivalue;
		
	else if (token.type == Token::FLOAT)
		out << "float | " << token.fvalue;
		
	else if (token.type == Token::CHAR)
		out << "char | " << token.cvalue;
		
	else if (token.type == Token::STRING)
		out << "string | " << token.svalue;
		
	else if (token.type == Token::VARIABLE)
	{
		if (token.LValue)
			out << "L-";
		out << "variable | " << token.svalue;
			
		if (token.vartype == LOCAL)
			out << " | { Local | " << token.ivalue << " }";
			
		else if (token.vartype == PARAMETER)
			out << " | { Parameter  | " << token.ivalue << " }";
	}
		
	else if (token.type == Token::FUNCCALL)
		out << "function | " << token.svalue;
		
	else if (token.type == Token::DEF)
		out << "def";
	
	return out;
}


bool isLetter(char symbol)
{
	return ((symbol >= 'A' and symbol <= 'Z') or (symbol >= 'a' and symbol <= 'z') or symbol == '.' or symbol == '_' or symbol == '\'' or symbol == '\"' or symbol == '-');
}

bool isNumber(char symbol)
{
	return ((symbol >= '0' and symbol <= '9') or symbol == '-');
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


Token::~Token()
{
	if (this -> svalue)
		delete[] svalue;
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

bool isChar(char* string)
{
	bool result = true;
	
	if (*string != '\'')
		result = false;
	
	while (*string != '\0')
		string++;

	string--;
	
	if (*string != '\'')
		result = false;
	
	return result;
}


bool isString(char* string)
{
	bool result = true;
	
	if (*string != '\"')
		result = false;
	
	while (*string != '\0')
		string++;

	string--;
	
	if (*string != '\"')
		result = false;
	
	return result;
}







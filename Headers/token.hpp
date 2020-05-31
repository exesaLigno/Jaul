#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include "debug.hpp"

enum SYMTYPES
{
	LETTER = 1, NUMBER, SYMBOL, SPECSYMBOL,
	COUNT_OF_SYMTYPES
};


enum VARTYPES
{
	LOCAL = 1,
	PARAMETER
};


class Token
{
  public:
  	int type;
  	int ivalue;
  	float fvalue;
  	char cvalue;
  	char* svalue;
  	bool LValue;
  	int vartype;
  	
  	
  public:
  	enum TYPES
  	{
  		UNKNOWN,
  		ARITHM_OPERATOR, CMP_OPERATOR, CTRL_OPERATOR,		// OPERATORS
  		VARIABLE, FUNCCALL, INT, FLOAT, STRING, CHAR,		// OPERANDS
  		LINE, FUNC, ENTRY, ITEM, DEF,						// SPECIAL SYMBOLS
  		_START,
  		COUNT_OF_TYPES
  	};
  	
  	enum OPERATORS
  	{
  		NOP,
  		PLUS, MINUS, MULTIPLY, DIVIDE, POWER,				// ariphmetics operators
  		MODULO, INT_DIVISION,
  		
  		EQUAL, NOT_EQUAL, MORE, LESS, MORE_EQ, LESS_EQ,		// comparison operators
  		AND, OR,
  		
  		IF, ELSE, WHILE, FOR, RETURN, ASSIGNMENT,			// control operators
  		PLUS_ASSIGNMENT, MINUS_ASSIGNMENT,
  		MULTIPLY_ASSIGNMENT, DIVIDE_ASSIGNMENT,
  		POWER_ASSIGNMENT,
  		
  		COUNT_OF_OPERATORS
  	};
  	
  	Token();
  	Token(int type);
  	Token(int type, char** text);
  	Token(char** text);
  	Token(const Token& that) = delete;
  	
  	friend std::ostream& operator<< (std::ostream &out, const Token &token);
  	
  	~Token();
};


bool isLetter(char symbol);
bool isNumber(char symbol);
bool isOperand(char symbol);
bool isEnd(char symbol);

bool isInteger(char* string);
bool isFloat(char* string);
bool isChar(char* string);
bool isString(char* string);





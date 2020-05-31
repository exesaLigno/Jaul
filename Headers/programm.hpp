#pragma once

#include "../Source/tree.cpp"
#include "settings.hpp"
#include "token.hpp"
#include "library.hpp"
#include "definition.hpp"
#include "debug.hpp"
#include "errcodes.hpp"
#include "label_storage.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <cctype>


#define skip_spaces(where)	while (*where == ' ')	\
								(where)++;			\


typedef Tree<Token*> PTree;
//typedef Tree<Token*> PTree;
typedef PTree::Node PNode;


#define WRONGINDENT -2
#define EMPTYLINE -1



class Programm
{
  public:
	char* text;
	long long int text_length;
	PTree programm_tree;
	LabelsStorage labels_storage;
	
	int importLibraries();
	int setDefinitions();
	int deleteComments();
	
	
  public:
	Programm();
	~Programm();
	
	void readSource(const Settings* settings);
	
	void preprocessor(const Settings* settings);
	
	void makeTree(const Settings* settings);
	void optimizeTree(const Settings* settings);
	void rebuildTree();
	void prepareVariables();
	void prepareVariables(PNode* node);
	
	void makeNasm(const Settings* settings);
	void compileDef(PNode* node, bool nasm_compilation);
	void makeBody(PNode* node, bool nasm_compilation);
	void pushParameters(PNode* node, bool nasm_compilation);
	void compile(PNode* node, bool nasm_compilation);
	
	void setLabelPosition(const char* label_name, long long int position);
	long long int getLabelPosition(const char* label_name, long long int position);
	
	void optimizeNasm(const Settings* settings);
	
	void makeHeader();
	
	void write(const Settings* settings);
	
	void add(const char* string);
	void add(const int number);
	void add(const float number);
	
	void addBin(const char* string);
	void addBin(const int bytes_count, const int number);
	void substituteNumber(long long int position, int count_of_bytes, int number);
	void addBin(const float number);
};


int strcount(char* str, const char* expression);
bool isSpace(char symbol);
int getHash(char*);

const char* colorize(Token* token);

int calculateIndent(char* text);
bool nextLine(char** _text);
PNode* parseBlock(int indent, char** _text);
PNode* parseLine(int indent, char** _text);

PNode* getDef(char** _line);

PNode* getAssignment(char** _line);
PNode* getOperators(char** _line);
PNode* getLogic(char** _line);
PNode* getCmp(char** _line);
PNode* getAddSub(char** _line);
PNode* getMulDiv(char** _line);
PNode* getPow(char** _line);
PNode* getNumVarFunc(char** _line);

PNode* getItemize(char** _line);

void enumerateBranching(PNode* node, int* number);
char* getUnnumeratedVariable(PNode* node);
void setVariables(PNode* node, const char* varname, int vartype, int varnumber);
char* makeLable(const char* string, int number);
char* readFile(const char* filename);




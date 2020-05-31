#pragma once
#include <cstdio>
#include <iostream>
#include <fstream>
//#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include "../Headers/debug.hpp"

static const int DEFAULT = 0;
static const int SHOW = 1;
static const int DELETE_PNG = 10;
static const int DELETE_TXT = 100;
static const int DETAILED = 1000;

static const int DATA_POINTER = 1;
static const int DATA_STRING = 10;


template <class D>
class Tree
{	
  public:

	class Node
	{		
		friend class Tree;
	  public:
		D data;
		Node* parent;
		Node* left;
		Node* right;
	
		Node(D data);
		~Node();
		
		void leftConnect(Node* left);
		void rightConnect(Node* right);
		void fixContainer();
		
		Tree<D>::Node* leftPush(D data);
		Tree<D>::Node* rightPush(D data);
		
		int dumper(std::ofstream& file, int mode);
		int dumper(std::ofstream& file, int mode, const char* colorize(D));
		
	  private:
		Tree* container;
	};
	
	Tree();
	~Tree();
	
	int dumper(const char* filename, int mode);
	int dumper(const char* filename, int mode, const char* colorize(D));
	Node* createNode(D data);
	
	void setType(int type);
	
	Node* head;
	
  private:
	unsigned long long int nodes_count;
	bool data_is_pointer;
	bool data_is_string;
};








template <class D>
Tree<D>::Node::Node (D data)
{
	this -> container = 0;
	this -> data = data;
	this -> parent = 0;
	this -> left = 0;
	this -> right = 0;
}

template <class D>
Tree<D>::Node::~Node()
{
	//std::cout << "\nTree::Node object deleting...\n";
	
	if (this -> left)
		delete this -> left;

	if (this -> right)
		delete this -> right;
		
	
	if (this -> container)
	{		
		if (this -> container -> data_is_pointer and this -> data)
			delete this -> data;
		else if (this -> container -> data_is_string and this -> data)
			delete[] this -> data;
	}
	
	//std::cout << "Tree::Node object deleted\n";
}


template <class D>
void Tree<D>::Node::leftConnect(Node* left)
{
	if (left)
	{
		this -> left = left;
		left -> parent = this;
		if (this -> container)
		{
			this -> container -> nodes_count += 1;
			left -> container = this -> container;
			left -> fixContainer();
		}
	}
}

template <class D>
void Tree<D>::Node::rightConnect(Node* right)
{
	if (right)
	{
		this -> right = right;
		right -> parent = this;
		if (this -> container)
		{
			this -> container -> nodes_count += 1;
			right -> container = this -> container;
			right -> fixContainer();
		}
	}
}

template <class D>
void Tree<D>::Node::fixContainer()
{
	if (this -> right)
	{
		this -> container -> nodes_count += 1;
		this -> right -> container = this -> container;
		this -> right -> fixContainer();
	}
	
	if (this -> left)
	{
		this -> container -> nodes_count += 1;
		this -> left -> container = this -> container;
		this -> left -> fixContainer();
	}
}


template <class D>
typename Tree<D>::Node* Tree<D>::Node::leftPush(D data)
{
	bool head_exists = false;
	if (this -> container -> head)
		head_exists = true;
		
	Node* new_node = this -> container -> createNode(data);
	
	if (head_exists)
		this -> leftConnect(new_node);
		
	return new_node;
}

template <class D>
typename Tree<D>::Node* Tree<D>::Node::rightPush(D data)
{
	bool head_exists = false;
	if (this -> container -> head)
		head_exists = true;
		
	Node* new_node = this -> container -> createNode(data);
	
	if (head_exists)
		this -> rightConnect(new_node);
		
	return new_node;
}



template <class D>
int Tree<D>::dumper(const char* filename, int mode)
{
	char* cmd = new char[2 * strlen(filename) + 1 + 8 + 29];
	strcpy(cmd, filename);
	strcat(cmd, ".txt");
    std::ofstream file;
    file.open(cmd);
    file << "digraph G{\n";
    file << "root [shape = box, style = filled, fillcolor = orange, color = black, label = \" Root = " << this -> head << "\"]\n";
    file << "count [shape = box, style = filled, fillcolor = orange, color = black, label = \" Count of nodes = " << this -> nodes_count << "\"]\n";
    Tree<D>::Node* element = this -> head;
    
    file << (long int) element << " [shape = record, style = filled, fillcolor = lightblue, color = black, label = \" {";	// element interprying as int
    
    if ((mode / DETAILED) % 10 == 1)
    	file << "<adr> Address: " << element << " | Data: ";
    
    if (this -> data_is_pointer)
    	file << *(element -> data);
    else
    	file << element -> data;
    
    if ((mode / DETAILED) % 10 == 1)
    	file << " |{<left> " << element -> left << " | <right> " << element -> right << "}";
    	
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
    system(cmd);
    
    if ((mode / DELETE_TXT) % 10 == 1)
    {
    	strcpy(cmd, "rm ");
    	strcat(cmd, filename);
    	strcat(cmd, ".txt");
    	system(cmd);
    }
    	
    if ((mode / SHOW) % 10 == 1)
   	{
    	strcpy(cmd, "eog ");
    	strcat(cmd, filename);
    	strcat(cmd, ".png");
    	system(cmd);
    }
    	
    if ((mode / DELETE_PNG) % 10 == 1)
    {
    	strcpy(cmd, "rm ");
    	strcat(cmd, filename);
    	strcat(cmd, ".png");
    	system(cmd);
    }
    	
    delete[] cmd;
    	
    return 0;
}


template <class D>
int Tree<D>::Node::dumper(std::ofstream& file, int mode)
{
	
    if (this -> left)
    {
        file << (long int) this -> left << " [shape = record, style = filled, fillcolor = lightblue, color = black, label = \" {";
        
        if ((mode / DETAILED) % 10 == 1)
        	file << "<adr> Address: " << this -> left << " | Data: ";
        
        if (this -> container -> data_is_pointer)
			file << *(this -> left -> data);
		else
			file << this -> left -> data;
        
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
		file << (long int) this -> right << " [shape = record, style = filled, fillcolor = lightblue, color = black, label = \" {";
        
        if ((mode / DETAILED) % 10 == 1)
        	file << "<adr> Address: " << this -> right << " | Data: ";
        
        if (this -> container -> data_is_pointer)
			file << *(this -> right -> data);
		else
			file << this -> right -> data;
        
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




template <class D>
int Tree<D>::dumper(const char* filename, int mode, const char* colorize(D))
{
	char* cmd = new char[2 * strlen(filename) + 1 + 8 + 29];
	strcpy(cmd, filename);
	strcat(cmd, ".txt");
    std::ofstream file;
    file.open(cmd);
    file << "digraph G{\n";
    file << "root [shape = box, style = filled, fillcolor = orange, color = black, label = \" Root = " << this -> head << "\"]\n";
    file << "count [shape = box, style = filled, fillcolor = orange, color = black, label = \" Count of nodes = " << this -> nodes_count << "\"]\n";
    Tree<D>::Node* element = this -> head;
    
    file << (long int) element << " [shape = record, style = filled, fillcolor = " << colorize(element -> data) << ", color = black, label = \" {";	// element interprying as int
    
    if ((mode / DETAILED) % 10 == 1)
    	file << "<adr> Address: " << element << " | Data: ";
    
    if (this -> data_is_pointer)
		file << *(element -> data);
	else
		file << element -> data;
    
    if ((mode / DETAILED) % 10 == 1)
    	file << " |{<left> " << element -> left << " | <right> " << element -> right << "}";
    	
    file << "}\"]\n";
    
    file << "root -> " << (long int) element << "\n [color = black]";	// element as int
    element -> dumper(file, mode, colorize);
    file << "}";
    file.close();
    
    strcpy(cmd, "dot -Tpng ");
    strcat(cmd, filename);
    strcat(cmd, ".txt -o ");
    strcat(cmd, filename);
    strcat(cmd, ".png");
    system(cmd);
    
    if ((mode / DELETE_TXT) % 10 == 1)
    {
    	strcpy(cmd, "rm ");
    	strcat(cmd, filename);
    	strcat(cmd, ".txt");
    	system(cmd);
    }
    	
    if ((mode / SHOW) % 10 == 1)
   	{
    	strcpy(cmd, "eog ");
    	strcat(cmd, filename);
    	strcat(cmd, ".png");
    	system(cmd);
    }
    	
    if ((mode / DELETE_PNG) % 10 == 1)
    {
    	strcpy(cmd, "rm ");
    	strcat(cmd, filename);
    	strcat(cmd, ".png");
    	system(cmd);
    }
    	
    delete[] cmd;
    	
    return 0;
}


template <class D>
int Tree<D>::Node::dumper(std::ofstream& file, int mode, const char* colorize(D))
{
	
    if (this -> left)
    {
        file << (long int) this -> left << " [shape = record, style = filled, fillcolor = " << colorize(this -> left -> data) << ", color = black, label = \" {";
        
        if ((mode / DETAILED) % 10 == 1)
        	file << "<adr> Address: " << this -> left << " | Data: ";
        
        if (this -> container -> data_is_pointer)
			file << *(this -> left -> data);
		else
			file << this -> left -> data;
        
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
        	
        this -> left -> dumper(file, mode, colorize);
    }

    if (this -> right)
    {
		file << (long int) this -> right << " [shape = record, style = filled, fillcolor = " << colorize(this -> right -> data) << ", color = black, label = \" {";
        
        if ((mode / DETAILED) % 10 == 1)
        	file << "<adr> Address: " << this -> right << " | Data: ";
        
        if (this -> container -> data_is_pointer)
			file << *(this -> right -> data);
		else
			file << this -> right -> data;
        
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
        	
        this -> right -> dumper(file, mode, colorize);
    }

    return 0;
}





template <class D>
Tree<D>::Tree()
{
	this -> head = 0;
	this -> nodes_count = 0;
	this -> data_is_pointer = false;
	this -> data_is_string = false;
}

template <class D>
void Tree<D>::setType(int type)
{
	if ((type / DATA_POINTER) % 10 == 1)
		this -> data_is_pointer = true;
		
	if ((type / DATA_STRING) % 10 == 1)
		this -> data_is_string = true;
}

template <class D>
Tree<D>::~Tree()
{
	if (this -> head)
		delete this -> head;
}

template <class D>
typename Tree<D>::Node* Tree<D>::createNode(D data)
{
	Node* new_node = new Node(data);
	this -> nodes_count++;
	new_node -> container = this;
	if (not this -> head)
		this -> head = new_node;
	
	return new_node;
}

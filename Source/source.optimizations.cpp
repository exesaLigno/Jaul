#include "../Headers/source.hpp"

int Source::substituteStatic(ASN* node)
{
	int change_count = 0;
	
	if (node -> type == ASN::CTRL_OPERATOR and node -> ivalue == ASN::ASSIGNMENT)
	{		
		if (node -> right -> type == ASN::CONSTANT)
		{
			if (node -> parent -> left)
			{
				bool declaration_needed = false;
				bool substituted = false;
				this -> setStatic(node -> parent -> left, node -> left, node -> right, &declaration_needed, &substituted);
				
				if (not declaration_needed and substituted)
				{
					//printf("(constant substituted) deleting line %d in file %s: %s = %g\n", node -> parent -> ivalue, node -> parent -> svalue, node -> left -> svalue, node -> right -> fvalue);
					
					node -> parent -> parent -> leftConnect(node -> parent -> left);
					node -> parent -> left = nullptr;
					delete node -> parent;
				}
				
				if (substituted)
					change_count++;
			}
		}
	}
	
	if (node -> right)
		change_count += this -> substituteStatic(node -> right);
	
	if (node -> left)
		change_count += this -> substituteStatic(node -> left);
	
	return change_count;
}


int Source::setStatic(ASN* node, ASN* variable, ASN* value, bool* declaration_needed, bool* substituted)
{	
	if (node -> type == ASN::VARIABLE and node -> LValue == true and !strcmp(node -> svalue, variable -> svalue))
		return -1;
	
	if (node -> type == ASN::CTRL_OPERATOR and node -> ivalue == ASN::WHILE)
	{
		int status = processBranching(node, variable);
		if (status == -1)
		{
			*declaration_needed = true;
			return -1;
		}
	}
	
	if (node -> right)
	{
		int status = this -> setStatic(node -> right, variable, value, declaration_needed, substituted);
		if (status == -1)
			return -1;
	}
	
	if (node -> type == ASN::VARIABLE and node -> LValue == false and !strcmp(node -> svalue, variable -> svalue))
	{
		node -> type = ASN::CONSTANT;
		node -> data_type = value -> data_type;
		node -> fvalue = value -> fvalue;
		node -> ivalue = value -> ivalue;
		*substituted = true;
	}
	
	if (node -> left)
	{
		int status = this -> setStatic(node -> left, variable, value, declaration_needed, substituted);
		if (status == -1)
			return -1;
	}
	
	return 0;
}


int Source::processBranching(ASN* node, ASN* variable)
{
	if (node -> left)
	{
		int status = processBranching(node -> left, variable);
		if (status == -1)
			return -1;
	}
	
	if (node -> type == ASN::VARIABLE and node -> LValue == true and !strcmp(node -> svalue, variable -> svalue))
		return -1;
	
	if (node -> right)
	{
		int status = processBranching(node -> right, variable);
		if (status == -1)
			return -1;
	}
	
	return 0;
}


int Source::foldConstants(ASN* node)
{
	int change_count = 0;
	
	if (node -> left)
		change_count += foldConstants(node -> left);
	
	if (node -> right)
		change_count += foldConstants(node -> right);
	
	if (node -> type == ASN::ARITHM_OPERATOR and node -> left and node -> right)
		change_count += foldArithmeticConstants(node);
	
	else if (node -> type == ASN::CMP_OPERATOR and node -> left and node -> right)
		change_count += foldCmpConstants(node);
	
	else if (node -> type == ASN::CTRL_OPERATOR and node -> left and node -> right)
		change_count += foldCtrlConstants(node);
	
	return change_count;
}


int Source::foldArithmeticConstants(ASN* node)
{
	int change_count = 0;
	
	switch (node -> ivalue)
	{
		case ASN::PLUS:
		{
			if (node -> left -> type == ASN::CONSTANT and node -> right -> type == ASN::CONSTANT)
			{
				node -> type = ASN::CONSTANT;
				node -> data_type = ASN::FLOAT;
				node -> fvalue = node -> left -> fvalue + node -> right -> fvalue;
				
				delete node -> left;
				node -> left = nullptr;
				
				delete node -> right;
				node -> right = nullptr;
				
				change_count++;
			}
			
			else if (node -> left -> type == ASN::CONSTANT and node -> left -> data_type == ASN::FLOAT and node -> left -> fvalue == 0)
			{
				ASN* right = node -> right;
				node -> right = nullptr;
				
				if (node -> parent -> left == node)
					node -> parent -> leftConnect(right);
				
				else if (node -> parent -> right == node)
					node -> parent -> rightConnect(right);
				
				delete node;
				
				change_count++;
			}

			else if (node -> right -> type == ASN::CONSTANT and node -> right -> data_type == ASN::FLOAT and node -> right -> fvalue == 0)
			{
				ASN* left = node -> left;
				node -> left = nullptr;
				
				if (node -> parent -> left == node)
					node -> parent -> leftConnect(left);
				
				else if (node -> parent -> right == node)
					node -> parent -> rightConnect(left);
				
				delete node;
				
				change_count++;
			}
		
			break;
		}
		
		case ASN::MINUS:
		{
			if (node -> left -> type == ASN::CONSTANT and node -> right -> type == ASN::CONSTANT)
			{
				node -> type = ASN::CONSTANT;
				node -> data_type = ASN::FLOAT;
				node -> fvalue = node -> left -> fvalue - node -> right -> fvalue;
				
				delete node -> left;
				node -> left = nullptr;
				
				delete node -> right;
				node -> right = nullptr;
				
				change_count++;
			}
			
			else if (node -> left -> type == ASN::CONSTANT and node -> left -> data_type == ASN::FLOAT and node -> left -> fvalue == 0)	///< unary "-" with "neg" command working incorrect
			{
				node -> left = node -> right;
				node -> right = nullptr;
				
				node -> ivalue = ASN::UNARY_MINUS;
			}
			
			else if (node -> right -> type == ASN::CONSTANT and node -> right -> data_type == ASN::FLOAT and node -> right -> fvalue == 0)
			{
				ASN* left = node -> left;
				node -> left = nullptr;
				
				if (node -> parent -> left == node)
					node -> parent -> leftConnect(left);
				
				else if (node -> parent -> right == node)
					node -> parent -> rightConnect(left);
				
				delete node;
				
				change_count++;
			}
			
			break;
		}
		
		case ASN::MULTIPLY:
		{
			if (node -> left -> type == ASN::CONSTANT and node -> right -> type == ASN::CONSTANT)
			{
				node -> type = ASN::CONSTANT;
				node -> data_type = ASN::FLOAT;
				node -> fvalue = node -> left -> fvalue * node -> right -> fvalue;
				
				delete node -> left;
				node -> left = nullptr;
				
				delete node -> right;
				node -> right = nullptr;
				
				change_count++;
			}
			
			else if (node -> left -> type == ASN::CONSTANT and node -> left -> fvalue == 1)
			{
				ASN* right = node -> right;
				node -> right = nullptr;
				
				if (node -> parent -> left == node)
					node -> parent -> leftConnect(right);
				
				else if (node -> parent -> right == node)
					node -> parent -> rightConnect(right);
				
				delete node;
				
				change_count++;
			}

			else if (node -> right -> type == ASN::CONSTANT and node -> right -> fvalue == 1)
			{
				ASN* left = node -> left;
				node -> left = nullptr;
				
				if (node -> parent -> left == node)
					node -> parent -> leftConnect(left);
				
				else if (node -> parent -> right == node)
					node -> parent -> rightConnect(left);
				
				delete node;
				
				change_count++;
			}
			
			else if ((node -> right -> type == ASN::CONSTANT and node -> right -> fvalue == 0) or (node -> left -> type == ASN::CONSTANT and node -> left -> fvalue == 0))
			{
				delete node -> right;
				node -> right = nullptr;
				
				delete node -> left;
				node -> left = nullptr;
				
				node -> type = ASN::CONSTANT;
				node -> data_type = ASN::FLOAT;
				node -> fvalue = 0;
				
				change_count++;
			}
			
			break;
		}
		
		case ASN::DIVIDE:
		{
			if (node -> left -> type == ASN::CONSTANT and node -> right -> type == ASN::CONSTANT)
			{
				node -> type = ASN::CONSTANT;
				node -> data_type = ASN::FLOAT;
				node -> fvalue = node -> left -> fvalue / node -> right -> fvalue;
				
				delete node -> left;
				node -> left = nullptr;
				
				delete node -> right;
				node -> right = nullptr;
				
				change_count++;
			}
			
			else if (node -> right -> type == ASN::CONSTANT and node -> right -> fvalue == 1)
			{
				ASN* left = node -> left;
				node -> left = nullptr;
				
				if (node -> parent -> left == node)
					node -> parent -> leftConnect(left);
				
				else if (node -> parent -> right == node)
					node -> parent -> rightConnect(left);
				
				delete node;
				
				change_count++;
			}
			
			else if (node -> left -> type == ASN::CONSTANT and node -> left -> fvalue == 0)
			{
				delete node -> right;
				node -> right = nullptr;
				
				delete node -> left;
				node -> left = nullptr;
				
				node -> type = ASN::CONSTANT;
				node -> data_type = ASN::FLOAT;
				node -> fvalue = 0;
				
				change_count++;
			}
			
			break;
		}
	}
	
	return change_count;
}


int Source::foldCmpConstants(ASN* node)
{
	int change_count = 0;
	
	switch (node -> ivalue)
	{
		case ASN::EQUAL:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue == node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
			
			break;
		}
		
		case ASN::NOT_EQUAL:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue != node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
			
			break;
		}
		
		case ASN::MORE:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue > node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
			
			break;
		}
		
		case ASN::MORE_EQ:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue >= node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
			
			break;
		}
		
		case ASN::LESS:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue < node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
			
			break;
		}
		
		case ASN::LESS_EQ:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue <= node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
			
			break;
		}
		
		case ASN::AND:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue and node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
				
				else if (node -> left -> type == ASN::FLOAT and node -> left -> fvalue == 0)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = 0;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
				
				else if (node -> right -> type == ASN::FLOAT and node -> right -> fvalue == 0)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = 0;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
						
			break;
		}
		
		case ASN::OR:
		{
			if (node -> left and node -> right)
			{
				if (node -> left -> type == ASN::FLOAT and node -> right -> type == ASN::FLOAT)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = node -> left -> fvalue or node -> right -> fvalue;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
				
				else if (node -> left -> type == ASN::FLOAT and node -> left -> fvalue == 1)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = 1;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
				
				else if (node -> right -> type == ASN::FLOAT and node -> right -> fvalue == 1)
				{
					node -> type = ASN::FLOAT;
					node -> fvalue = 1;
					
					delete node -> left;
					node -> left = nullptr;
					
					delete node -> right;
					node -> right = nullptr;
					
					change_count++;
				}
			}
						
			break;
		}
	}
	
	return change_count;
}


int Source::foldCtrlConstants(ASN* node) ///< TODO Valgrind maybe
{
	int change_count = 0;
	
	switch (node -> ivalue)
	{
		case ASN::IF:
		{
			if (node -> right -> type == ASN::FLOAT and node -> right -> fvalue == 0)
			{
				node -> parent -> parent -> leftConnect(node -> parent -> left);
				ASN* next_line = node -> parent -> left;
				
				node -> parent -> left = nullptr;
				delete node -> parent;
				
				if (next_line)
				{				
					if (next_line -> right -> type == ASN::CTRL_OPERATOR and next_line -> right -> ivalue == ASN::ELSE)
					{
						next_line -> parent -> leftConnect(next_line -> right -> left);
						
						ASN* line = next_line -> right -> left;
						if (line)
						{
							while (line -> left)
								line = line -> left;
						}
						
						line -> leftConnect(next_line -> left);
						
						next_line -> left = nullptr;
						next_line -> right -> left = nullptr;
						
						delete next_line;
					}
				}
				
				change_count++;
			}
			
			else if (node -> right -> type == ASN::FLOAT and node -> right -> fvalue != 0)
			{
				node -> parent -> parent -> leftConnect(node -> left);
				
				ASN* line = node -> left;
				
				node -> left = nullptr;
				
				if (line)
				{
					while (line -> left)
						line = line -> left;
				}
				
				line -> leftConnect(node -> parent -> left);
				
				ASN* next_line = node -> parent -> left;
				
				node -> parent -> left = nullptr;
				
				delete node -> parent;
				
				if (next_line)
				{
					if (next_line -> right -> type == ASN::CTRL_OPERATOR and next_line -> right -> ivalue == ASN::ELSE)
					{
						next_line -> parent -> leftConnect(next_line -> left);
						next_line -> left = nullptr;
						
						delete next_line;
					}
				}
				
				change_count++;
			}
			
			break;
		}
		
		case ASN::WHILE:
		{
			if (node -> right -> type == ASN::FLOAT and node -> right -> fvalue == 0)
			{
				node -> parent -> parent -> leftConnect(node -> parent -> left);
				node -> parent -> left = nullptr;
				delete node -> parent;
				
				change_count++;
			}
			
			break;
		}
	}
	
	return change_count;
}



int Source::inlineFunctions(ASN* node)
{
	int change_count = 0;
	return change_count;
}



int Source::deleteUnused(ASN* node)
{
	int change_count = 0;
	return change_count;
}



int Source::optimizeBuiltInFunctions(ASN* node)
{
	int change_count = 0;

	if (node -> type == ASN::FUNCCALL and !strcmp(node -> svalue, "int"))
	{
		if (node -> right -> right -> type == ASN::CONSTANT)
		{
			node -> type = ASN::CONSTANT;
			node -> data_type = ASN::INT;
			
			if (node -> right -> right -> data_type == ASN::FLOAT)
				node -> ivalue = round(node -> right -> right -> fvalue);
			
			else
				node -> ivalue = node -> right -> right -> ivalue;
			
			delete node -> right;
			node -> right = nullptr;
		}
		
		else if (node -> right -> right -> data_type == ASN::INT)
		{
			node -> type = node -> right -> right -> type;
			
			node -> ivalue = node -> right -> right -> ivalue;
			node -> fvalue = node -> right -> right -> fvalue;
			node -> cvalue = node -> right -> right -> cvalue;
			
			node -> svalue = node -> right -> right -> svalue;
			node -> right -> right -> svalue = nullptr;
			
			node -> LValue = node -> right -> right -> LValue;
			node -> vartype = node -> right -> right -> vartype;
			node -> enumerated = node -> right -> right -> enumerated;
			node -> data_type = node -> right -> right -> data_type;
			
			node -> line_content = node -> right -> right -> line_content;
			node -> right -> right -> line_content = nullptr;
			
			delete node -> right;
			node -> right = nullptr;
		}
		
		else if (node -> right -> right -> type == ASN::FUNCCALL and !strcmp(node -> right -> right -> svalue ? node -> right -> right -> svalue : "eRr0R", "input"))
		{
			delete[] node -> svalue;
			node -> svalue = new char[strlen("i_input") + 1]{0};
			strcpy(node -> svalue, "i_input");
			
			delete node -> right;
			node -> right = nullptr;
		}
		
	}
	
	if (node -> type == ASN::FUNCCALL and !strcmp(node -> svalue, "float"))
	{
		if (node -> right -> right -> type == ASN::CONSTANT)
		{
			node -> type = ASN::CONSTANT;
			node -> data_type = ASN::FLOAT;
			
			if (node -> right -> right -> data_type == ASN::FLOAT)
				node -> fvalue = node -> right -> right -> fvalue;
			
			else
				node -> fvalue = node -> right -> right -> ivalue;
			
			delete node -> right;
			node -> right = nullptr;
		}
		
		else if (node -> right -> right -> data_type == ASN::FLOAT)
		{
			node -> type = node -> right -> right -> type;
			
			node -> ivalue = node -> right -> right -> ivalue;
			node -> fvalue = node -> right -> right -> fvalue;
			node -> cvalue = node -> right -> right -> cvalue;
			
			node -> svalue = node -> right -> right -> svalue;
			node -> right -> right -> svalue = nullptr;
			
			node -> LValue = node -> right -> right -> LValue;
			node -> vartype = node -> right -> right -> vartype;
			node -> enumerated = node -> right -> right -> enumerated;
			node -> data_type = node -> right -> right -> data_type;
			
			node -> line_content = node -> right -> right -> line_content;
			node -> right -> right -> line_content = nullptr;
			
			delete node -> right;
			node -> right = nullptr;
		}
		
		else if (node -> right -> right -> type == ASN::FUNCCALL and !strcmp(node -> right -> right -> svalue ? node -> right -> right -> svalue : "eRr0R", "i_input"))
		{
			delete[] node -> svalue;
			node -> svalue = new char[strlen("input") + 1]{0};
			strcpy(node -> svalue, "input");
			
			delete node -> right;
			node -> right = nullptr;
		}
		
	}
	
	if (node -> left)
		this -> optimizeBuiltInFunctions(node -> left);
	
	if (node -> right)
		this -> optimizeBuiltInFunctions(node -> right);
	
	return change_count;
}

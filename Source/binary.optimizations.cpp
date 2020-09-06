#include "../Headers/binary.hpp"


void Binary::optimize()
{
	int changes_count = 0;
	return;
	changes_count += this -> deleteUnusedFunctions();
	changes_count += this -> foldPushPop();
	changes_count += this -> foldMovMov();
	changes_count += this -> deleteUselessOperations();
	
	if (changes_count != 0)
		this -> optimize();

	return;
}


int Binary::deleteUnusedFunctions()
{
	int changes_count = 0;
	
	Token* current = this -> start;
	
	while (current)
	{
		current = current -> next;
	}
	
	return changes_count;
}


int Binary::foldPushPop()
{
	int changes_count = 0;
	
	Token* current = this -> start;
	
	while (current)
	{
		if (current -> text and current -> next)
		{
			if (current -> next -> text)
			{
				if (!strncmp(current -> text, "push", 4) and !strncmp(current -> next -> text, "pop", 3))
				{
					char* new_command = new char[13]{0};
					strcpy(new_command, "mov ");
					strcat(new_command, (current -> next -> text) + 4);
					strcat(new_command, ", ");
					strcat(new_command, (current -> text) + 5);
					
					delete[] current -> text;
					current -> text = new_command;
					
					Token* deleting = current -> next;
					current -> next = deleting -> next;
					current -> next -> prev = current;
					
					deleting -> next = nullptr;
					delete deleting;
					changes_count++;
				}
			}
		}
		
		current = current -> next;
	}
	
	return changes_count;
}


int Binary::foldMovMov()
{
	int changes_count = 0;
	
	Token* current = this -> start;
	
	while (current)
	{
		if (current -> text and current -> next)
		{
			if (current -> next -> text)
			{
				if (!strncmp(current -> text, "mov", 3) and !strncmp(current -> next -> text, "mov", 3) and
					!strncmp((current -> text) + 4, (current -> next -> text) + 9, 3) and
					strlen(current -> text) == 12 and strlen(current -> next -> text) == 12)
				{
					char* new_command = new char[10 + strlen((current -> text) + 9)]{0};
					strcpy(new_command, "mov ");
					strncat(new_command, (current -> next -> text) + 4, 3);
					strcat(new_command, ", ");
					strcat(new_command, (current -> text) + 9);
					
					delete[] current -> text;
					current -> text = new_command;
					
					Token* deleting = current -> next;
					current -> next = current -> next -> next;
					current -> next -> prev = current;
					
					deleting -> next = nullptr;
					delete deleting;
					changes_count++;
				}
			}
		}
		
		current = current -> next;
	}
	
	return changes_count;
}


int Binary::deleteUselessOperations()
{
	int changes_count = 0;
	
	Token* current = this -> start;
	
	while (current)
	{
		if (current -> text)
		{
			if (!strncmp(current -> text, "mov", 3))
			{
				if (!strncmp((current -> text) + 4, (current -> text) + 9, 3))
				{
					current -> prev -> next = current -> next;
					current -> next -> prev = current -> prev;
					current -> next = nullptr;
					delete current;
					changes_count++;
				}
			}
			
			if ((!strncmp(current -> text, "add", 3) or !strncmp(current -> text, "sub", 3)) and current -> ivalue == 0)
			{
				current -> prev -> next = current -> next;
				current -> next -> prev = current -> prev;
				current -> next = nullptr;
				delete current;
				changes_count++;
			}
		}
		
		current = current -> next;
	}
	
	return changes_count;
}

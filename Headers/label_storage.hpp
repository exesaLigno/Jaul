#pragma once

#include <cstring>
#include <iostream>

class LabelsStorage
{
  private:
	char** names;
	long long int* positions;
	int* references_count;
	long long int** references;
	int labels_count;
	
  public:
	LabelsStorage();
	~LabelsStorage();
	
	int findLabel(const char* label_name);
	
	void setPosition(int label_number, long long int position);
	long long int getPosition(int label_number);
	
	void addReference(int label_number, long long int reference);
	int getReferencesCount(int label_number);
	long long int* getReferences(int label_number);
	void clearReferences(int label_number);
};

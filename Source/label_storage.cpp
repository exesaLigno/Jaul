#include "../Headers/label_storage.hpp"

LabelsStorage::LabelsStorage()
{
	names = 0;
	positions = 0;
	references_count = 0;
	references = 0;
	labels_count = 0;
}

LabelsStorage::~LabelsStorage()
{
	if (names)
	{
		for (int i = 0; i < labels_count; i++)
		{
			if (names[i])
				delete[] names[i];
		}
		
		delete[] names;
	}
	
	if (positions)
		delete[] positions;
		
	if (references_count)
		delete[] references_count;
		
	if (references)
	{
		for (int i = 0; i < labels_count; i++)
		{
			if (references[i])
				delete[] references[i];
		}
		
		delete[] references;
	}
}



int LabelsStorage::findLabel(const char* label_name)
{
	for (int i = 0; i < labels_count; i++)
	{
		if (!strcmp(names[i], label_name))
			return i;
	}
	
	
	int new_labels_count = labels_count + 1;
	int new_label = labels_count;
	
	char** new_names = new char*[new_labels_count];
	long long int* new_positions = new long long int[new_labels_count];
	int* new_references_count = new int[new_labels_count];
	long long int** new_references = new long long int*[new_labels_count];
	
	for (int i = 0; i < labels_count; i++)
	{
		new_names[i] = names[i];
		new_positions[i] = positions[i];
		new_references_count[i] = references_count[i];
		new_references[i] = references[i];
	}
	
	new_names[new_label] = new char[strlen(label_name) + 1];
	strcpy(new_names[new_label], label_name);
	
	new_positions[new_label] = 0;
	new_references_count[new_label] = 0;
	new_references[new_label] = 0;
	
	if (names)
		delete[] names;
	
	if (positions)
		delete[] positions;
		
	if (references_count)
		delete[] references_count;
		
	if (references)
		delete[] references;
		
	names = new_names;
	positions = new_positions;
	references_count = new_references_count;
	references = new_references;
	labels_count = new_labels_count;
	
	return new_label;	
}


void LabelsStorage::setPosition(int label_number, long long int position)
{
	positions[label_number] = position;
}


long long int LabelsStorage::getPosition(int label_number)
{
	return positions[label_number];
}


void LabelsStorage::addReference(int label_number, long long int reference)
{
	int old_references_count = references_count[label_number];
	long long int* old_references = references[label_number];
	
	int new_references_count = old_references_count + 1;
	long long int* new_references = new long long int[new_references_count];
	
	for (int i = 0; i < old_references_count; i++)
		new_references[i] = old_references[i];
	
	new_references[old_references_count] = reference;
	
	if (old_references)
		delete[] old_references;
		
	references[label_number] = new_references;
	references_count[label_number] = new_references_count;
}


int LabelsStorage::getReferencesCount(int label_number)
{
	return references_count[label_number];
}


long long int* LabelsStorage::getReferences(int label_number)
{
	return references[label_number];
}


void LabelsStorage::clearReferences(int label_number)
{
	delete[] references[label_number];
	references[label_number] = 0;
	references_count[label_number] = 0;
}


















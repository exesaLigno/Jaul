#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc <= 2)
	{
		std::cout << "usage: readBinary <elf64-file> <output_file>" << std::endl;
		return 0;
	}

	char* bin_text = 0;
	FILE* binary = fopen(argv[1], "r");
	
	fseek(binary, 0, SEEK_END);
	int text_length = ftell(binary);
	rewind(binary);

	bin_text = new char[text_length + 1]{0};

	text_length = fread(bin_text, sizeof(char), text_length, binary);
	
	fclose(binary);
	
	FILE* parsed = fopen(argv[2], "w");
	
	int counter = 0;
	
	fprintf(parsed, "%8u |  ", counter);
	
	while (counter < text_length)
	{
		unsigned char sym = *bin_text;
		fprintf(parsed, "%02x ", sym);
		if (counter % 4 == 3)
			fprintf(parsed, "   ");
		if (counter % 24 == 23)
			fprintf(parsed, "\n%8u |  ", counter + 1);
		bin_text++;
		counter++;
	}
		
	fclose(parsed);
	
	return 0;
}

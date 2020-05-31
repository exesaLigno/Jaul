CC=g++
CFLAGS=-c -std=c++2a -O3
LDFLAGS=
SOURCES=main.cpp Source/settings.cpp Source/programm.cpp Source/token.cpp Source/library.cpp Source/definition.cpp Source/label_storage.cpp
OBJECTS=$(SOURCES:.cpp=.o)
COMPILER=jc
ELFPARSER=relf

all: $(SOURCES) $(COMPILER) $(ELFPARSER)
	
	
$(COMPILER): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	
$(ELFPARSER):
	$(CC) $(LDFLAGS) Accessory/readBinary.cpp -o $(ELFPARSER)


.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
	
	
.PHONY: examples
examples: $(COMPILER)
	./jc Examples/fibonacci.jaul -s
	./jc Examples/power_of_two.jaul -s

	
.PHONY: clean
clean:
	rm -rf *.o Source/*.o $(COMPILER) $(ELFPARSER)
	
.PHONY: rm
rm:
	rm -rf *.png *.out *.s *.txt Examples/*.out Examples/*.png Examples/*.s Examples/*.txt
	
.PHONY: nasm
nasm:
	nasm -f elf64 test.s -o test.o
	ld -s -o a.out test.o

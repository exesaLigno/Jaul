# JAUL Compiler Pack

**JAUL** means **J**ust **A**nother **U**seless **L**anguage.

### Contents

* [About language - Syntax](#jc_syntax)

* [Using - Compiler](#jc_usage)  
  [Using - Running JVM](#jvm_usage)  
  [Compilation principle](#principle)  
  [Object files](#object)  
  [JAUL Virtual Mashine](#jvm)  
  [Assembly](#assembly)  
  [Examples](#examples)  
  [Performance tests](#performance)  

* [Source documentation](.docs/html/index.html)


<a name="jc_syntax"><h2>JAUL syntax bible</h2></a>

1. Blocks of code are separated with indents.

2. Syntax have simple dynamic-typization -- two types (integer and float) has 4 bytes length.
   
   Type redefinition avalible only in linear blocks - if you try to assign 
   float number to integer variable in cycle you get **assignment error**. 
   To redifine type in cycle and fix this error, use manual redefinition - 
   functions `float()` and `int()`
   
3. New functions declaring like this:
   
   ```
   def new_function(a, b, ...)
	   operator
	   operator
	   .
	   .
	   .
	   operator
	   return result
   ```
   or like this:
   
   ```
   def new_void_function(a, b, ...)
	   operator
	   operator
	   .
	   .
	   .
	   operator
   ```
   
   Also you can create functions without any parameters.
   
   **Beware!** Compiler doesn't check count and correctness of parameters in function call.
   
4. Language supports the following branching operators:
   
   **If-else branching:**  
   ```
   if statement
	   operator
	   .
	   .
	   .
   else
	   operator
	   .
	   .
	   .
   ```
   In `if-else` branching you can omit `else` branch.
   
   **While cycle:**  
   ```
   while statement
	   operator
	   operator
	   .
	   .
	   .
   ```
   
   **For cycle:**  
   ```
   repeat count times
	   operator
	   operator
	   .
	   .
	   .
   ```
   Here **_count_** - integer variable or integer number - count of reiterations,
   **_times_** - name of variable-counter in this cycle.
   
   You can also use conrol words **_break_** or **_continue_** in cycles to control
   execution flow
   
5. Default `input()` and `print()` functions are created for floats only.  
   To use those functions for ints, use `i_input()` and `i_print()`.
   
6. Supported operators:

   **Arithmetics:**  `+, -, *, /`  
   **Comparison:**   `>, >=, <, <=, ==, !=`  
   **Logic:**        `&&, ||`
   
7. Examples of working code you can find in [examples folder](Examples/)
   

<a name="jc_usage"><h2>Using - Compiler</h2></a>

**Usage**: `jc [options] file...`  
If compiler not installed to `/usr/bin/`, use `./jc [options] file...`

Compiler supports compilation options:

* `-v` or `--verbose` - verbose compilation (by default compiler shows only errors/warnings)  
  `-h` or `--help` - show help and exit

* `-p` or `--only_preprocess` - only preprocess and save preprocessed code to output path  
  `-l` or `--nasm_listing` - make NASM listing it to output path

* `--virtual` - compile code for [JVM](#jvm)  
  `--obj` - compile code and save it to [object file](#object)  
  If options `-p`, `-l`, `--virtual` or `--obj` not given, compiling directly into ELF-64 executable.

* `-o0`, `-o1`, `-o2` - optimization levels

* `-o <filename>` - set output file name  

Compiler takes three file types as input:
* **`.j`** - simple hi-level source ([Examples](#examples))
* **`.jo`** - [object files](#object)
* **`.s`** - special JAUL assembler language (Example of .jasm files at .std folder)


<a name="jvm_usage"><h2>Using - JVM</h2></a>

Using of [JVM](#jvm)

<h3><span style = "color: red">JVM is not implemented!</style></h3>



<a name="principle"><h2>Compilation principle</h2></a>

All source files importing to *Source* objects.

* **JAUL Source**  
  First of all, *Source* class building *Abstract Syntax Tree* and by which compiling instructions to *Binary* class object. 
  After that *Binary* object optimizing and calculating local labels, byte code unites function by function 
  (one function in one *Binary::Token*)

* **JAUL Object**  
  *Source* class just keeping byte code of object file, after that transmits it to *Binary* object. 
  It builds object file function by function (one function in one *Binary::Token*).

* **JAUL Assebler Language**  
  It works like object files, but building *Binary* object basing on JASM code 
  and after that optimizing and calculating local labels like in JAUL source compilation.

When all *Binary* objects are compiled, optimized and calculated, compiler linking them to one *Binary* object with all functions.
If object generation choosen, this *Binary* object just saving in file ([Object files](#object))
Else compiler calculating function labels in linked *Binary*, generating ELF-64 (or JVM) header and
saving all compiled code to executable file.


<a name="object"><h2>Object files</h2></a>

Object files contains pre-compiled code and function names for linker.

First 4 bytes is `JAUL`, next 8 bytes is *long int* with length of subsequent executable bytecode.

Bytecode contains name of function and bytecode of this function.

|      | Name length |  Name   | Bytes count |  Bytecode |
| ----:|:-----------:|:-------:|:-----------:|:---------:|
| Size |   4 bytes   |   ???   |   8 bytes   |    ???    |
| Type |    `int`    | `char*` | `long int`  |  `char*`  |


You can make your own object library from assembler with **_makeobj_** tool from [Accessory](Accessory/makeObject/)  
Information about usage and requirements of this tool you can read by the link above.


<a name="jvm"><h2>JAUL Virtual Mashine</h2></a>

JAUL Virtual Mashine
<h3><span style = "color: red">JVM is not implemented!</style></h3>


<a name="assembly"><h2>Assembly</h2></a>

Compile JC: `make jc`  
Compile JVM: `make jvm`  
Compile both: `make jcp`

If you want to install **JAUL Compiler Pack** to `/usr/bin` type `sudo make install`

`make` builds all programms in repository (inlcuding [readBinary](Accessory/ReadBinary)).

`make clean` - Delete all compilation results  
`sudo make uninstall` - Uninstall all programms and delete all compilation results


<a name="examples"><h2>Examples</h2></a>

Examples of working code you can find in [examples](Examples/) folder.


<a name="performance"><h2>Performance tests</h2></a>




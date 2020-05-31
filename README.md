# **J**ust **A**nother **U**seless **L**anguage compiler

Programm compiling Hi-level source code into NASM or directly into binary ELF-64 file.
To assembly programm use `make` or `make jc` in repo directory. 
`make` building all programms in repository inlcuding readBinary ([see more](/Accessory/README.md)).


## Using

Compiler supports compilation keys:
`-s` - silent compilation,
`-p` - only preprocess,
`-l` - make NASM listing,
`-o <filename>` - set output file name to `<filename>`.
If keys `-p` or `-l` not given, compiling directly into ELF-64.

Examples of programms on jaul are given in "[Examples](/Examples)" folder.

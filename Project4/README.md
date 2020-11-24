
# Project 4
Your task is to implement the genroutinecallop() method in the stmt.c (line 189). This method handles function invocations during the code generation phase. 

## Install Dependencies
    sudo apt update
    sudo apt install build-essential # Install gcc, g++, make, etc. 
    sudo apt-get install flex # Install flex
    sudo apt-get install bison # Install Bison (2.4.1 or older)
    sudo apt-get install byacc # Install Berkeley Yacc (1.9 or older)
    gcc --version # Confirm gcc installation
    flex --version # Confirm flex installation
    yacc --version # Confirm Yacc version
In case you want to use department resources, turn on the Plus Secure VPN, and ssh to **germanium.cs.pitt.edu**. Above packages are already installed on this machine. 

## Install
    make 

## Uninstall
    make clean

## Run
    chmod +x spim.linux # Give execute permission to the simulator (run this only one time)
    ./main examples/src1.txt && ./spim.linux -file code.s

## Files
    lex.l: Input of lex
    table.c: Hash table and string table
    grammar.y: Grammar for YACC
    proj2.c & proj2.h: Library to print syntax tree
    proj3.c & proj3.h: Library to print semantic tree
    seman.c: Semantic analyzer
    stmt.c: Register allocator
    codegen.c: code generation
    go: Executable file
    Makefile: Make file

## Submission
Make a your_pitt_id.[zip|tar|tar.gz] archive, e.g., moh18.zip from your project and submit it by the due date. 

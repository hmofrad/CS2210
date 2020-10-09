
# Project 2
Your task is to complete the grammar for the if-statement (line 289 in grammar.y).

## Install Dependencies
    sudo apt update
    sudo apt install build-essential # Install gcc, g++, make, etc. 
    sudo apt-get install flex # Install flex
	sudo apt-get install bison # Install Bison
	sudo apt-get install byacc # Install Berkeley Yacc
    gcc --version # Confirm gcc installation
    flex --version # Confirm flex installation
    yacc --version # Confirm Yacc version
In case you want to use department resources, turn on the Plus Secure VPN, and ssh to **germanium.cs.pitt.edu**. Above packages are already installed on this machine. 

## Install
    make 

## Uninstall
    make clean

## Run
    ./go examples/hello.txt

## Files
    lex.l: Input of lex
    table.c: Hash table and string table
    grammar.y: Grammar for YACC
    proj2.c & proj2.h: Library to print syntax tree
    parserdrv.c: Temporary driver
    go: Executable file
    Makefile: Make file

## Submission
Make a your_pitt_id.[zip|tar|tar.gz], e.g., moh18.zip archive from your project and submit it by the due date. 
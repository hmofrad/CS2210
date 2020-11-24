
# Project 3
Your task is to complete seman.c for recognizing (un)defined variables (line 246 in seman.c).

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
    ./go examples/err1.txt

## Files
    lex.l: Input of lex
    table.c: Hash table and string table
    grammar.y: Grammar for YACC
    proj2.c & proj2.h: Library to print syntax tree
    proj3.c & proj3.h: Library to print semantic tree
    go: Executable file
    Makefile: Make file

## Submission
Make a your_pitt_id.[zip|tar|tar.gz] archive, e.g., moh18.zip from your project and submit it by the due date. 

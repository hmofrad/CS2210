# Project 1
The project is already implemented. This is to help you easily configure and install it.

## Install Dependencies
    sudo apt update
    sudo apt install build-essential # Installs gcc, g++, make, etc. 
    sudo apt-get install flex # Installs flex
    gcc --version # Confirm gcc installation
    flex --version # Confirm flex installation
In case you want to use department resources, turn on the Plus Secure VPN, and ssh to **germanium.cs.pitt.edu**. Above packages are already installed on this machine. 

## Install
    make 

## Unistall
    make clean

## Run
    ./go test.txt

## Files
    lex.l: Input of lex
    table.c: Hash table and string table
    token.h: Defines token numbers
    lexdrv.c: Temporary driver
    Makefile: Make file
    go: Executable file

## Notes
    Comments in this form is treated as correct:
        /*....../*........*/
        ^------------------^match
    Nested comments are considered as:
        /*....../*........*/........*/
        ^------------------^---------^
        match      comment with no beginning

## Submission
    Your task is to have a look at the source files and try to understand what is happening. 
    No submission is required for this project

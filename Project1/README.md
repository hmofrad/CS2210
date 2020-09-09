# Project 1
The project is already implemented. This is to help you easily configure and install the project.
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
    Input of lex: lex.l
    Hash table and string table: table.c
    Defines token numbers: token.h
    Temporary driver: lexdrv.c
    Make file: Makefile
    Executable file: go

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
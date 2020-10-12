#!/usr/bin/make

all:
	flex lex.l
	yacc -v grammar.y
	gcc -o parserdrv.o -c parserdrv.c 
	gcc -o y.tab.o -c y.tab.c 
	gcc -o proj2.o -c proj2.c
	gcc -o go y.tab.o proj2.o parserdrv.o -lfl

clean:
	rm -rf lex.yy.o lex.yy.c y.tab.c y.tab.o y.output proj2.o parserdrv.o go 

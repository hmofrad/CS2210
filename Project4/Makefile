#!/bin/make

main:   codegen.c seman.c stmt.c proj3.c proj2.o table.c y.tab.c lex.yy.c
	gcc  -g -o main codegen.c seman.c stmt.c proj3.c proj2.o table.c y.tab.c -lfl

proj2.o: proj2.c proj2.h
	gcc -c proj2.c

y.tab.c:  grammar.y
	yacc -v grammar.y

lex.yy.c: lex.l
	flex lex.l

clean:
	rm -rf lex.yy.c y.tab.c y.output proj2.o main code.s 

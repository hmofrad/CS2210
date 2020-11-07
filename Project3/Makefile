#!/usr/bin/make

all:
	flex lex.l
	yacc -v grammar.y
#	gcc -o driver.o -c driver.c
#	gcc -o y.tab.o -c y.tab.c 
#	gcc -o proj2.o -c proj2.c
#	gcc -o proj3.o -c proj3.c
#	gcc -o seman.o -c seman.c 
	gcc -o go y.tab.c table.c seman.c proj2.c proj3.c -lfl

clean:
	rm -rf lex.yy.o lex.yy.c y.tab.c y.tab.o y.output proj2.o proj3.o seman.o go 

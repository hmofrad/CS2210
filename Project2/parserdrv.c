/*
  Copyright (C) Mohammad Hasanzadeh Mofrad - All Rights Reserved
  Course: CS 2210 - Compiler Design (Spring 2016)
  Project: Mini Java - part 2 - Syntax Analyzer
  File: Parser Driver engine
  Author: Mohammad Hasanzadeh Mofrad
  Email: hasanzadeh@cs.pitt.edu
  Date: February 2016; Last revision 02-25-2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int yyparse ();
int yycolumn, yyline, yyleng;
extern char* yytext;
extern void* yyin;
FILE *treelst;

int main(int argc, char *argv[])
{
    char* file_name = argv[1];
    FILE *fid = fopen(file_name, "rb");
    if(fid==NULL) { printf("File %s does not exist!\n", file_name); exit(1); }
    yyin = fid;

    yyline = 1;  yycolumn = 0;
    treelst = stdout;
    yyparse();
    return(0);
}

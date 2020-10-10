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

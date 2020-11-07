%{
/* #include <stdlib.h> */
/* #include "token.h" */

extern int yycolumn, yylength, yyline;
extern void install_id(); /* install string constants & identifiers into string tbl */
%}

/* regular definitions */
delim         [ \t\r]
ws            {delim}+
newline       [\n]
declarations [dD][eE][cC][lL][aA][rR][aA][tT][iI][oO][nN][sS]
enddeclarations [eE][nN][dD][dD][eE][cC][lL][aA][rR][aA][tT][iI][oO][nN][sS]
iint         [iI][nN][tT]
method      [mM][eE][tT][hH][oO][dD]
program     [pP][rR][oO][gG][rR][aA][mM]
val         [vV][aA][lL]
while       [wW][hH][iI][lL][eE]
class       [cC][lL][aA][sS][sS]
else        [eE][lL][sS][eE]
if          [iI][fF]
return      [rR][eE][tT][uU][rR][nN]
void        [vV][oO][iI][dD]
identifier    [A-Za-z][A-Za-z0-9]*
int           [0-9]+
errid         [0-9]+{identifier}
strcnst       '([^'\n\\]*(\\.)?)*'
errstr        '([^'\n\\]*(\\.)?)*[\n]
slashstar     \/\*
errcomm1      \*\/

%%
{ws}                {
                      int i;
                      for ( i=0; i<yyleng; i++ )
                      {
                        if ( yytext[i]=='\t' )
                          yycolumn = yycolumn + 8 - (yycolumn&0x7);
                        else
                          yycolumn++;
                      }
                    }
{newline}           {yyline++; yycolumn=0;}
"&&"                {yycolumn += yyleng; return(ANDnum);}
":="                {yycolumn += yyleng; return(ASSGNnum);}
{declarations}      {
                     yycolumn += yyleng; 
                     return(DECLARATIONSnum);
                    } 
"."                 {yycolumn += yyleng; return(DOTnum);}
{enddeclarations}   {
                     yycolumn+= yyleng;
                     return(ENDDECLARATIONSnum);
                    }
"="                 {yycolumn += yyleng; return(EQUALnum);}
">"                 {yycolumn += yyleng; return(GTnum);}
{method}            {yycolumn += yyleng; tolowercase(); return(METHODnum);}
{program}           {yycolumn += yyleng; tolowercase(); return(PROGRAMnum);}
{val}               {yycolumn += yyleng; tolowercase(); return(VALnum);}
{while}             {yycolumn += yyleng; tolowercase(); return(WHILEnum);}
{class}             {yycolumn += yyleng; tolowercase(); return(CLASSnum);}
{else}              {yycolumn += yyleng; tolowercase(); return(ELSEnum);}
{if}                {yycolumn += yyleng; tolowercase(); return(IFnum);}
{iint}              {yycolumn += yyleng; tolowercase(); return(INTnum);}
{return}            {yycolumn += yyleng; tolowercase(); return(RETURNnum);}
{void}              {yycolumn += yyleng; return(VOIDnum);}
{identifier}        {
                     yycolumn += yyleng;
                     tolowercase();
                     install_id(yytext, IDnum);
                     return(IDnum);}
{errid}             {ReportError("wrong identifier"); yycolumn += yyleng;}
"["                 {yycolumn += yyleng; return(LBRACnum);}
"("                 {yycolumn += yyleng; return(LPARENnum);}
"!="                {yycolumn += yyleng; return(NEnum);}
"||"                {yycolumn += yyleng; return(ORnum);}
"]"                 {yycolumn += yyleng; return(RBRACnum);}
")"                 {yycolumn += yyleng; return(RPARENnum);}
";"                 {yycolumn += yyleng; return(SEMInum);}
","                 {yycolumn += yyleng; return(COMMAnum);}
"/"                 {yycolumn += yyleng; return(DIVIDEnum);}
"=="                {yycolumn += yyleng; return(EQnum);}
">="                {yycolumn += yyleng; return(GEnum);}
{int}               {yycolumn += yyleng; yylval.intg = atoi(yytext); return(ICONSTnum);}
"{"                 {yycolumn += yyleng; return(LBRACEnum);}
"<="                {yycolumn += yyleng; return(LEnum);}
"<"                 {yycolumn += yyleng; return(LTnum);}
"-"                 {yycolumn += yyleng; return(MINUSnum);}
"!"                 {yycolumn += yyleng; return(NOTnum);}
"+"                 {yycolumn += yyleng; return(PLUSnum);}
"}"                 {yycolumn += yyleng; return(RBRACEnum);}
{strcnst}           {yycolumn += yyleng;
                     install_id( yytext, yyleng);
                     return(SCONSTnum);}
{errstr}            {ReportError("string doesn't end"); yyline++; yycolumn=0;}
"*"                 {yycolumn += yyleng; return(TIMESnum);}
{slashstar}         { yycolumn += yyleng; match(); }
{errcomm1}          {yycolumn++; ReportError("comment without beginning");}
.                   {ReportError("unknown string"); printf("--%s**\n", yytext); while (input() != '\n'); }
%%
tolowercase()
{
/* transform an identifier to lower case representation */
  int i;
  for( i=0; i<yyleng; i++ )
  {
     if ( 'A'<=yytext[i] && yytext[i]<='Z' )
        yytext[i] = yytext[i] - 'A' + 'a';
  }
}

ReportError(char* msg)
{
  printf("ERROR at line %d column %d: %s\n", yyline, yycolumn, msg);
}

match()
{
/* trying to find the ending of the comment, print error msg when necessary */
   char nextchar;

   do{
     nextchar = input();
     yycolumn++;
     switch ( nextchar ){
     case '*':  nextchar = input();
                if ( nextchar == '/')
                {
                  yycolumn++;
                  return;
                }
                break;
     case '\n': yyline++;
                yycolumn=0;
                break;
     case 0:    ReportError("Fatal error: unexpected EOF found in comment");
                exit(0);
     default:   break;
     }
   } while(1);
}
         

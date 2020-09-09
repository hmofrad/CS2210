#include <stdio.h>
#include <stdlib.h>
#include "token.h"
 
extern  int yylex();
extern  void prt_hash_tbl();
extern  void prt_string_tbl();
extern  void init_hash_tbl();
extern  void init_string_tbl();
int yyline, yycolumn, yylval;
extern void* yyin;

void main(int argc, char *argv[])
{
  if(argc!=2) { printf("USAGE: %s <input>\n", argv[0]); exit(1); }
    
  char* file_name = argv[1];
  FILE *fid = fopen(file_name, "rb");
  if(fid==NULL) { printf("File %s does not exist!\n", file_name); exit(1); }
  yyin = fid;
    
  int lexrtn;
  char* tokenid[40]={"EOFnum", "ANDnum", "ASSGNnum", 
                     "DECLARATIONSnum", "DOTnum", "ENDDECLARATIONSnum",
                     "EQUALnum", "GTnum", "IDnum", "INTnum", "LBRACnum",
                     "LPARENnum", "METHODnum", "NEnum", "ORnum", "PROGRAMnum",
                     "RBRACnum", "RPARENnum", "SEMInum", "VALnum", "WHILEnum",
                     "CLASSnum", "COMMAnum", "DIVIDEnum", "ELSEnum", "EQnum",
                     "GEnum", "ICONSTnum", "IFnum", "LBRACEnum", "LEnum", "LTnum",
                     "MINUSnum", "NOTnum", "PLUSnum", "RBRACEnum", "RETURNnum",
                     "SCONSTnum", "TIMESnum", "VOIDnum" };

  init_hash_tbl();
  init_string_tbl();
  yyline = 1;  yycolumn = 0;
  printf("Line\tColumn\t%-16s\tyylval\n", "Token");
  while( 1 )
  {
     switch ( lexrtn = yylex() ){
     default:        printf("%d\t%d\t%-16s\n",yyline,yycolumn, tokenid[lexrtn - 256]); break;
     case ICONSTnum: printf("%d\t%d\t%-16s\t%d\n",yyline,yycolumn, tokenid[lexrtn - 256], yylval);
                     break;
     case SCONSTnum:
     case IDnum:     printf("%d\t%d\t%-16s\t%d\n",yyline,yycolumn, tokenid[lexrtn - 256], yylval); 
                     break;
     case 0:         printf("%d\t%d\t%-16s\n",yyline,yycolumn, tokenid[0]);  /* end of file incurred */
                     /*printf("\nHash Table :\n");
                     prt_hash_tbl();  */
                     printf("\nString Table :\n");
                     prt_string_tbl();
                     exit(1);
     }
  }
}

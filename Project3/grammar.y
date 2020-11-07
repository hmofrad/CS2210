%{
#include  "proj2.h"
#include  <stdio.h>

  tree type_record, type_method, argtype;  /* globals used to store treenode pointers */

extern  tree  SyntaxTree;
%}

%union{
  int intg;
  tree tptr;
  }
  
%token <intg>  PROGRAMnum IDnum SEMInum CLASSnum  DECLARATIONSnum  ENDDECLARATIONSnum
%token <intg>  COMMAnum EQUALnum LBRACEnum RBRACEnum LBRACnum RBRACnum LPARENnum RPARENnum VOIDnum
%token <intg>  INTnum METHODnum VALnum DOTnum ASSGNnum RETURNnum IFnum ELSEnum WHILEnum
%token <intg>  LTnum LEnum EQnum NEnum GEnum GTnum PLUSnum MINUSnum ORnum TIMESnum DIVIDEnum ANDnum
%token <intg>  NOTnum ICONSTnum SCONSTnum

%type  <tptr>  Program ClassDecl_rec ClassDecl ClassBody MethodDecl_z1 MethodDecl_rec Decls
%type  <tptr>  FieldDecl_rec FieldDecl Tail FieldDecl_body VariableDeclId Bracket_rec1 Bracket_rec2
%type  <tptr>  VariableInitializer ArrayInitializer ArrayInitializer_body  ArrayCreationExpression
%type  <tptr>  ArrayCreationExpression_tail MethodDecl FormalParameterList_z1 FormalParameterList
%type  <tptr>  FormalParameterList_rec IDENTIFIER_rec Block Type Type_front 
%type  <tptr>  StatementList Statement_rec Statement AssignmentStatement MethodCallStatement
%type  <tptr>  MethodCallStatement_tail Expression_rec ReturnStatement IfStatement If_rec WhileStatement
%type  <tptr>  Expression Comp_op SimpleExpression Term Factor Expression_rec2
%type  <tptr>  UnsignedConstant Variable Variable_tail Variable_rec Variable_1 


%%/* yacc specification*/
Program          :      PROGRAMnum IDnum SEMInum ClassDecl_rec
                        {  
                          $$ = MakeTree(ProgramOp, $4, NullExp()); 
                          SyntaxTree = $$;
                          /* printtree($$, 0); */
                        }
                 ;
ClassDecl_rec    :      ClassDecl                        /* 1 or More of ClassDecl */
                          {  $$ = MakeTree(ClassOp, NullExp(), $1); } 
                 |      ClassDecl_rec ClassDecl
			  {  $$ = MakeTree(ClassOp, $1, $2); }
                 ;
ClassDecl        :      CLASSnum IDnum ClassBody
                          {  $$ = MakeTree(ClassDefOp, $3, MakeLeaf(IDNode, $2)); }
                 ;
ClassBody        :      LBRACEnum MethodDecl_z1 RBRACEnum
                          {  $$ = $2; }
                 |      LBRACEnum Decls MethodDecl_z1  RBRACEnum
                          {  /*tree tt;
			       tt = MakeTree(BodyOp, NullExp(), $2);*/

                             if ( NodeKind($3) == DUMMYNode )/* $3 is a NULL node */
                               $$ = $2;
                             else/* $2 is the left child of $3 */
			       $$ = MkLeftC($2, $3);
			  }
                 ;
MethodDecl_z1    :        
/* root is BodyOp */      {  $$ = NullExp(); }
                 |      MethodDecl_rec      
                          {  $$ = $1; } 
                 ;
MethodDecl_rec   :      MethodDecl
                           {  $$ = MakeTree(BodyOp, NullExp(), $1); }      
                 |      MethodDecl_rec MethodDecl
                           {  $$ = MakeTree(BodyOp, $1, $2); }
                 ;
Decls            :      DECLARATIONSnum ENDDECLARATIONSnum
/* root is BodyOp */       {  $$ = MakeTree(BodyOp, NullExp(), NullExp()); }
                 |      DECLARATIONSnum FieldDecl_rec ENDDECLARATIONSnum
/* root is BodyOp */       {  $$ = $2; }
                 ;
FieldDecl_rec    :      FieldDecl                      
                           {  $$ = MakeTree(BodyOp, NullExp(), $1); }
                 |      FieldDecl_rec FieldDecl
                           {  $$ = MakeTree(BodyOp, $1, $2); }
                 ;
FieldDecl        :      Type { type_record = $1; } FieldDecl_body SEMInum 
/* root is DeclOp */       {  $$ = $3; /* $3 is FieldDecl_body */ }
                 ;
Tail             :      VariableDeclId
/* root is CommaOp */      { 
                              tree commat;
                              commat = MakeTree(CommaOp, type_record, NullExp());
                              $$ = MakeTree(CommaOp, $1, commat);
                           }
                 |      VariableDeclId EQUALnum VariableInitializer
                           {
                              tree commat;
                              commat = MakeTree(CommaOp, type_record, $3);
                              $$ = MakeTree(CommaOp, $1, commat);
                           }
                 ;
FieldDecl_body   :      Tail
/* root is DeclOp */      {  $$ = MakeTree(DeclOp, NullExp(), $1);  }
                 |      FieldDecl_body COMMAnum Tail
                           {  $$ = MakeTree(DeclOp, $1, $3);  }
                 ;
VariableDeclId   :      IDnum
                           {  $$ = MakeLeaf(IDNode, $1); }
                 |      IDnum Bracket_rec1
                           {   $$ = MakeLeaf(IDNode, $1); /* "[]" do not construct tree */ }
                 ;
Bracket_rec1     :      LBRACnum RBRACnum { $$ = NullExp(); }
                 |      Bracket_rec1 LBRACnum RBRACnum { $$ = NullExp(); }
                 ;
VariableInitializer      :   Expression  
                               {  $$ = $1; }
                         |   ArrayInitializer 
                               {  $$ = $1; }
                         |   ArrayCreationExpression  
                               {  $$ = $1; }
                         ;
ArrayInitializer         :   LBRACEnum ArrayInitializer_body RBRACEnum
/* root is ArrayTypeOp */      {  $$ = MakeTree(ArrayTypeOp, $2, type_record); }
                         ;
ArrayInitializer_body    :   VariableInitializer
/* root is CommaOp */          { $$ = MakeTree( CommaOp, NullExp(), $1); }
                         |   ArrayInitializer_body COMMAnum VariableInitializer
/* root is CommaOp */          { $$ = MakeTree( CommaOp, $1, $3 ); }
                         ;
ArrayCreationExpression  :   INTnum ArrayCreationExpression_tail
/* root is ArrayTypeOp */      {  $$ = MakeTree( ArrayTypeOp, $2, MakeLeaf(INTEGERTNode, $1) ); }
                         ;
ArrayCreationExpression_tail  :  LBRACnum Expression RBRACnum
/* root is BoundOp */             { $$ = MakeTree( BoundOp, NullExp(), $2 ); }
                              |  ArrayCreationExpression_tail LBRACnum Expression RBRACnum
/* root is BoundOp */             { $$ = MakeTree( BoundOp, $1, $3 ); }
                              ;
MethodDecl     :    METHODnum Type {type_method=$2;} IDnum LPARENnum FormalParameterList_z1 RPARENnum Block 
/* root is MethodOp */
                      { 
                        tree head;
                        head = MakeTree(HeadOp, MakeLeaf(IDNode, $4), $6);
                        $$ = MakeTree(MethodOp, head, $8);
                      }
               |    METHODnum VOIDnum{type_method=NullExp();} IDnum LPARENnum FormalParameterList_z1 RPARENnum Block 
                      { 
                        tree head;
                        head = MakeTree(HeadOp, MakeLeaf(IDNode, $4), $6);
                        $$ = MakeTree(MethodOp, head, $8);
                      }
               ;
FormalParameterList_z1        :
                                   {  $$ = MakeTree(SpecOp, NullExp(), type_method); }
                              |  FormalParameterList
                                   {  $$ = MakeTree(SpecOp, $1, type_method); }
                                 /*type_method is the return type of method, global*/
                              ;
FormalParameterList           :  FormalParameterList_rec
/* root is VArgTypeOp*/            { $$ = $1; }
/*  or RArgTypeOp */          |  FormalParameterList SEMInum FormalParameterList_rec
                                   { $$ = MkRightC($3, $1); } /* $3 is the right child of $1 */
                              ;
FormalParameterList_rec       :  INTnum 
/*root is RArgTypeOp*/              {
                                      argtype = MakeTree(RArgTypeOp, NullExp(), NullExp());
                                      $$ = MakeTree(RArgTypeOp, NullExp(), argtype);/* lchild is unknown*/
                                    }
                                 IDENTIFIER_rec
                                    { 
                                      SetLeftChild($2, $3);
                                      if ( NodeKind(LeftChild(RightChild($2))) == DUMMYNode )/* no comma */
                                         SetRightChild( $2, NullExp() );
                                      else
					{
					  tree t = $2; /* head of the tree */
                                          /* delete the tangling right most child */
                                          while ( t->RightC != argtype )
                                                 t = t->RightC;
                                          t->RightC = NullExp();
                                          free( argtype );
                                        }
                                      $$ = $2;  /* return to FormalParameterList_rec */                  
                                    }
                                  /* right child is to be put in by IDENTIFIER_rec  */
                              |  VALnum INTnum 
/*root is VArgTypeOp*/              {
                                      argtype = MakeTree(VArgTypeOp, NullExp(), NullExp());
                                      $$ = MakeTree(VArgTypeOp, NullExp(), argtype);/* lchild is unknown*/
				    }
                                 IDENTIFIER_rec
                                    {
                                      SetLeftChild($3, $4); 
                                      if ( NodeKind(LeftChild(RightChild($3))) == DUMMYNode )/* no comma */
                                         SetRightChild( $3, NullExp() );  
                                      else
					{
					  tree t = $3; /* head of the tree */
                                          /* delete the tangling right most child */
                                          while ( t->RightC != argtype )
                                                 t = t->RightC;
                                          t->RightC = NullExp();
                                          free( argtype );
                                        }
                                      $$ = $3;  /* return to FormalParameterList_rec */                  
			            }
                                  /* right child is to be put in by IDENTIFIER_rec */
                              ;
IDENTIFIER_rec                :  IDnum
/* root is CommaOp */              { $$ = MakeTree(CommaOp, MakeLeaf(IDNode,$1), MakeLeaf(INTEGERTNode,0)); }
                              |  IDENTIFIER_rec COMMAnum IDnum
                                  { 
                                    tree comma, temp;

                                    $$ = $1;
                                    comma = MakeTree(CommaOp, MakeLeaf(IDNode,$3), MakeLeaf(INTEGERTNode,0));
                                    MkLeftC( comma, argtype );
                                    temp = MakeTree(NodeOp(argtype), NullExp(), NullExp());
                                    SetRightChild(argtype, temp);
                                    argtype = temp;
                                  }
                              ;
Block          :    StatementList 
/* root is BodyOp */  { $$ = MakeTree(BodyOp, NullExp(), $1); }
               |    Decls StatementList 
/* root is BodyOp */  { $$ = MakeTree(BodyOp, $1, $2); }
               ;
Type           :    Type_front
/* root is a leaf */   { $$ = MakeTree(TypeIdOp, $1, NullExp()); }  
               |    Type_front Bracket_rec2
/* root is TypeIdOp */ { $$ = MakeTree(TypeIdOp, $1, $2); }  
               |    Type_front Bracket_rec2 DOTnum Type
/* root is TypeIdOp */ { 
                         tree temp;
                         temp = MakeTree(TypeIdOp, $1, $2);
                         $$ = MkRightC( $4, temp );
		       }
               ;
Type_front     :    IDnum                          
                     { $$ = MakeLeaf(IDNode, $1); }
               |    INTnum                         
                     { $$ = MakeLeaf(INTEGERTNode, $1);}
               ;                                   
Bracket_rec2   :    LBRACnum RBRACnum
/* root is IndexOp */ { $$ = MakeTree(IndexOp, NullExp(), NullExp()); }
               |    LBRACnum RBRACnum Bracket_rec2
/* root is IndexOp */ { $$ = MakeTree(IndexOp, NullExp(), $3); }
               ;
StatementList  :    LBRACEnum Statement_rec RBRACEnum
/* root is StmtOp */  { $$ = $2; }
               ;
Statement_rec  :    Statement
/* root is StmtOp */  { $$ = MakeTree(StmtOp, NullExp(), $1);  } 
               |    Statement_rec SEMInum Statement
                      { 
                        if ( NodeKind($3) == DUMMYNode )
                           $$ = $1;
                        else
                           $$ = MakeTree(StmtOp, $1, $3);
                      }
               ;
Statement      :   
/* root is NullExp() */       { $$ = NullExp(); }
               |    AssignmentStatement
/* root is AssignOp */        { $$ = $1; }
               |    MethodCallStatement
/* root is RoutineCallOp */   { $$ = $1; }
               |    ReturnStatement
/* root ReturnOp */           { $$ = $1; }
               |    IfStatement
/* root is IfElseOp */        { $$ = $1; }
               |    WhileStatement
/* root is LoopOp */          { $$ = $1; }
               ;
AssignmentStatement :   Variable ASSGNnum Expression
/* root is AssignOp */    { $$ = MakeTree(AssignOp, MakeTree(AssignOp, NullExp(), $1), $3); }
                    ;
MethodCallStatement :   Variable MethodCallStatement_tail
/* root is RoutineCallOp */ { $$ = MakeTree(RoutineCallOp, $1, $2); }
                    ;
MethodCallStatement_tail   :   LPARENnum Expression_rec RPARENnum
/* root is CommaOp */            { $$ = $2; }
                           |   LPARENnum RPARENnum
                                 { $$ = NullExp(); }
                    ;
Expression_rec      :   Expression
/* root is CommaOp */     {  $$ = MakeTree( CommaOp, $1, NullExp() ); }
                    |   Expression COMMAnum Expression_rec
                          {  $$ = MakeTree( CommaOp, $1, $3 ); }
                    ;
ReturnStatement     :   RETURNnum
/* root is ReturnOp */    {  $$ = MakeTree(ReturnOp, NullExp(), NullExp() ); }
                    |   RETURNnum Expression
/* root is ReturnOp */    {  $$ = MakeTree(ReturnOp, $2, NullExp()); }
                    ;
IfStatement         :   IFnum Expression StatementList  /* A */
/* root is IfElseOp */    { $$ = MakeTree(IfElseOp, NullExp(), MakeTree(CommaOp, $2, $3)); }
                    |   If_rec StatementList            /* If_rec C */
/* root is IfElseOp */    { $$ = MakeTree(IfElseOp, $1, $2); }
                    |   If_rec IFnum Expression StatementList /* If_rec A */ 
/* root is IfElseOp */    { 
                            tree temp;
                            temp = MakeTree(IfElseOp, NullExp(), MakeTree(CommaOp, $3, $4)); 
                            $$ = MkLeftC( $1, temp);
                          }
                    ;
If_rec              :   IFnum Expression StatementList ELSEnum /* AB */
/* root is IfElseOp */    { $$ = MakeTree(IfElseOp, NullExp(), MakeTree(CommaOp, $2, $3)); }
                    |   If_rec IFnum Expression StatementList ELSEnum /* If_rec AB */
/* root is IfElseOp */    {  
                            tree temp;
                            temp = MakeTree(IfElseOp, NullExp(), MakeTree(CommaOp, $3, $4));
                            $$ = MkLeftC( $1, temp);                          
                          }
                    ;
WhileStatement      :   WHILEnum Expression StatementList
/* root is LoopOp */      {  $$ = MakeTree(LoopOp, $2, $3); }
                    ;
Expression          :   SimpleExpression 
                          {$$ = $1;}
                    |   SimpleExpression Comp_op SimpleExpression
                          {
                            MkLeftC($1, $2);
                            $$ = MkRightC($3, $2);
                          }
                    ;
Comp_op             :   LTnum
                          { $$ = MakeTree(LTOp, NullExp(), NullExp()); }
                    |   LEnum
                          { $$ = MakeTree(LEOp, NullExp(), NullExp()); }
                    |   EQnum
                          { $$ = MakeTree(EQOp, NullExp(), NullExp()); }
                    |   NEnum
                          { $$ = MakeTree(NEOp, NullExp(), NullExp()); }
                    |   GEnum
                          { $$ = MakeTree(GEOp, NullExp(), NullExp()); }
                    |   GTnum
                          { $$ = MakeTree(GTOp, NullExp(), NullExp()); }
                    ;
SimpleExpression    :   Term
                          { $$ = $1; }
                    |   PLUSnum Term
                          { $$ = $2; }
                    |   MINUSnum Term
                          { $$ = MakeTree(UnaryNegOp, $2, NullExp()); }
                    |   SimpleExpression PLUSnum Term
                          { $$ = MakeTree(AddOp, $1, $3); }
                    |   SimpleExpression MINUSnum Term
                          { $$ = MakeTree(SubOp, $1, $3); }
                    |   SimpleExpression ORnum Term
                          { $$ = MakeTree(OrOp, $1, $3); }
                    ;
Term                :   Factor
                          { $$ = $1; }
                    |   Term TIMESnum Factor
                          { $$ = MakeTree(MultOp, $1, $3); }
                    |   Term DIVIDEnum Factor
                          { $$ = MakeTree(DivOp, $1, $3); }
                    |   Term ANDnum Factor   
                          { $$ = MakeTree(AndOp, $1, $3); }
                    ;
Factor              :   UnsignedConstant
                          { $$ = $1; }
                    |   Variable
                          { $$ = $1; }
                    |   MethodCallStatement
                          { $$ = $1; }
                    |   LPARENnum Expression RPARENnum
                          { $$ = $2; }
                    |   NOTnum Factor
                          { $$ = MakeTree(NotOp, $2, NullExp()); }
                    ;
UnsignedConstant    :   ICONSTnum
                          { $$ = MakeLeaf(NUMNode, $1); }
                    |   SCONSTnum
                          { $$ = MakeLeaf(STRINGNode, $1); }
                    ;
Variable            :   IDnum  Variable_tail
/* root is VarOp */       { $$ = MakeTree( VarOp, MakeLeaf(IDNode, $1), $2); }
                    ;
Variable_tail       : 
/* root is DummyOp */     {  $$ = NullExp(); }     
                    |   Variable_rec
/* root is SelectOp */    {  $$ = $1; }
                    ;
Variable_rec        :   Variable_1
/* root is SelectOp */    { $$ = MakeTree( SelectOp, $1, NullExp() ); }
                    |   Variable_1 Variable_rec 
                          { $$ = MakeTree( SelectOp, $1, $2 ); }
                    ;
Variable_1          :   LBRACnum Expression_rec2 RBRACnum
/* root is IndexOp */     {  $$ = $2; }
                    |   DOTnum IDnum
/* root is FieldOp */     {  $$ = MakeTree(FieldOp, MakeLeaf(IDNode, $2), NullExp() ); }
                    ;
Expression_rec2     :   Expression
/* root is IndexOp */     { $$ = MakeTree(IndexOp, $1, NullExp()); }
                    |   Expression COMMAnum Expression_rec2  
/* root is IndexOp */     { $$ = MakeTree(IndexOp, $1, $3); }
                    ;

%%

int yycolumn, yyline;

FILE *treelst;

/*main()
{
  treelst = stdout;
  yyparse();
} */

yyerror(char *str)
{
  printf("yyerror: %s at line %d\n", str, yyline);
}

#include "lex.yy.c"



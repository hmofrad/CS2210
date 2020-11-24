#include "proj2.h"
#include "proj3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define CODEFILE "code.s"
#define REGST    0xFF0007FF  /* used for finding abailbe s and t regs */

extern void* yyin;

tree SyntaxTree; /* root of syntax tree */
FILE *treelst;   /* tree print file */
FILE *asmf;      /* code file */
int  labelcnt;   /* global label counter */
char database[5];/* the label of base of .data area */
int  coff;       /* offset within a class are relative */
int  csize;      /* class size of one class */
int  goff;       /* global offset */

extern int st[ST_SIZE];
extern attr_type attrarray[ATTR_SIZE];
extern int st_top;
extern void semantics();
extern int  arraysize( tree );
extern expression(tree);

void GenCode( tree );
void Genclassbodyop( tree, int );
void Genmethodop(tree);
int  classdeclop(tree);
void setargoff( tree );
void methodbody( tree, int* );
void methoddeclop( tree, int* );
void pusharray( tree, int );
void allarry( tree,int);
int  arraysize(tree);
int  constfold(tree);
void cinit(tree, int);

int  lregister;                    /* list of registers that have been used, marked by 1 */



int main(int argc, char *argv[])
{
    char* file_name = argv[1];
    FILE *fid = fopen(file_name, "rb");
    if(fid==NULL) { printf("File %s does not exist!\n", file_name); exit(1); }
    yyin = fid;

   SyntaxTree = NULL;
   yyparse(); /* make syntax tree */
   if(SyntaxTree == NULL) { printf( "Syntax Tree not created, exiting...\n" ); exit(1); }
   treelst = stdout;


   STInit();
   MkST(SyntaxTree);

  lregister = 0xFC000003;        
  labelcnt = 0;                  
  strcpy( database, "base" );    
  if( (asmf = fopen( CODEFILE, "w" )) == NULL )
    printf("can not open file %s to append\n", CODEFILE), exit(1);

  fprintf( asmf, ".data\n" );        
  fprintf( asmf, "Enter:\t.asciiz \"  \n\"\n" );
  fprintf( asmf, "%s:\n", database); 

  fprintf( asmf, ".text\n" );        
  fprintf(asmf, ".globl main\n" );   
  goff = 0;              
  GenCode( SyntaxTree ); 

  fclose(asmf);          

  //STPrint();
  //printtree(SyntaxTree,0); 

   return(0); 
}
/*
void main()
{
  semantics();                    // create symbol table first
  //STPrint();
  //printtree(SyntaxTree, 0);
  

  if( SyntaxTree == NULL )
    {
      printf("Syntax tree not created, exiting...\n");
      exit(1);
    }

  lregister = 0xFC000003;                       // $31-$26, $1-$0 are used 
  labelcnt = 0;                                 // Initialize label counter 
  strcpy( database, "base" );                   // Initialize global data base label 
  if( (asmf = fopen( CODEFILE, "w" )) == NULL ) // Open assembly code file 
    printf("can not open file %s to append\n", CODEFILE), exit(1);

  fprintf( asmf, ".data\n" );        // code -- .data 
  fprintf( asmf, "Enter:\t.asciiz \"  \n\"\n" );
  fprintf( asmf, "%s:\n", database); // code -- base: so that others can access globals by adding offset 

  fprintf( asmf, ".text\n" );        // code -- .text 
  fprintf(asmf, ".globl main\n" );                 // code--.globl main 
  goff = 0;                          // initialize global offset 
  GenCode( SyntaxTree );             // Generate code into code file 

  fclose(asmf);                      // close code file 

  STPrint();
  printtree(SyntaxTree,0); 
}
*/

/* Generate code for the whole program */
void GenCode( tree treenode )
{
  tree classid, varop;
  int  nSymInd;

  if( ! IsNull(treenode) )
    {
      switch( NodeOp(treenode) ){
      case ClassDefOp:
	coff = 0;                                      /* reset class offset to 0 */
	csize = 0;                                     /* reset class size to 0 */
	/*before generate code, set beginning offset in .data of the class */
	nSymInd = IntVal( (tree)((uintptr_t)RightChild(treenode)) );/* get the class name symbol table entry ind */
	SetAttr(nSymInd, OFFSET_ATTR, goff);
	Genclassbodyop( (tree)((uintptr_t)LeftChild(treenode)), nSymInd );/* generate code for class body */
	if( !IsAttr(nSymInd, SIZE_ATTR) )              /* is SIZE attr has not been added(class doesn't have methods only decls)*/
	  SetAttr(nSymInd, SIZE_ATTR, csize);          /* set SIZE attr now */
	goff += csize;                                 /* declarations are done, classsize is know now, add to global offset */
	break;
      default:
	GenCode( (tree)((uintptr_t)LeftChild(treenode)) );
	GenCode( (tree)((uintptr_t)RightChild(treenode)) );
	break;
      }
    }
}


/* Generate code for Classes */
void Genclassbodyop( tree treenode , int ind )
{
  tree def;

  if ( IsNull(treenode) )                      /* NULL */
    return;
  
  Genclassbodyop( (tree)((uintptr_t)LeftChild(treenode)), ind ); /* recursively call left child to get the bottom of the tree */

  def = (tree)((uintptr_t)RightChild(treenode));            /* def */
  switch( NodeOp(def) ){
  case DeclOp:                                 /* class decls: allocate space in .data */
    fprintf(asmf,".data\n");                   /* code-- .data */
    csize = classdeclop(def);                  /* get current decl offset, self-add global off, allocate space in .data */
    break;
  case MethodOp:
    SetAttr(ind, SIZE_ATTR, csize);            /* before gen code for method, set class SIZE attr */
    Genmethodop( def );                        /* generate code for methods */
    break;
  default:
    printf("DEBUG--in class body: not method nor decl\n");
    break;
  }
}

/* allocate space for declarations in class */
int classdeclop( tree treenode )
{
  tree declop, commaop, In, T, Vn;          /* nodes in declop subtree */
  int  attr, nSymInd;
  tree TIdnode, classdef, body;

  declop = treenode;                        /* DeclOp */
  commaop= (tree)((uintptr_t)RightChild(declop));        /* CommaOp */
  In     = (tree)((uintptr_t)LeftChild(commaop));        /* In */
  T = (tree)((uintptr_t)LeftChild(RightChild(commaop))); /* T */
  Vn=(tree)((uintptr_t)RightChild(RightChild(commaop))); /* Vn */
  while( !IsNull(declop) )                  /* do from I1 to In in one declaration */
    {
      nSymInd = IntVal(In);
      attr = GetAttr(nSymInd, KIND_ATTR);   /* get the In KIND attr */

      if ( attr == VAR )                    /* In is an int variable or a class obj */
	{
	  switch( NodeKind((tree)((uintptr_t)LeftChild(T))) ){      /* check var type */
	  case INTEGERTNode:                           /* integer var */
	    SetAttr(nSymInd, OFFSET_ATTR, coff);       /* set OFFSET attr */                    
	    coff += 4;                                 /* class offset increment by 4 bytes */
	    fprintf(asmf,"\t.word\t%d\t\t\t#offset %d\n",constfold(Vn), goff+coff-4);  /* code-- .word val $$ val is 0 if there is no init value */
	    break;
	  case STNode:                                 /* obj */
	    SetAttr(nSymInd, OFFSET_ATTR, coff);    /* set OFFSET attr for the obj */
	    TIdnode = (tree)((uintptr_t)LeftChild(T));                   /* class STNode */
	    /* need to initialize all members in class */
	    classdef = (tree)((uintptr_t)GetAttr(IntVal(TIdnode),TYPE_ATTR));/* get ClassDefOp */
	    body = (tree)((uintptr_t)LeftChild(classdef));               /* BodyOp */
	    cinit(body, 0);                                 /* initialize class in data */
	    coff += GetAttr(IntVal(TIdnode),SIZE_ATTR);/* offset increment by class size */

	    break;
	  default:
	    printf("DEBUG--class decls: not integer nor class\n");
	    break;
	  }
	}
      else if( attr == ARR )                         /* In is an array */
	{
	  switch( NodeOp(Vn) ){                      /* ArrayTypeOp or VarOp */
	  case ArrayTypeOp:                          /* integer array */
	    SetAttr( nSymInd, OFFSET_ATTR, coff );   /* set OFFSET attr for array */
	    coff += arraysize(Vn)*4;                 /* class offset increment by array total byte size */
	    allarry( (tree)((uintptr_t)LeftChild(Vn)),nSymInd);   /* allocate space for array in .data, set more attr for In */
	    break;
	  case VarOp:                                /* obj array */
	    break;
	  default:
	    printf("DEBUG--class decls: array elements not int nor class\n");
	    break;
	  }
	}
      declop = (tree)((uintptr_t)LeftChild(declop));
      commaop= (tree)((uintptr_t)RightChild(declop));
      In     = (tree)((uintptr_t)LeftChild(commaop));
      T      = (tree)((uintptr_t)LeftChild(RightChild(commaop)));
      Vn=(tree)((uintptr_t)RightChild(RightChild(commaop)));
    }/* end of while */

  return coff; /* return the current DeclOp top offset */
}

/* Generate code for Methods */
void Genmethodop( tree treenode )
{
  int  i, stoffset, nSymInd, mainoff, argnum;
  char etrlbl[10];                                     /* entry label for the method */ 
  tree headop, specop, bodyop;

  headop = (tree)((uintptr_t)LeftChild(treenode));                  /* HeadOp */
  nSymInd = IntVal((tree)((uintptr_t)LeftChild(headop)));
  strcpy( etrlbl, (char*)getname(GetAttr(nSymInd, NAME_ATTR)) );  /* get the method name as label */

  fprintf( asmf, ".text\n" );                                            /* code--.text */

  if ( !strcmp( "main", etrlbl  ) )                    /* if this is main method */
    {
      fprintf( asmf, "%s:\t\t\t\t\t#method entry\n", etrlbl );    /* code--f_name:   #method entry */
      fprintf( asmf, "\tla\t$28\t%s\t\t#store global area address into $gp\n",database);
                                                       /* code -- la $gp base #.... */
      fprintf(asmf,"\tmove\t$t1\t$28\t\t#init base\n"); /* code -- move $t1 $28 */
      do{                                              /* look into synbol table for enclosing class */ 
	nSymInd--;
      }while( nSymInd>0&& GetAttr(nSymInd, KIND_ATTR) != CLASS );
      mainoff = GetAttr(nSymInd,OFFSET_ATTR);          /* get enclosing class offset */
      fprintf(asmf,"\tadd\t$t1\t$t1\t%d\t#init main access link in $t1; .data %d\n",mainoff,mainoff);
                                                       /* code -- add $t1 $t1 mainoff */
      fprintf(asmf,"\tli\t$t2\t0\t\t#init $t2 as 0-global access\n"); /* code--li $t2 0 .. t2 is global flag */
      fprintf(asmf,"\tmove\t$fp\t$sp\t\t#init fp pointer\n"); /* code -- move $fp $sp */
    }
  else
    {
      fprintf( asmf, "%s_%d:\t\t\t\t\t#method entry\n", etrlbl,nSymInd );      /* code--f_name:   #method entry */
      argnum = GetAttr(nSymInd, SIZE_ATTR);                                    /* get number of args */
      fprintf(asmf,"\tlw\t$t1\t%d($fp)\t\t#init base\n", (20+argnum)*4);       /* 20 include, fp, r8--r25 */
      fprintf(asmf,"\tlw\t$t2\t%d($fp)\t\t#init base\n", (21+argnum)*4);       /* 21 include base, fp, r8--r25 */
    }
  fprintf( asmf, "\tsw\t$ra\t0($sp)\t\t#save return address on stack\n" ); /* code--sw $ra 0($sp) #save return address on stack */
  fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#increase st\n" );                 /* code--addi $sp $sp -4 #increase st */

  specop = (tree)((uintptr_t)RightChild(headop));                  /* SpecOp */
  setargoff(specop );                                 /* set argument offset */
  bodyop = (tree)((uintptr_t)RightChild(treenode));                /* BodyOp */
  stoffset = 4;                                       /* first 4B from fp include ar */
  methodbody(bodyop, &stoffset);                      /* gen code for method decls and stmts */

  fprintf(asmf,"\tlw\t$ra\t0($fp)\t\t#get back control line\n");/* code -- lw $ra 0($fp) */
  fprintf(asmf,"\tmove\t$sp\t$fp\t\t#pop stack to fp\n");       /* code -- move $sp $fp */
  fprintf(asmf,"\tjr\t$ra\t\t\t#routine call return\n");        /* code -- jr $ra */
}

void setargoff( tree specop )
{
  int nSymInd, off;
  tree argop, commaop;

  argop = (tree)((uintptr_t)LeftChild(specop));   /* VArgTypeOp or RArgTypeOp */
  commaop = (tree)((uintptr_t)LeftChild(argop));  /* CommaOp */

  if( IsNull(argop) )                /* no arguments in this method */
    return;
  
  off = 4;                           /* the first arg is right above fp */
  while( !IsNull( argop ) )
    {
      nSymInd = IntVal((tree)((uintptr_t)LeftChild(commaop)));
      SetAttr( nSymInd, OFFSET_ATTR, off );  /* add offset attribute to IDNode */
      off += 4;                              /* value is of integer type */
      argop = (tree)((uintptr_t)RightChild(argop));
      commaop = (tree)((uintptr_t)LeftChild(argop));
    }
}

void methodbody( tree treenode, int *off )    /* off is preserved across recursion */
{
  tree bodyop, bodychild;

  if ( IsNull(treenode) )                     /*  NULL node */
    return;
  methodbody( (tree)((uintptr_t)LeftChild(treenode)), off );    /* left recursion first, to get to the bottom of the subtree */

  bodyop = treenode;                          /* BodyOp */
  bodychild = (tree)((uintptr_t)RightChild(bodyop));       /* StmtOp or DeclOp */

  switch( NodeOp(bodychild) ){
  case DeclOp:
    methoddeclop(bodychild, off);             /* push declared values on stack */
    break;
  case StmtOp:
    stmtop( bodychild );                      /* generate code for statements */
    break;
  }

}

/* push declarations in method on stack */
void methoddeclop( tree treenode, int *stackoffset )
{
  tree declop, commaop, In, T, Vn, TIdnode; /* nodes in declop subtree */
  tree classdef, body;
  int  attr, nSymInd, init, reg, i;

  declop = treenode;                        /* DeclOp */
  commaop= (tree)((uintptr_t)RightChild(declop));        /* CommaOp */
  In     = (tree)((uintptr_t)LeftChild(commaop));        /* In */
  T = (tree)((uintptr_t)LeftChild(RightChild(commaop))); /* T */
  Vn=(tree)((uintptr_t)RightChild(RightChild(commaop))); /* Vn */
  init = 0;                                 /* default initial value */
  while( !IsNull(declop) )                  /* do from I1 to In in one declaration */
    {
      nSymInd = IntVal(In);
      attr = GetAttr(nSymInd, KIND_ATTR);   /* get the In KIND attr */

      if ( attr == VAR )                    /* In is an int variable or a class obj */
	{
	  switch( NodeKind((tree)((uintptr_t)LeftChild(T))) ){          /* check var type */
	  case INTEGERTNode:                               /* integer var */
	    SetAttr( nSymInd, OFFSET_ATTR, *stackoffset ); /* set In OFFSET attr */ 
	    (*stackoffset) += 4;
	    if( NodeKind(Vn) == NUMNode )                  /* In has init value */
	      init = IntVal(Vn);
	    /* push the init value on stack */
	    i = nextr();     
	    fprintf(asmf,"\tli\t$%d\t%d\t\t#load immediate %d to t%d\n",i, init,init,i);/*code--li $i init #... */
	    fprintf(asmf,"\tsw\t$%d\t0($sp)\t\t#store t%d to stack top\n",i,i);         /*code--sw $i 0($sp) #...*/
	    fprintf(asmf,"\taddi\t$sp\t$sp\t-4\t#increase st\n" );             /* code--addi $sp $sp -4 #increase st */
	      break;
	  case STNode:                                      /* obj */
	    SetAttr(nSymInd, OFFSET_ATTR, *stackoffset);    /* set OFFSET attr for the obj */
	    TIdnode = (tree)((uintptr_t)LeftChild(T));                   /* class STNode */
	    /* need to initialize all members in class */
	    classdef = (tree)((uintptr_t)GetAttr(IntVal(TIdnode),TYPE_ATTR));/* get ClassDefOp */
	    body = (tree)((uintptr_t)LeftChild(classdef));               /* BodyOp */
	    cinit(body, 1);                                 /* initialize class on stack */
	    (*stackoffset) += GetAttr(IntVal(TIdnode),SIZE_ATTR);/* offset increment by class size */
	    break;
	  default:
	    printf("method decl: not integer nor obj\n");
	    break;
	    }
	}
      else if ( attr = ARR )                                /* In is an array of either integer or obj */
	{
	  switch( NodeOp(Vn) ){                             /* ArrayTypeOp or VarOp */
	  case ArrayTypeOp:                                 /* integer array */
	    SetAttr( nSymInd, OFFSET_ATTR, *stackoffset );  /* set In OFFSET attr */
	    (*stackoffset) += arraysize(Vn) * 4;            /* increase offset */
	    /* push (base-((low1*n2)+low2))*w on stack */ 
	    pusharray((tree)((uintptr_t)LeftChild(Vn)), nSymInd);                 /* push the array on the stack */
	    break;
	  case VarOp:                                       /* obj array */
	    break; 
	  default:
	    printf("DEBUG--method decl array: elements type not integer nor class\n");/* otherwise impossible */
	    break;
	  }
	}
      declop = (tree)((uintptr_t)LeftChild(declop));
      commaop= (tree)((uintptr_t)RightChild(declop));
      In     = (tree)((uintptr_t)LeftChild(commaop));
      T      = (tree)((uintptr_t)LeftChild(RightChild(commaop)));
      Vn=(tree)((uintptr_t)RightChild(RightChild(commaop)));
      init = 0;
    }/* end of while */
}


/* called by methoddeclop to push an array on stack taking care of initialization */
void pusharray( tree treenode, int ind )
{
  /* treenode is BoundOp or CommaOp */

  tree lchild, rlchild;
  int  val, i, d, d2, reg;

  if ( IsNull(treenode ) )               /* NULL */
    return;
  
  if( NodeOp(treenode) == BoundOp )      /* multi dimensional array */
    {
      d = GetAttr(ind, DIM1_ATTR);
      d2 = GetAttr(ind, DIM2_ATTR);
      if (d2!=0)
	d *= d2;
  
      for(i=0; i<d; i++)               /* push val 0's on stack */
	{
	  fprintf(asmf,"\tsw\t$0\t0($sp)\t\t#store $0 to stack top\n");   /*code--sw $0 0($sp) #...*/
	  fprintf(asmf,"\taddi\t$sp\t$sp\t-4\t#increase st\n" );          /*code--addi $sp $sp -4 #increase st */
	}
    }
  else if( NodeOp(treenode) == CommaOp )                                  /* initialize 1 dimen array */
    {
      pusharray((tree)((uintptr_t)LeftChild(treenode)),ind);
      rlchild=(tree)((uintptr_t)RightChild(treenode));                                 /* initdef */
      val=constfold(rlchild);                                             /* calculate value */
      i = nextr();                                                        /* find next available register */
      fprintf(asmf,"\tli\t$%d\t%d\t\t#load d1 %d to $%d\n",i,val,val,i);  /* code -- li $i d1 */
      fprintf(asmf,"\tsw\t$%d\t0($sp)\t\t#store $%d to stack top\n",i,i); /*code--sw $i 0($sp) #...*/
      fprintf(asmf, "\taddi\t$sp\t$sp\t-4\t#increase st\n" );             /* code--addi $sp $sp -4 #increase st */
    }
}

/* called by classdeclop() to allocate space for array defined in class */
void allarry( tree treenode, int ind )
{
  /* treenode is BoundOp or CommaOp */

  tree lchild, rlchild;
  int  val, i, d, d2, reg;

  if ( IsNull(treenode ) )               /* NULL */
    return;

  if( NodeOp(treenode) == BoundOp )      /* multi dimensional array */
    {
      d = GetAttr(ind, DIM1_ATTR);
      d2 = GetAttr(ind, DIM2_ATTR);
      if (d2!=0)
	d *= d2;
  
      for(i=0; i<d; i++)                     /* push val 0's on stack */
	fprintf(asmf,"\t.word\t0\n");        /* code--.word 0 */
    }
  else if( NodeOp(treenode) == CommaOp )     /* initialize 1 dimen array */
    {
      allarry( (tree)((uintptr_t)LeftChild(treenode)),ind);   
      rlchild=(tree)((uintptr_t)RightChild(treenode));    /* initdef */
      val=constfold(rlchild);                /* calculate value */
      fprintf(asmf,"\t.word\t%d\n", val);    /* code -- .word val */
    }
}

int arraysize(tree treenode)
{
  int size;
  tree lchild, rlchild;

  if ( NodeOp(treenode) != ArrayTypeOp )    /* only work for ArrayTypeOp subtree */
    {
      printf("wrong routine call\n");
      return (-1);
    }

  size = 0;
  lchild = (tree)((uintptr_t)LeftChild(treenode));       /* CommaOp or BoundOp */
  rlchild = (tree)((uintptr_t)RightChild(lchild));       /* initdef or En */
  while( !IsNull(lchild) )                  /* loop for left subtree */
    {
      if ( NodeKind(rlchild) != NUMNode )   /* assuming each dimension is simple number */
	{                                   /* constant expression is left open */
	  printf("array size not known at compile time. exiting...\n");
	  exit(1);
	}
      if ( NodeOp(lchild) == CommaOp )      /* CommaOp */
	size ++;
      else                                  /* BoundOp */
	{
	  if ( size == 0 )
	    size = IntVal( rlchild );
	  else
	    size *= IntVal( rlchild );
	}
      lchild = (tree)((uintptr_t)LeftChild(lchild));
      rlchild = (tree)((uintptr_t)RightChild(lchild));
    }
  return ( size );                        /* return # of elements in the array */
}

/* initialize class on stack, resembles classdefop() */
void cinit(tree treenode, int flag)  /* treenode is an BodyOp */
     /* flag   action
      *  1     push on stack
      *  0     init in .data
      */
{
  tree def, Comma, In, T, V, classdef, body;
  int  nSymInd, attr, init, r;

  if( IsNull(treenode) )
    return;

  cinit((tree)((uintptr_t)LeftChild(treenode)), flag);  /* recursively go down to the 1st declop */
  def=(tree)((uintptr_t)RightChild(treenode));          /* 1st def */
  if( NodeOp(def)==MethodOp )              /* do not do anything for a methodop */
    return;

  /* def is DeclOp */
  Comma = (tree)((uintptr_t)RightChild(def));          /* CommaOp */
  In    = (tree)((uintptr_t)LeftChild(Comma));         /* In */
  T=(tree)((uintptr_t)LeftChild(RightChild(Comma)));   /* T */
  V=(tree)((uintptr_t)RightChild(RightChild(Comma)));  /* Vn */
  while( !IsNull(def) )
    {
      nSymInd = IntVal(In);
      attr = GetAttr(nSymInd, KIND_ATTR);   /* get the In KIND attr */

      if ( attr == VAR )                    /* In is an int variable or a class obj */
	{
	  switch( NodeKind((tree)((uintptr_t)LeftChild(T))) ){      /* check var type */
	  case INTEGERTNode:                           /* integer var */
	    init = constfold(V);                       /* get the initial value */
	    if( flag == 1 )                            /* push class on stack */
	      {
		r = nextr();
		fprintf(asmf,"\tli\t$%d\t%d\t\t#initialize %d to $%d\n" ,r,init,init,r);
		                                       /* code -- li $r init */
		fprintf(asmf,"\tsw\t$%d\t0($sp)\t\t#store $%d to stack top\n" ,r,r);
	                                               /* code -- sw $r 0($sp) */
		fprintf(asmf, "\taddi\t$sp\t$sp\t-4\t#increase st\n" );  
                                                       /* code--addi $sp $sp -4 #increase st */
	      }
	    else                                       /* init class on .data */
	      fprintf(asmf,"\t.word\t%d\n", init);     /* code -- .word init */
	    break;
	  case STNode:                                 /* obj */
	    classdef=(tree)((uintptr_t)GetAttr(IntVal((tree)((uintptr_t)LeftChild(T))), TYPE_ATTR));
	                                               /* ClassDefOp */
	    body=(tree)((uintptr_t)LeftChild(classdef));            /* BodyOp */
	    cinit(body, flag);                         /* initialize this obj on stack */
	    break;
	  default:
	    printf("DEBUG--class decls: not integer nor class\n");
	    break;
	  }
	}
      else if( attr == ARR )                           /* In is an array */
	{
	  switch( NodeOp(V) ){                         /* ArrayTypeOp or VarOp */
	  case ArrayTypeOp:                            /* integer array */
	    if( flag == 1 )                            /* push class on stack */
	      pusharray( (tree)((uintptr_t)LeftChild(V)), nSymInd );/* initialize array on stack */
	    else                                       /* init in .data */
	      allarry( (tree)((uintptr_t)LeftChild(V)), nSymInd); 
	    break;
	  case VarOp:                                  /* obj array */
	    break;
	  default:
	    printf("DEBUG--class decls: array elements not int nor class\n");
	    break;
	  }
	}

      def=(tree)((uintptr_t)LeftChild(def));               /* go down one layer */
      Comma = (tree)((uintptr_t)RightChild(def));          /* CommaOp */
      In    = (tree)((uintptr_t)LeftChild(Comma));         /* In */
      T=(tree)((uintptr_t)LeftChild(RightChild(Comma)));   /* T */
      V=(tree)((uintptr_t)RightChild(RightChild(Comma)));  /* Vn */
    }
}

int constfold(tree treenode)              /* constant folding */
{
  int rtn;
 
  rtn = -1;

  if( IsNull(treenode) )
      rtn = 0;

  if( NodeKind(treenode)==NUMNode )
      rtn = IntVal(treenode);

  return rtn;

}

int nextr()           /* find a available register */
{
  int i, reg;

  reg = lregister | REGST  ;                     /* find available registers in s and t */
  for(i=0; i<26; i++)                            /* search from $0 -- $25 */
    {
      if( (reg & 0x00000001) == 0 )              /* lowest bit is 0, available */
	break;
      reg = reg >> 1;                            /* shift right 1 bit */
    }
  if( i==26 )
    printf("DEBUG codegen.c--registers used up\n"), exit(1);

  return i;
}


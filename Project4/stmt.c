/* Register allocation:
 * genvarop() return address, value in $8 --t0, method entry is static
 * expression() return value in stack top
 * $9 stores base
 */

#include "proj2.h"
#include "proj3.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CODEFILE "code.s"
#define REGST    0xFE0007FF  /* used for finding abailbe s and t regs */

extern FILE *asmf;      /* code file */
extern int  labelcnt;   /* global label counter */
extern char database[5];/* the label of base of .data area */
extern int  lregister;  /* register list */
extern int  nextr();
extern int goff;        /* global offset need to include string length */
extern int labelcnt;    /* global label count */

void ifelseop( tree, int );
void loopop( tree, int );
void assignop( tree );
void allstrings(tree);
void genroutinecallop( tree );
void returnop( tree );
int  genvarop(tree, int);
void  expression( tree );
void pusharg(tree, int);
void stmtop(tree);

void stmtop( tree treenode )           /* generate code for all statements */
{
  tree rchild, varop;
  int  endif, endwhile;

  if( IsNull( treenode ) )             /* recursive return point */
    return;

  stmtop( (tree)((uintptr_t)LeftChild(treenode)) ); /* recursively call left child to get the 1st stmt */
  
  rchild = (tree)((uintptr_t)RightChild(treenode)); /* Si */
  switch( NodeOp(rchild) ){
  case IfElseOp:                       /* if-then-else */
    endif = labelcnt;                  /* remember endif label */
    labelcnt++;                        /* increase label so that it can't be reused accidentally */
    ifelseop( rchild, endif );
    fprintf(asmf,"L_%d:\t\t\t\t\t#end if\n", endif); /* code -- L_labelcnt: */
    break;
  case LoopOp:                         /* while loop */
    endwhile = labelcnt;
    labelcnt++;
    loopop( rchild, endwhile );
    fprintf(asmf,"L_%d:\t\t\t\t\t#end while\n", endwhile);
    break;
  case AssignOp:                       /* assignments */
    assignop( rchild );
    break;
  case RoutineCallOp:                  /* routine calls */
    varop = (tree)((uintptr_t)LeftChild(rchild));
    if(IntVal((tree)((uintptr_t)LeftChild(varop)))==1)/* system call */
      {
	allstrings((tree)((uintptr_t)RightChild(treenode)));     /* allocate space in .data for string constants */
      }
    genroutinecallop( rchild );
    break;
  case ReturnOp:                       /* return */
    returnop( rchild );
    break;
  }
}

void ifelseop( tree treenode, int lbl )
     /* lbl is end_if label */
{
  tree rchild, b, s;
  int  r, falselbl;

  if( IsNull(treenode) )
    return;

  ifelseop( (tree)((uintptr_t)LeftChild(treenode)), lbl );       /* get to the first if_else */

  rchild = (tree)((uintptr_t)RightChild(treenode));              /* CommaOp or StmtOp */
  if( NodeOp(rchild) == CommaOp )
    {
      b = (tree)((uintptr_t)LeftChild(rchild));                  /* boolean op */
      s = (tree)((uintptr_t)RightChild(rchild));                 /* statement op */
      expression(b);                                /* result is on top of stack */
      r = nextr();                                  /* find r to store b value */
      fprintf(asmf,"\tlw\t$%d\t4($sp)\t\t#store boolean result in register\n" ,r); 
                                                    /* code -- lw $r 4($sp) */
      falselbl = labelcnt;
      fprintf(asmf,"\tbeqz\t$%d\tL_%d\t\t#if false, jump to L_%d\n" ,r,falselbl,falselbl);
                                                    /* code -- beqz $r L_false */
      labelcnt++;
      if( NodeOp(s) == StmtOp )                     /* true branch */
	{
	  stmtop(s);
	  fprintf(asmf,"\tb\tL_%d\t\t\t#true branch end, jump to end_if\n" ,lbl);
	                                            /* code -- b L_endif */
	}
      else
	printf("If else statement not of StmtOp\n");
      fprintf(asmf,"L_%d:\t\t\t\t#false label\n",falselbl); 
                                                    /* code -- L_false: */
    }
  else                                              /* statement */
    {
      if( NodeOp(rchild) == StmtOp )
	{
	  stmtop(rchild);
	  fprintf(asmf,"\tb\tL_%d\t\t\t#false stmt branch end, jump to end_if\n" ,lbl);
	                                            /* code -- b L_endif */
	}
      else
	printf("If else stmt not of StmtOp\n");
    }
}

void loopop( tree treenode, int endlabel )
{
  tree B, S;
  int loopbegin, r;

  B = (tree)((uintptr_t)LeftChild(treenode));                    /* boolean expression */
  S = (tree)((uintptr_t)RightChild(treenode));                   /* statements */
  loopbegin = labelcnt;
  labelcnt++;
  fprintf(asmf,"L_%d:\t\t\t\t#begin of a while loop\n", loopbegin); /* code -- L_loopbegin */
  expression(B);                                    /* evaluate boolean expression, result in top of stack */
  r = nextr();
  fprintf(asmf,"\tlw\t$%d\t4($sp)\t\t#load boolean value into register\n", r);
                                                    /* code -- lw $r 4($sp) */
  fprintf(asmf,"\tbeqz\t$%d\tL_%d\t\t#boolean false, jump to the end of while\n" ,r, endlabel);
                                                    /* code -- beqz $r L_endlabel */
  stmtop(S);                                          /* generate code for statements */
  fprintf(asmf,"\tb\tL_%d\t\t\t#go to the loop beginning\n", loopbegin);
                                                    /* code -- b L_loopbegin */
}

void assignop( tree treenode )
{
  tree E, V;
  int  addr, val;
  
  E = (tree)((uintptr_t)RightChild(treenode));
  V = (tree)((uintptr_t)RightChild(LeftChild(treenode)));
  
  if(NodeOp(V) != VarOp)
    printf("Assignment: V is not of VarOp\n");
  genvarop( V, 0 );                                   /* get address for V, in $t0 */
  expression(E);                                      /* evaluate expression E, on stack top */
  fprintf(asmf,"\tlw\t$v0\t4($sp)\t\t#stack top -> $v0\n");/* code -- lw $v0 4($sp) */
  fprintf(asmf,"\tsw\t$v0\t0($t0)\t\t# (t0):=v0\n");       /* code -- sw $v0 0($t0) */
  fprintf(asmf, "\taddi\t$sp\t$sp\t4\t#pop st\n" );  /* code--addi $sp $sp 4 #pop st */
}

void allstrings(tree treenode)
     /* treenode is CommaOp */
{
  tree comma, A;
  int  len, remainder;

  comma = treenode;
  while( ! IsNull(comma) )
    {
      A = (tree)((uintptr_t)LeftChild(comma));
      if( NodeKind(A) == STRINGNode )   /* string constant */
	{
	  fprintf(asmf,".data\n");                                       /* code -- .data */
	  fprintf(asmf,".align 2\n");
	  fprintf(asmf,"S_%d:\t.asciiz\t\"%s\"\n" ,IntVal(A),getname(IntVal(A)));     /* code -- L_#: stringconstant */
      len = (int) strlen((char*)getname(IntVal(A)));
	  remainder = (len+1) % 4;
	  if( remainder != 0 )                                                  /* string length is not multiple of 4 */
	    goff += len+1+4-remainder;
	  fprintf(asmf,".text\n");
	}
      comma = (tree)((uintptr_t)RightChild(comma));
    }
}

void genroutinecallop( tree treenode )
{
    printf("ToDo: Implement genroutinecallop\n");
/*****************************************/
/*  Please generate the method call code */
/*****************************************/
}

void pusharg( tree treenode, int ind )
{
  /* treenode is CommaOp, ind is the symbl tbl index for current arg */
  tree A;
  int  r;

  if(IsNull(treenode))
    return;

  A = (tree)((uintptr_t)LeftChild(treenode));
  /*if( constfold(A) >= 0  )                                          /* constant par */
  /*  {
  /*    r = nextr();
  /*    fprintf(asmf,"\tli\t$%d\t%d\t\t#const par\n" ,r, IntVal(A));  /* code -- li $r IntVal(A) */
  /*    fprintf(asmf,"\tsw\t$%d\t0($sp)\t\t#push const on stack\n",r);/* code -- sw $r 0($sp) */
  /*    fprintf(asmf,"\taddi\t$sp\t$sp\t-4\t#increase st\n" );        /* code -- addi $sp $sp -4  */
  /*    pusharg((tree)RightChild(treenode),ind);                      /* symbol table index does not increase */
  /*  }
  /*else
   */
    pusharg((tree)((uintptr_t)RightChild(treenode)),ind+1);

  if( GetAttr(ind, KIND_ATTR) == VALUE_ARG )       /* current arg is value arg */
    expression( (tree)((uintptr_t)LeftChild(treenode)) );       /* value on stack top */
  else if (GetAttr(ind, KIND_ATTR) == REF_ARG )    /* current arg is reference arg */
    {
      if( NodeOp(A) != VarOp )
	printf("Ref argument should be VarOp\n");
      genvarop(A, 0);                              /* get address of current arg, in t0 */
      fprintf(asmf,"\tsw\t$t0\t0($sp)\t\t#push arg address\n");
                                                   /* code -- sw $t0 0($sp) */
      fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#increase st\n" ); /* code--addi $sp $sp -4 #increase st */
    }
}

void returnop( tree treenode )
{
  expression((tree)((uintptr_t)LeftChild(treenode))); /* result on stack top */
  fprintf(asmf,"\tlw\t$v1\t4($sp)\t\t#copy return value to register v1\n"); /* code -- lw $v1 4($sp) */
}

int genvarop(tree treenode, int val_addr)
{
  /* treenode is VarOp
   * val_addr   return
   *    0      put address ( int offset ) in $t0
   *    1      push value on stack top
   *    2      methodentry (symbol table index);base in $t1
   */
  
  tree idnode, selectop, lselect, lindex, lfield;
  int nSymInd1, nSymInd, off, e1, e2, kind, gl, i, offr, reg, addlabel, aftlabel, length;

  idnode = (tree)((uintptr_t)LeftChild(treenode));               /* IDNode */
  selectop = (tree)((uintptr_t)RightChild(treenode));            /* SelectOp */
  nSymInd1 = IntVal(idnode);                        /* 1st field symbol tbl ind */
  nSymInd  = nSymInd1;                              /* save the 1st, make a copy */

  off = GetAttr(nSymInd, OFFSET_ATTR);         /* get offset; base offset if nSymInd is a class */
  offr = nextr();
  fprintf(asmf,"\tli\t$%d\t%d\t\t#load offset in $%d\n",offr,off,offr);
                                               /* code -- li $offr off #... */
  reg = 0x00000001;                            /* set the offrth bit in register to 1 */
  reg = reg << offr;
  lregister |= reg;                            /* make register $offr unavailable */

  while( ! IsNull(selectop) )                  /* an array exp or a field exp */
    {
      lselect = (tree)((uintptr_t)LeftChild(selectop));     /* IndexOp or FieldOp */
      if(NodeOp(lselect)==IndexOp)             /* IndexOp */
	{
	  lindex = (tree)((uintptr_t)LeftChild(lselect));   /* E1 */
	  expression(lindex);                  /* evaluate E1, result on stack top */
	  fprintf(asmf,"\tlw\t$v0\t4($sp)\t\t#stack top -> $v0\n");/* code -- lw $v0 4($sp) */
	  fprintf(asmf,"\tsll\t$v0\t$v0\t2\t#shift $v0 2 bits left\n");
	                                       /* code -- sll $v0 $v0 2 #..., e1*=4 */
	  fprintf(asmf,"\taddi\t$sp\t$sp\t4\n"); /* code -- addi $sp $sp 4 */
	  if( GetAttr(nSymInd, DIMEN_ATTR)==1) /* 1 dimensional array */
	    {
	      fprintf(asmf,"\tadd\t$%d\t$%d\t$v0\t#offr=base+i*width\n" ,offr,offr);
	                                       /* code -- add $offr $v0 #... */
	      /* off += e1 * 4;                      off = base + i*width */
	      reg = 4;                         /* set $4--$v0 available */
	      lregister |= reg;
	    }
	  else                                 /* 2 dimension array */
	    {
	      lselect=(tree)((uintptr_t)RightChild(lselect));/* go down along IndexOp right branch */
	      if(!IsNull(lselect))              /* a[2,3] */
		{
		  off = GetAttr(nSymInd,DIM2_ATTR);      /* d2 */
		  fprintf(asmf,"\tmul\t$v0\t$v0\t%d\t#v0=v0*d2\n", off);
		                                         /* code -- mul $v0 $v0 off #... */
		  fprintf(asmf,"\tadd\t$%d\t$%d\t$v0\t# offr=base+i*d2*width\n", offr, offr);
		                                         /* code -- add $offr $offr $v0 */
		  /* off += e1*GetAttr(nSymInd,DIM2_ATTR)*4;   off = base + i*d2*width */
		  reg = 4;                               /* set $4--$v0 available */
		  lregister |= reg;

		  if(GetAttr(nSymInd, DIMEN_ATTR)!=2)    /* assert it's 2 dim array */
		    printf("Array should be 2 dim\n");
		  lindex=(tree)((uintptr_t)LeftChild(lselect));
		  expression(lindex);                    /* evaluate E2, result on stack top */
		  fprintf(asmf,"\tlw\t$v0\t4($sp)\t\t#stack top -> $v0\n");/* code -- lw $v0 0($sp) */
		  fprintf(asmf,"\tsll\t$v0\t$v0\t2\t#shift $v0 2 bits left\n");
	                                                 /* code -- sll $v0 $v0 2 #..., e2*=4 */
		  fprintf(asmf,"\tadd\t$%d\t$%d\t$v0\t# offr+=e2*4\n",offr,offr);
		                                         /* code -- add $offr $offr $v0 */
		  reg = 4;                               /* set $4--$v0 available */
		  lregister |= reg;
		  /* off+=e2*4; */
		}
	      selectop=(tree)((uintptr_t)RightChild(selectop));       /* go down,expecting to see a[][] */
	      lselect=(tree)((uintptr_t)LeftChild(selectop));         /* IndexOp or FieldOp */
	      if(NodeOp(lselect)==IndexOp)               /* the 2nd dim */
		{
		  off = GetAttr(nSymInd,DIM2_ATTR);      /* d2 */
		  fprintf(asmf,"\tmul\t$v0\t$v0\t%d\t#v0=v0*d2\n", off);
		                                         /* code -- mul $v0 $v0 off #... */
		  fprintf(asmf,"\tadd\t$%d\t$%d\t$v0\t# offr=base+i*d2*width\n",offr,offr);
		                                         /* code -- add $offr $offr $v0 */
		  /* off += e1*GetAttr(nSymInd,DIM2_ATTR)*4;  off = base + i*d2*width */
		  reg = 4;                               /* set $4--$v0 available */
		  lregister |= reg;
		  expression((tree)((uintptr_t)LeftChild(lselect)));  /* evaluate E2, result on stack top */
		  fprintf(asmf,"\tlw\t$v0\t4($sp)\t\t#stack top -> $v0\n");/* code -- lw $v0 0($sp) */
		  fprintf(asmf,"\tsll\t$v0\t$v0\t2\t#shift $v0 2 bits left\n");
		                                         /* code -- sll $v0 $v0 2 #..., e2*=4 */
		  fprintf(asmf,"\tadd\t$%d\t$%d\t$v0\t# offr+=e2*4\n",offr,offr);
		                                         /* code -- add $offr $offr $v0 */
		  reg = 4;                               /* set $4--$v0 available */
		  lregister |= reg;
		  /* off += e2 * 4;                          off = base + i*d1 + j*width */
		} 
	    }
	}
      else                                               /* FieldOp */
	{
	  lfield=(tree)((uintptr_t)LeftChild(lselect));               /* STNode, or IDNode(length) */
	  if( NodeKind(lfield) == IDNode )               /* length field */
	    {
	      if( strcmp( (char*)getname(IntVal(lfield)), "length" ) == 0
		  && GetAttr(nSymInd1, KIND_ATTR)==ARR )
		{
		  if( GetAttr(nSymInd1, DIMEN_ATTR)==1 )
		    length = GetAttr(nSymInd1, DIM1_ATTR);
		  else if( GetAttr(nSymInd1, DIMEN_ATTR) == 2 )
		    length = GetAttr(nSymInd1, DIM1_ATTR)*GetAttr(nSymInd1,DIM2_ATTR);
		  fprintf(asmf,"\tli\t$v0\t%d\t\t#length of array\n" , length);
		  fprintf(asmf,"\tsw\t$v0\t0($sp)\t\t#put on stack\n");
		  fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#push st\n" );   /* code--addi $sp $sp -4  */
		  reg = 0x00000001;
		  reg = reg << offr;
		  reg = ~reg;
		  lregister &= reg;                                      /* make offrth register available */
		  return;
		}
	    }
	  nSymInd = IntVal(lfield);                      /* Update nSymInd cuz it might be any array in a class */
	  off = GetAttr(nSymInd,OFFSET_ATTR);
	  fprintf(asmf, "\tadd\t$%d\t$%d\t%d\t#offr+field relative addr\n",offr,offr, off);
	                                                 /* code -- add $offr $offr off */
	  /* off+=GetAttr(nSymInd,OFFSET_ATTR);          /* off+field relative addr */
	}
      
      selectop = (tree)((uintptr_t)RightChild(selectop));             /* go down one layer for the next while loop */
    }

  switch( GetAttr(nSymInd1, NEST_ATTR) ){
  case 0:                                                /* 1st field is a class name */
    fprintf(asmf,"\tadd\t$%d\t$%d\t$gp\t\t#class.---, add to t1 base\n" ,offr ,offr);
                                                         /* code -- add $offr $offr $gp */
    break;
  case 1:                                                /* 1st field is defined in a class */
    addlabel = labelcnt;
    labelcnt++;
    aftlabel = labelcnt;
    labelcnt++;
    fprintf(asmf,"\tbeqz\t$t2\tL_%d\t\t#if called from a global obj\n", addlabel);  
                                                         /* t2 is 0, called from global obj */
    fprintf(asmf,"\tsub\t$%d\t$t1\t$%d\t#sub $offr from t1\n" ,offr ,offr);
                                                         /* code -- sub $offr $t1 $offr */
    fprintf(asmf,"\tb\tL_%d\n", aftlabel);
    fprintf(asmf,"L_%d:\n", addlabel);                   /* L_labelcnt: */
    fprintf(asmf,"\tadd\t$%d\t$%d\t$t1\t#add $offr with base\n" ,offr,offr);/* suppose the value of base addr is in $t1 */
                                                         /* code -- add $offr $offr $t1 */
    fprintf(asmf,"L_%d:\n", aftlabel);
    break;
  case 2:                                                /* 1st field is a local */
    kind = GetAttr(nSymInd, KIND_ATTR);                  /* get it's kind, could be var, ref_arg, value_arg etc */
    if( kind == REF_ARG )                                /* reference arg */
      {
	fprintf(asmf,"\tadd\t$%d\t$fp\t$%d\t#reference arg is above fp\n" ,offr, offr);
	                                                 /* code -- add $offr $fp $offr */
	fprintf(asmf,"\tlw\t$%d\t($%d)\t\t#get ref_arg\n" ,offr,offr);
	                                                 /* code -- lw $offr ($offr) */
      }
    else if( kind == VALUE_ARG )                         /* value arg */
      {
	fprintf(asmf,"\tadd\t$%d\t$fp\t$%d\t#reference arg is above fp\n" ,offr, offr);
	                                                 /* code -- add $offr $fp $offr */
      }
    else                                                 /* other kind of locals */
	fprintf(asmf,"\tsub\t$%d\t$fp\t$%d\t#absoult addr by $fp\n" ,offr, offr);
                                                         /* code -- sub $offr $fp $offr */
    break;
  }
  
  switch( val_addr ){ 
  case 0:                                              /* return address */
    fprintf(asmf, "\tmove\t$t0\t$%d\t\t#$offr=>$t0, return address\n",offr);
                                                       /* code -- move $t0 $offr */
    break;
  case 1:                                              /* return value */
    i = nextr();
    fprintf(asmf,"\tlw\t$%d\t($%d)\t\t#load value from ($%d) to t%d\n",i,offr,offr,i);
                                                       /* code--lw $t0 $offr , load value to t0 */
    fprintf(asmf,"\tsw\t$%d\t0($sp)\t\t#push value on stack\n", i);
                                                         /* code -- sw $t0 0($sp) */
    fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#increase st\n" ); /* code--addi $sp $sp -4 #increase st */
    break;
  case 2:                                               /* return method entry */
    {
      fprintf(asmf, "\tmove\t$25\t$%d\t\t#$offr=>$t25, base address\n",offr);
                                                         /* code -- move $t25 $offr */
      if( GetAttr(nSymInd1, NEST_ATTR) == 2 )            /* if is 1, parent access type unknown, inherite */
	fprintf(asmf,"\tli\t$24\t1\t\t#routine call from a local on stack\n"); /* code -- li $24 1 */
      else
	fprintf(asmf,"\tmove\t$24\t$t2\t\t#$offr=>$t24, flag\n");
                                                         /* code -- move $t25 $offr */

      fprintf(asmf,"\tsw\t$24\t0($sp)\t\t#push base $t2\n"); /* code -- sw $t24 0($sp) === flag */ 
      fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#push st\n" );   /* code--addi $sp $sp -4  */
      fprintf(asmf,"\tsw\t$25\t0($sp)\t\t#push base $t1\n"); /* code -- sw $t25 0($sp) ===base */ 
      fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#push st\n" );   /* code--addi $sp $sp -4  */
    }
    reg = 0x00000001;
    reg = reg << offr;
    reg = ~reg;                                              /* make offrth register available */
    lregister &= reg;
    return nSymInd;
  }
  
  reg = 0x00000001;
  reg = reg << offr;
  reg = ~reg;
  lregister &= reg;                                          /* make offrth register available */

}


/* called by pusharray to evaluate constant expression 
 * expected to evaluate expression */
void expression( tree treenode )
{
  int r1, r2, reg;

  if( IsNull(treenode) )                      /* Null */
    {                                         /* push 0 on stack */
      fprintf(asmf,"\tsw\t$0\t0($sp)\t\t#null exp, push 0\n");/*code -- sw $0 0($sp) */
      fprintf(asmf, "\taddi\t$sp\t$sp\t-4\t#push st\n" );   /* code--addi $sp $sp -4  */
      return;
    }

  if( NodeKind(treenode) == NUMNode )         /* treenode is a NUMNode */
    {
      r1 = nextr();
      fprintf(asmf,"\tli\t$%d\t%d\t\t#exp NUMNode $%d<-imme\n" ,r1,IntVal(treenode),r1);
                                              /* code -- li $r1 IntVal(treenode) */
      fprintf(asmf,"\tsw\t$%d\t0($sp)\t\t#push imme on stack\n" ,r1);
                                              /* code -- sw $r1 0($sp) */
      fprintf( asmf, "\taddi\t$sp\t$sp\t-4\t#increase st\n" ); /* code--addi $sp $sp -4 #increase st */
    }

  switch( NodeOp(treenode) ){
  case RoutineCallOp:
    genroutinecallop(treenode);
    fprintf(asmf,"\tsw\t$v1\t0($sp)\n");   /* code -- sw $v1 0($sp) */
    fprintf(asmf, "\taddi\t$sp\t$sp\t-4\t#push st\n" );   /* code--addi $sp $sp -4  */
    break;
  case VarOp:
    genvarop(treenode,1);                /* result is on stack top */
    break;
  case AddOp:                            /* Binary Op */
  case SubOp:
  case MultOp:
  case DivOp:
  case LTOp:
  case LEOp:
  case GTOp:
  case GEOp:
  case EQOp:
  case NEOp:
  case AndOp:
  case OrOp:
    expression( (tree)((uintptr_t)LeftChild(treenode)));  /* result is on stack top */
    expression( (tree)((uintptr_t)RightChild(treenode))); /* result is on stack top */
    r2 = nextr();                             /* find a free register */
    fprintf(asmf,"\tlw\t$%d\t4($sp)\t\t#stack top -> $%d\n",r2 ,r2);/* code -- lw $r1 4($sp) */
    fprintf(asmf, "\taddi\t$sp\t$sp\t4\t#pop st\n" );               /* code--addi $sp $sp 4 # */
    reg = 0x00000001;
    reg = reg<<r2;
    lregister |= reg;                         /* make r1th register unavailable */

    r1 = nextr();                             /* find a free register */
    fprintf(asmf,"\tlw\t$%d\t4($sp)\t\t#stack top -> $%d\n",r1 ,r1);/* code -- lw $r2 4($sp) */
    switch( NodeOp(treenode) ){
    case AddOp:
      fprintf(asmf,"\tadd\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- add $r2 $r1 $r2 */
      break;
    case SubOp:
      fprintf(asmf,"\tsub\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- sub $r2 $r1 $r2 */
      break;
    case MultOp:
      fprintf(asmf,"\tmul\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- mul $r2 $r1 $r2 */
      break;
    case DivOp:
      fprintf(asmf,"\tdiv\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- div $r2 $r1 $r2 */
      break;
    case LTOp:
      fprintf(asmf,"\tslt\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- slt $r2 $r1 $r2 */
      break;
    case LEOp:
      fprintf(asmf,"\tsle\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- sle $r2 $r1 $r2 */
      break;
    case GTOp:
      fprintf(asmf,"\tsgt\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- sgt $r2 $r1 $r2 */
      break;
    case GEOp:
      fprintf(asmf,"\tsge\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- sge $r2 $r1 $r2 */
      break;
    case EQOp:
      fprintf(asmf,"\tseq\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- seq $r2 $r1 $r2 */
      break;
    case NEOp:
      fprintf(asmf,"\tsne\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- sne $r2 $r1 $r2 */
      break;
    case AndOp:
      fprintf(asmf,"\tand\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- and $r2 $r1 $r2 */
      break;
    case OrOp:
      fprintf(asmf,"\tor\t$%d\t$%d\t$%d\t#add r1 to r2\n" ,r2,r1,r2);/* code -- or $r2 $r1 $r2 */
      break;
    }
    fprintf(asmf,"\tsw\t$%d\t4($sp)\t\t#push the sum on stack\n" ,r2);/*code-- sw $r2 4($sp) */
    reg = 0x00000001;
    reg = reg << r2;
    reg = ~reg;
    lregister &= reg;
    /* reg = 0x11111110;
    reg  = reg<<r2;
    lregister &= reg;                         /* make r1th register available */
    break;
  case UnaryNegOp:
  case NotOp:
    expression( (tree)((uintptr_t)LeftChild(treenode)) );  /* result is on stack top */
    r1 = nextr();                             /* find a free register */
    fprintf(asmf,"\tlw\t$%d\t4($sp)\t\t#stack top -> $%d\n",r1 ,r1);/* code -- lw $r1 4($sp) */
    if( NodeOp(treenode)==UnaryNegOp )
      fprintf(asmf,"\tneg\t$%d\t$%d\t\t#unaryneg\n" ,r1,r1);        /* code--neg $r1 $r1 */
    else
      fprintf(asmf,"\tnot\t$%d\t$%d\t\t#not\n" ,r1,r1);             /* code--not $r1 $r1 */
    fprintf(asmf,"\tsw\t$%d\t4($sp)\t\t#push value on stack\n" ,r1); /* code--sw $r1 4($sp) */
    break;
  }
  
}






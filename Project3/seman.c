#include "proj2.h"
#include "proj3.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern void* yyin;

extern int loc_str();
extern void yyparse();
extern int st_top;

/* Root of the syntax tree */
tree  SyntaxTree;
/* printtree output direction */
FILE *treelst;

void MkST(tree);
void typeidop(tree);
void varop(tree, int);

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
   STPrint();
   printtree(SyntaxTree, 0);

   return(0); 
}

void classdefop(tree node)
{
  int nStrInd;     /* String Table index */
  int nSymInd;     /* Symbol Table index */
  tree child;

  /* Increment nesting level
   * Make a new ST entry with class name, on the right child branch
   * Change the right child to STNode with ST entry index as the value
   * Traverse left child */
  child = (tree)((uintptr_t)RightChild(node));
  nStrInd = IntVal(child);
  nSymInd = InsertEntry( nStrInd );      /* push on the stack first */
  SetAttr( nSymInd, KIND_ATTR, CLASS );
  OpenBlock();                           /* then open block */
  free( child );
  SetRightChild(node, MakeLeaf( STNode, nSymInd ) );
  MkST( (tree)((uintptr_t)LeftChild(node)) );
  CloseBlock();

}

void methodop(tree node)
{
  int nStrInd;     /* String Table index */
  int nSymInd;     /* Symbol Table index */
  tree child, child1;

	/* Increment nesting level
	 * Make a new ST entry with method name, on the left child branch
	 * Add a type attribute for returning type
	 * Change the name node from IDNode to STNode with ST entry index
	 * Traverse right child of HeadOp and right branch */

	child = (tree)((uintptr_t)LeftChild(node));               /* method name */
	nStrInd = IntVal(LeftChild(child));
        if( nStrInd == loc_str("main") )
	  {
	    int i;
	    for (i=0; i<=st_top; i++)
	      if( IsAttr(i, NAME_ATTR) )
		if( GetAttr(i, NAME_ATTR)== nStrInd )
		  {
		    error_msg(REDECLARATION, CONTINUE, nStrInd, 0);
		    return;
		  }
	  }
	nSymInd = InsertEntry( nStrInd );
	OpenBlock();
	child1 = (tree)((uintptr_t)RightChild(RightChild(child)));/* returning type not null */
	if( ! IsNull(child1)  )
	  {
	    SetAttr(nSymInd, KIND_ATTR, FUNC);
	    SetAttr(nSymInd, TYPE_ATTR, (uintptr_t)child1 ); /* if child1 null, void type */
	  }
	else
	  SetAttr(nSymInd, KIND_ATTR, PROCE);
	free( (tree)((uintptr_t)LeftChild(child)) );
	SetLeftChild(child, MakeLeaf( STNode, nSymInd ) );
	MkST( (tree)((uintptr_t)RightChild(child)) );
	MkST( (tree)((uintptr_t)RightChild(node)) );
	CloseBlock();
}

void declop(tree node)
{
  int nStrInd;     /* String Table index */
  int nSymInd;     /* Symbol Table index */
  tree child, child1, child2, typenode;

  /* Make new ST entries
   * Add type attributes for the lc of CommaOp
   * For array type ID nodes, add dimension attribute
   * Add InitValAttr if the rc of CommaOp is nonempty
   * Change all IDNodes to STNode
   * Didn't check redeclaration yet */

  child = node;
  while( ! IsNull(child) )
    {
      child1  = (tree)((uintptr_t)RightChild(child));            /* CommaOp */
      nStrInd = IntVal(LeftChild(child1));          /* IDNode  */
      if ( ! (nSymInd = InsertEntry( nStrInd )) )   /* redeclaration */
	return;
      typenode = (tree)((uintptr_t)LeftChild(RightChild(child1)));
      SetAttr( nSymInd, TYPE_ATTR, (uintptr_t)typenode );
      free( (tree)((uintptr_t)LeftChild(child1)) );
      SetLeftChild( child1, MakeLeaf( STNode, nSymInd ) );
      typeidop( typenode );
      if( IsNull(RightChild(typenode)) )
	SetAttr( nSymInd, KIND_ATTR, VAR );
      else if( NodeOp(RightChild(typenode))==IndexOp )
	{
	  int  n;
	  tree temp;

	  n = 0;
	  temp = (tree)((uintptr_t)RightChild(typenode));
	  while( !IsNull(temp) )
	    {
	      n++;
	      temp = (tree)((uintptr_t)RightChild(temp));
	    }
	  SetAttr( nSymInd, DIMEN_ATTR, n );
	  SetAttr( nSymInd, KIND_ATTR, ARR);
	}
      child2  = (tree)((uintptr_t)RightChild(RightChild(child1)));
      if ( NodeOp(child2)==VarOp )
	varop(child2, 1);
      else
	MkST(child2);

      child = (tree)((uintptr_t)LeftChild(child));
    }
}
void specop(tree node)
{
  int nStrInd;     /* String Table index */
  int nSymInd;     /* Symbol Table index */
  tree child, child1;

  /* Make new ST entries
   * Add type attribute on the rc
   * Add IsFormalAttr attribute for a argument, value 0: value argument, 1: reference argument
   * Change the IDNode to STNode */

  child = (tree)((uintptr_t)LeftChild(node));               /* VArgTypeOp or RArgTypeOp */
  while( ! IsNull(child) )
    {
      child1 = (tree)((uintptr_t)LeftChild(child));         /* CommmaOp */
      nStrInd = IntVal(LeftChild(child1));     /* IDNode   */
      nSymInd = InsertEntry( nStrInd );
      SetAttr( nSymInd, TYPE_ATTR, (int)RightChild(child1) );
      if( NodeOp(child) == VArgTypeOp )
	SetAttr( nSymInd, KIND_ATTR, VALUE_ARG );   /* value argument */
      else if( NodeOp(child) == RArgTypeOp )
	SetAttr( nSymInd, KIND_ATTR, REF_ARG );     /* reference argument */
      free( (tree)((uintptr_t)LeftChild(child1)) );                    /* free IDNode    */
      SetLeftChild( child1, MakeLeaf( STNode, nSymInd ) );

      child = (tree)((uintptr_t)RightChild(child));
    }
}

void typeidop( tree node )
{
  /* a type or element type of an array can only be INTEGERTNode or a class */
  tree lchild, rchild;
  int nSymInd;

  lchild = (tree)((uintptr_t)LeftChild(node));
  rchild = node;

  while( !IsNull(rchild) )
    {
      if( !IsNull(lchild) && NodeKind(lchild) != INTEGERTNode )
	{
	  nSymInd = LookUp( IntVal(lchild) );
	  free( lchild );
	  SetLeftChild( rchild, MakeLeaf( STNode, nSymInd ));
	}
      rchild = (tree)((uintptr_t)RightChild(rchild));
      lchild = (tree)((uintptr_t)LeftChild(rchild));
    }
}

void varop(tree node, int declop)/* 1 if called by declop, 2 if called by routineop, 0 otherwise */
{
  /* Id: Id is a local variable;                                  \
   *     Id is a local method of the same nesting level;          / right child is empty
   * Id[E1,E2,...,En]
   *     Id is a local array name
   * Id1.Id2. ... Idn :
   *     Id1 is a class name
   *     Id1 is an object of a class
   *     Id2 is a method declared in class Id1
   *       ...
   * Id[E1,E2,...,En].class_id                 ????? */

  /* int  nSymInd, tempnest;
  int  stind0, nest0;
  int  dimension, d;
  tree lchild, rchild, typeidop, temp, lastfield;
  bool found;*/

  int  nSymInd, tempind;
  int  st_ind0, nest0;
  int  dimension, d;
  int  i;
  tree lchild, rchild, typeidop, temp, selectop, fld_indop;
  bool found;

  st_ind0 = nest0 = -1;
  dimension = d = 0;
  lchild = (tree)((uintptr_t)LeftChild(node));           /* IDNode */
  rchild = (tree)((uintptr_t)RightChild(node));          /* SelectOp */
  if( nSymInd = LookUp(IntVal(lchild)) )
    {
      free(lchild);
      SetLeftChild(node, MakeLeaf( STNode, nSymInd ));
    }
  else    /* didn't find */
      return;

  st_ind0 = nSymInd;
  do{
    switch( GetAttr( st_ind0, KIND_ATTR ) ){
    case VAR:
//****************************************************************************
//***************** Insert code here to check undeclared variable ************
//****************************************************************************
      break;
    case PROCE:
    case FUNC:
      if ( IsNull(rchild) )
	return;
      else
	printf("method %s members can not be accessed\n", getname(IntVal(lchild))), exit(1);
      break;
    case CLASS:
      nest0    = GetAttr( st_ind0, NEST_ATTR );
      selectop = rchild;
      fld_indop  = (tree)((uintptr_t)LeftChild(selectop));
      if ( NodeOp(fld_indop) == FieldOp )
	{
	  i = st_ind0 + 1;
	  do{
	    if( GetAttr(i, NAME_ATTR)==IntVal(LeftChild(fld_indop))
		&& GetAttr(i, NEST_ATTR)==(nest0+1) ) /* found declaration in correct level */
	      {
		free( (tree)((uintptr_t)LeftChild(fld_indop)) );
		SetLeftChild(fld_indop, MakeLeaf( STNode, i ));
		found = true;

		st_ind0 = i;
		nest0++;
		rchild = (tree)((uintptr_t)RightChild(rchild));

		break;
	      }
	    i++;
	  }while(i<=st_top && GetAttr(i,NEST_ATTR) > nest0);
	  if (!found)
	    {
	      error_msg(UNDECLARATION, CONTINUE, IntVal(LeftChild(fld_indop)), 0);
	      return;
	    }
	}
      else if (NodeOp(fld_indop) == IndexOp)
	{
	  if ( declop != 1)
	    {
	      error_msg(TYPE_MIS, CONTINUE, GetAttr(st_ind0, NAME_ATTR), 0);
	      return;
	    }
	  else
	    {
	      int di;

	      di = 1;
	      rchild = (tree)((uintptr_t)RightChild(rchild));
	      while( !IsNull(rchild) )
		{
		  di++;
		  rchild = (tree)((uintptr_t)RightChild(rchild));
		  fld_indop = (tree)((uintptr_t)LeftChild(rchild));
		  if( NodeOp(fld_indop) != IndexOp )
		    {
		      error_msg( FIELD_MIS, CONTINUE, IntVal(LeftChild(fld_indop)), 0);
		      return;
		    }
		}
	    }
	}
      break;
    case ARR:
      if ( IsNull(rchild) )
	{
	  error_msg( INDX_MIS, CONTINUE, GetAttr(st_ind0, NAME_ATTR), 0 );
	  return;
	}
      typeidop = (tree)((uintptr_t)GetAttr( st_ind0, TYPE_ATTR ));
      temp     = (tree)((uintptr_t)LeftChild(typeidop));
      selectop  = rchild;
      fld_indop = (tree)((uintptr_t)LeftChild(rchild));
      dimension = GetAttr( st_ind0, DIMEN_ATTR );
      if ( NodeKind(temp) == INTEGERTNode )
	{
	  d = 0;
	  while( !IsNull(rchild) && NodeOp(fld_indop) != FieldOp )
	    {
	      d++;
	      if ( d > dimension )
		{
		  error_msg( INDX_MIS, CONTINUE, GetAttr(st_ind0,NAME_ATTR), 0);
		  return;
		}
	      if ( NodeKind( LeftChild(fld_indop) ) == EXPRNode )
		MkST( (tree)((uintptr_t)LeftChild(fld_indop)) );
	      selectop = rchild = (tree)((uintptr_t)RightChild(rchild));
	      fld_indop= (tree)((uintptr_t)LeftChild(selectop));
	    }

	  if ( IsNull(rchild) )
	    {
	      if ( d < dimension )
		{
		  error_msg( INDX_MIS, CONTINUE, GetAttr(st_ind0,NAME_ATTR), 0);
		  return;
		}
	    }
	  else
	    {
	      if(  IntVal(LeftChild(fld_indop)) != loc_str("length") )
		{
		  error_msg( TYPE_MIS, CONTINUE, GetAttr(st_ind0,NAME_ATTR), 0);
		  return;
		}
	      else /* length */
		{
		  if ( !IsNull( RightChild(selectop) ) )
		    {
		      error_msg( TYPE_MIS, CONTINUE,  GetAttr(st_ind0,NAME_ATTR), 0);
		      return;
		    }

		  rchild = (tree)((uintptr_t)RightChild(rchild));
		  fld_indop= (tree)((uintptr_t)LeftChild(selectop));
		}
	    }
	}
      else if ( tempind = IntVal(temp) ) /* a class name STNode */
	{
	  d = 0;
	  while( !IsNull(rchild) && NodeOp(fld_indop) != FieldOp )
	    {
	      d++;
	      if ( d > dimension )
		{
		  error_msg( INDX_MIS, CONTINUE,  GetAttr(st_ind0,NAME_ATTR), 0);
		  return;
		}
	      if ( NodeKind( LeftChild(fld_indop) ) == EXPRNode )
		MkST( (tree)((uintptr_t)LeftChild(fld_indop)) );
	      selectop = rchild = (tree)((uintptr_t)RightChild(rchild));
	      fld_indop= (tree)((uintptr_t)LeftChild(selectop));
	    }

	  if ( IsNull(rchild) )
	    {
	      if (d < dimension )
		{
		  error_msg( INDX_MIS, CONTINUE,  GetAttr(st_ind0,NAME_ATTR), 0);
		  return;
		}
	    }
	  else
	    {
	      if( IntVal(LeftChild(fld_indop)) != loc_str("length") )
		{
		  st_ind0 = tempind;
		  nest0   = GetAttr( st_ind0, NEST_ATTR );
		}
	      else
		{
		  if ( !IsNull( RightChild(selectop) ) )
		    {
		      error_msg( TYPE_MIS, CONTINUE, GetAttr(st_ind0,NAME_ATTR), 0);
		      return;
		    }
		  rchild = (tree)((uintptr_t)RightChild(rchild));
		  fld_indop= (tree)((uintptr_t)LeftChild(selectop));
		}
	    }
	}
      break;
    default:
      break;
    }

  }while( ! IsNull(rchild) );
}


void routinecallop(tree node)
{
  /* if right child is NULL, there is no parameters, call varop for the left child
   * if not NULL, call MkST for each parameter expression */
  tree lchild, rchild;

  lchild = (tree)((uintptr_t)LeftChild(node));
  rchild = (tree)((uintptr_t)RightChild(node));

  varop(lchild, 2);
  if( !IsNull(rchild) )
    {
      while( ! IsNull(rchild) )
	{
	  MkST((tree)((uintptr_t)LeftChild(rchild)));
	  rchild = (tree)((uintptr_t)RightChild(rchild));
	}
    }
}


void MkST(tree treenode)
{
  if( ! IsNull(treenode) )
    {
      switch( NodeOp(treenode) ){
      case ClassDefOp:
	classdefop(treenode);
	break;
      case MethodOp:
	methodop(treenode);
	break;
      case DeclOp:
	declop(treenode);
	break;
      case SpecOp:
	specop(treenode);
	break;
      case TypeIdOp:
	typeidop(treenode);
	break;
      case VarOp:
	varop(treenode, 0);
	break;
      case RoutineCallOp:
	routinecallop(treenode);
	break;
      default:
	MkST((tree)((uintptr_t)LeftChild(treenode)));
	MkST( (tree)((uintptr_t)RightChild(treenode)));
	break;
      }
    }
}

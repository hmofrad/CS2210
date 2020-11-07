#define TBL_LEN    101        /* Hash table length */
#define STRTBL_LEN 3001       /* String table length */
#define STR_SPRTR  0          /* String seperator in string table */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yyleng;
extern long yylval;

struct hash_ele{
       int id;        /* token ID: ICONSTnum or IDnum */
       int len;       /* token length */
       int index;     /* string table index */
       struct hash_ele *next;/*over flow pointer*/
};

struct hash_ele *hash_tbl[TBL_LEN];      /* Hash table of length TBL_LEN */
char     strg_tbl[STRTBL_LEN];   /* String table of length STRTBL_LEN */
int      last = 0;               /* end of the string table, empty */

void init_hash_tbl()  /* Initialize hash table */
{
   int i;

   for( i=0; i<TBL_LEN; i++ )
   {
     hash_tbl[i] = NULL;
   }
}

void init_string_tbl()
{
   int i;
   for ( i=0; i<STRTBL_LEN; i++ )
      strg_tbl[i] = 0;
}

void prt_hash_tbl()
{
   int i;
   struct hash_ele *p;

   printf("TokenID\tTokenLen\tIndex\tNext...\n");
   for( i=0; i<TBL_LEN; i++ )
   {
     p = hash_tbl[i];
     while ( p != NULL )
     {
       printf("%d\t%d\t%d\t\t", p->id, p->len, p->index);
       p = p->next;
     }
     printf("\n");
   }
}

void prt_string_tbl()
{
   int i;

   for( i=0; i<last; i++ )
   {
     if( strg_tbl[i] == -1 )
       printf(" ");
     else
      printf("%c", strg_tbl[i]);
   }
   printf("\n");
}

int hashpjw( char *s, int l)      /* compute hash value for string in yytext 
                                      taken from book Pg. 436 */
{
   int  i;
   char *p;
   unsigned h=0, g;

   for(i=0, p=s; i<l ; p=p+1, i++)
   {
     h = ( h<<4 ) + (*p);
     if ( g=h&0xf0000000 )
     {
        h = h^(g>>24);
        h = h^g;
     }
   }
   return h%TBL_LEN;
}

void install_id(char* text, int tokenid ) /* insert an id/string into hash table , set yylval */
{
   int ind, i;
   struct hash_ele *p, *p0;

   yylval = last;  /* starting index in the string table */
   /* search first */
   ind = hashpjw( text, yyleng );
   p = hash_tbl[ind];
   p0 = p;
   while ( p != NULL )
   {
       if ( ! strncmp( (char*)&strg_tbl[p->index], text, yyleng ) ) /* found */
       {
          yylval = p->index;
          return;
       }
       p0 = p;
       p = p->next;
   }

   if( p == NULL )/* did not find */
   {
     /* if the string table is to be overflowed */
     if ( last + yyleng > STRTBL_LEN )
     {
        printf("There is not enough space in string table!!!\n");
        exit(0);
     }

     p = (struct hash_ele *)malloc( sizeof(struct hash_ele) );
     p->id = tokenid;
     p->len = yyleng;
     p->index = last;
     p->next = NULL;
     if ( hash_tbl[ind] == NULL )
        hash_tbl[ind] = p;
     else
     {
        p0->next = p;
     }
       
     /*for(i=0; i<yyleng; i++)
       strg_tbl[last+i] = text[i];
     strg_tbl[last+i] = STR_SPRTR;
     last += i+1; */
     i=0;
     while ( i< yyleng )
     {
       if( text[i] != '\\' )
         strg_tbl[last] = text[i];
       else
       {
         i++;
         switch( text[i] ) {
         case 't':  strg_tbl[last] = '\t'; break;
         case 'n':  strg_tbl[last] = '\n'; break;
         case '\\': strg_tbl[last] = '\\' ; break;
         case '\'': strg_tbl[last] = '\'' ; break;
         default:   strg_tbl[last]='\\'; i--;
         }
       }
       i++;
       last++;
     }
     strg_tbl[last] = STR_SPRTR;
     last++;
   }
}
     
int loc_str(char* string) /* return string table index if string is stored */
                          /* -1 if not stored */
{
  int i=0; /*start of the strg_tbl*/

  while ( i<last )
    {
      if( strcmp((char*)&strg_tbl[i], string) == 0 ) /* found */
	return i;
      else
	i += strlen((char*)&strg_tbl[i]) + 1; /* forward i to next token */
    }

  if ( i==last )
    return -1;
}

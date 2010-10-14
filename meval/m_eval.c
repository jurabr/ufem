/*
   File name: m_eval.c
   Date:      2008/06/13 12:46
   Author:    Jiri Brozovsky

   Copyright (C) 2008 Jiri Brozovsky

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
  
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
  
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
*/

/* Data types:
 * 0 .. unidentified
 * 1 .. number
 * 2 .. number with "E" embedded
 * 3 .. operation symbol
 * 4 .. left bracket
 * 5 .. right bracket
 * 6 .. function
 * 7 .. string
 *
 *
 * Operation types:
 * 0 .. unknown
 * 1 .. +
 * 2 .. -
 * 3 .. *
 * 4 .. /
 * 5 .. ^
 */ 

#include "m_eval.h"

int test_char(char a)
{
	long i ;
	char s[]=
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_" ;

	for (i=0; i<strlen(s); i++) { if (a == s[i]) {return(AF_YES);} }

	return(AF_NO);
}

int test_b_left(char a)
{
	long i ;
	char s[]= "({[" ;

	for (i=0; i<strlen(s); i++) { if (a == s[i]) {return(AF_YES);} }

	return(AF_NO);
}

int test_b_right(char a)
{
	long i ;
	char s[]= ")}]" ;

	for (i=0; i<strlen(s); i++) { if (a == s[i]) {return(AF_YES);} }

	return(AF_NO);
}

int test_symbol(char a)
{
	long i ;
	char s[]= "*/+-^" ;

	for (i=0; i<strlen(s); i++) { if (a == s[i]) {return(AF_YES);} }

	return(AF_NO);
}

int test_number(char a)
{
	long i ;
	char s[]= "0123456789." ;

	for (i=0; i<strlen(s); i++) { if (a == s[i]) {return(AF_YES);} }

	return(AF_NO);
}

void cut_string(char *orig, int from, int to0, char *targ, int max)
{
  int i ;
  int to ;

  if ((to0-from+1) > max) { to = from + max - 1 ; }
  else { to = to0 ; }
  
  for (i=from; i<=to; i++) { targ[i-from] = orig[i] ; }
}

/** does mathematical operation between "a" and "b"  */
double do_op(double a, long symbol, double b)
{

#if 0
printf("OP: %f  _%li_ %f\n",a,symbol,b);
#endif

	switch (symbol)
	{
		case 1: return(a+b); break ;
		case 2: return(a-b); break ;
		case 3: return(a*b); break ;
		case 4: return(a/b); break ;
		case 5: return(pow(a,b)); break ;
		default: return(a); break ;
	}
	return(a);
}

/** Does actual evaluation work
 * @param str string to be parsed 
 * @return result (real number) 
 */
double mev_parse(char *str)
{
	long   num = 0 ;
	long   type[MAX_ME_ARGS] ;
	long   prio[MAX_ME_ARGS] ;
	long   from[MAX_ME_ARGS] ;
	long   to  [MAX_ME_ARGS] ;
	long   symb[MAX_ME_ARGS] ;
	long   stat[MAX_ME_ARGS] ;
	double val [MAX_ME_ARGS] ;
	double subv[MAX_ME_ARGS] ;
  char   cutstr[MAX_ME_STR];
	double lval = 0.0 ;
	long   ops = 0 ;
	long   n_brk_l = 0 ;
	long   n_brk_r = 0 ;
	long   sum = 0 ;
	long   act = 0 ;
	double a, b ;
  long   ai, bi ;
  long   no = 0 ; /* number of operators and TODO: funcions (TODO: add!)*/

	long i, j, k ;

	for (i=0; i<MAX_ME_ARGS; i++)
  {
    from[i] = -1   ;
    to  [i] = -1   ;
		prio[i] =  0   ;
    type[i] =  0   ;
		symb[i] =  0   ;
		val [i] =  0.0 ;
		subv[i] =  0.0 ;
		stat[i] = -1   ;
  }

	ciStrCompr(str);

  /* identify stuff: */
	for (i=0; i<strlen(str); i++)
	{
    if 	(test_number(str[i]) == AF_YES)
		{
      type[num] = 1 ; /* number */
      from[num] = i ;
      to[num] = i ;
      for (j=(i+1); j<strlen(str); j++)
      {
        if (test_number(str[j]) == AF_YES)
        {
          /* OK */
          to[num] = j ;
        }
        else
        {
          if ((str[j] == 'e') || (str[j] == 'E'))
          {
            if ((strlen(str)-1) >= (j+1))
            {
              if (test_number(str[j+1]) == AF_YES)
              {
                type[num] = 2 ;
                to[num] = j+1 ;
                j+=1 ;
                continue ;
              }
              else
              {
                if ((str[j+1] == '-') || (str[j+1] == '+'))
                {
                  if ((strlen(str)-1) >= (j+2))
                  {
                    if (test_number(str[j+2]) == AF_YES)
                    {
                      type[num] = 2 ;
                      to[num] = j+2 ;
                      j+=2 ; /* really? */
                      continue ;
                    }
                    else { break ; }
                  }
                  else { break ; }
                }
                else { break ; }
              }
            }
            else { break ; }
          }
          else { break ; }
        }
      }
      num++ ;
      i = to[num-1] ;
		}

    if 	(test_symbol(str[i]) == AF_YES)
		{
      type[num] = 3 ;
      no++ ;
			switch (str[i])
			{
				case '+': symb[num] = 1 ; break ;
				case '-': symb[num] = 2 ; break ;
				case '*': symb[num] = 3 ; break ;
				case '/': symb[num] = 4 ; break ;
				case '^': symb[num] = 5 ; break ;
				default : type[num] = 0 ; symb[num] = 0 ; break ;
			}
      num++ ;
      continue ;
		}

    if 	(test_b_left(str[i]) == AF_YES)
		{
      type[num] = 4 ;
      num++ ;
      continue ;
		}

    if 	(test_b_right(str[i]) == AF_YES)
		{
      type[num] = 5 ;
      num++ ;
      continue ;
		}


		if 	(test_char(str[i]) == AF_YES)
		{
      type[num] = 7 ; /* string */
      from[num] = i ;
      to[num] = i ;

      for (j=(i+1); j<strlen(str); j++)
      {
		    if 	(test_char(str[j]) == AF_YES)
        {
          to[num] = j ;
        }
        else
        {
          if (test_number(str[j]))
          {
            if ((strlen(str)-1) >= (j+1))
            {
              if (test_char(str[j+1]) == AF_YES)
              {
                to[num] = j ;
                continue ;
              }
              else { break ; }
            }
            else { break ; }
          }
          else { break ; }
        }
      }
      num++ ;
      i = to[num-1] ;
		}
	} /* for i*/

  if (no <= 0) /* no operators or functions? */
  {
    return(atof(str));
  }

  /* fill data field with numbers: */
  for (i=0; i<num; i++)
  {
    for (j=0; j<MAX_ME_STR; j++) { cutstr[j] = '\0' ; }

    switch (type[i])
    {
      case 1:
      case 2:
          cut_string(str, from[i], to[i], cutstr, MAX_ME_STR-1) ;
          val[i] = atof(cutstr) ;
#if 0
printf(" num >%s< [%li] %e\n", cutstr, i, val[i]);
#endif
          break ;
      case 0:
      default: break ;
    }
  }

	/* ****************************************** *
	 * TODO: set order and priority of operators: *
	 * ****************************************** */

	/* TODO brackets: */
	for (i=0; i< num; i++)
	{
		/* TODO */
	}

	/* TODO: functions TODO! */
	for (i=0; i< num; i++)
	{
		/* TODO */
	}


	/* operators: */
	sum = 0 ;
	for (i=0; i< num; i++) { if (type[i] == 3) { sum++ ; } }

	act = 1 ; /* TODO might  be changed later! */
	for (i=0; i< num; i++) 
	{ 
		if (type[i] == 3) 
		{
			if (symb[i] == 5) { prio[i] = act ; act++ ; }
		} 
	}
	
	for (i=0; i< num; i++) 
	{ 
		if (type[i] == 3) 
		{
			if ((symb[i]==4)||(symb[i]==3)) { prio[i] = act ; act++ ; }
		} 
	}
	for (i=0; i< num; i++) 
	{ 
		if (type[i] == 3) 
		{
			if ((symb[i]==2)||(symb[i]==1)) { prio[i] = act ; act++ ; }
		} 
	}
	
#if 0
for (i=0; i<= num; i++) { printf("PR[%li] %li\n",i,prio[i]); }
#endif

	/* bit less (but still extremely) naive solver: */
	for (i=0; i<= act; i++) /* ! act */
	{
		for (j=0 ; j<num; j++)
		{
			if (prio[j] == (i+1))
			{
        ai = -2 ;
        bi = -2 ;

#if 0
printf("OPERATOR no %li -------------------- type %li\n",i,symb[j]);
printf("i=%li j=%li pr=%li\n",i,j,prio[j]);
#endif

				/* do action: */
				if (type[j] != 3) {continue;}
				ops = symb[j] ;
				if (j == 0)
				{
					a = 0.0 ;
					
					if (stat[j+1] != -1) 
          { 
            b = subv[stat[j+1]] ; 
            bi = stat[j+1] ;
          }
					else { b = val[j+1] ; }
					
					subv[i] = do_op(a, ops, b) ;
					stat[j+1] = i ;
				}
				else
				{
					if (j == (num-1))
					{
						if (stat[j-1] != -1) 
            { 
              a = subv[stat[j-1]] ; 
              ai = stat[j-1] ;
            }
						else { a = val[j-1] ; }
					
						b = 0.0 ;

						subv[i] = do_op(a, ops, b) ;
						stat[j-1] = i ;
					}
					else
					{
						if (stat[j-1] != -1) 
						{
							a = subv[stat[j-1]] ; 
              ai = stat[j-1] ;
						}
						else 
						{ 
							a = val[j-1] ; 
						}

						if (stat[j+1] != -1) 
						{ 
							b = subv[stat[j+1]] ; 
              bi = stat[j+1] ;
						}
						else 
						{
							b = val[j+1] ; 
						}

						subv[i] = do_op(a, ops, b) ;
						stat[j-1] = i ;
						stat[j+1] = i ;
					}
					stat[j] = i ;
				}
				lval = subv[i] ;

				for (k=0 ; k<num; k++)
				{
					if ( (stat[k] == ai)||(stat[k] == bi) ) { stat[k] = i ; }
				}

#if 0
for (k=0; k<=num; k++) { printf ("%3li %3li %3li %f %f\n", k, type[k], stat[k], subv[k], val[k]); }
#endif
				break ;
			}
		}
#if 0
		printf("%li/%li %f\n",i, act, subv[i]);
#endif
	}

	
	return(lval);
}


/* end of m_eval.c */

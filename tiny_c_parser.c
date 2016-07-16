#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int token;
char *src, *old_src;
int poolsize;
int line;


//below for virtual machine
int *text, *old_text, *stack;
char *data;

//virtual machine registers
int *pc, *bp, *sp, ax, cycle;

//instructions
enum {LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, 
LI, LC, SI, SC, PUSH, OR, XOR, AND, EQ, NE, LT, GT, LE, 
GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,OPEN, READ, CLOS,
PRTF, MALC, MSET, MCMP, EXIT};

//tokens and classes
enum {Num = 128, Fun, Sys, Glo, Loc, Id, Char, Else, Enum, If,
Int, Return, Sizeof, While, Assign, Cond, Lor, Lan, Or, Xor, And,
Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak};


//identifier struct
struct identifer
{
    int token;
    int hash;
    char * name;
    int class;
    int type;
    int value;
    int Bclass;
    int Btype;
    int Bvalue;
};

int token_val;
int *current_id, *symbols;
int *idmain;                  // the `main` function
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};

void next(void)
{
    char *last_pos;
    int hash;
	
    while(token = *src)
    {
        ++src;//* and ++ have the same operator precedemce, and joint from right to left
        if(token == '\n')
        {
            ++line;
        }
	  else if(token == '#')
	  {
            while(*src != 0 && *src != '\n')
            	{
                 src++;//since we don't support macro, so skip these here
		}
	  }
	  else if((token >= 'a' && token <= 'z')||(token >= 'A'&&token<='Z')||(token == '_'))//identifier
	  {
            last_pos = src -1;//because src has been ++src, so here decrease 1, now it euqals to token value's addr
	      hash = token;

	     while((*src >= 'a' && *src <= 'z')||(*src >= 'A'&&*src <= 'Z')||(*src >= '0'&&*src <= '9')||(*src=='_'))
	     {
                hash = hash*147 + *src;
		   src++;
	     }

	     //look for existing identifer, linear search
	     current_id = symbols;
	     while(current_id[Token])
	     {

               if(current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos))
               {
                   token = current_id[Token];
			return;
	             
	         }
		   current_id = current_id + IdSize;

	     }

	     current_id[Name] = (int)last_pos;
	     current_id[Hash] = hash;
	     token = current_id[Token] = Id;
	     return;
	  }
	  else if(token >= '0' && token <= '9')//num include dec,hex,oct
	  {
            //dec hex oct 
            token_val = token - '0';
	      if(token_val > 0)
	      	{
                 while(*src >= '0' && *src <= '9')
                 {
                     token_val = token_val*10+*src++ -'0';
		    }
		}
		else
		{
                 if(*src == 'x' || *src =='X')
                 {
                     token = *++src;
			  while((token >= '0' && token <= '9')||(token >= 'a'&&token <= 'f')||(token >= 'A'&& token <='F'))
			  {
			       //'A' = 0x41, 'a' = 0x61 token&15(0x0F) we got 'A'(0x01) 'B'(0x02) 'C'(0x03)... +9 = 0x0A , 0x0B, 0x0C...
                          token_val = token_val*16+(token&15)+(token>='A'?9:0);
				token = *++src;
			  }
   
		    }
		    else
		    {
		        while(*src >= '0' && *src <='7')
		        {
                         token_val = token_val*8 + *src++ - '0';
		        }
		    }
		}

		token = Num;
		return;
	  }
	  else if(token == '"' || token == '\'')//string
	  {
             last_pos = data;
		while(*src !=0 && *src != token)
		{
                 token_val = *src++;
		    if(token_val == '\\')//check if it's '\n'
		    {
                     token_val = *src++;
			  if(token_val == 'n')
			  {
                         token_val = '\n';
			  }
		    }

		    if(token == '"')
		    {
                     *data++ = token_val;
		    }
		}

		src++;
		if(token == '"')//if the last char is " means it's a string, otherwise, it's a single char end up with '
		{
                 token_val = (int)last_pos;
		}
		else
		{
                 token = Num;
		}

		return;
	  }
	  else if(token == '/')// comment only support //now
	  {
             if(*src == '/')
             { 
                //skip comments
                 while(*src !=0&& *src !='\n')
                 {
                     ++src;
		    }
		}
		else
		{
                 token = Div;
		    return;
		}
	  }
	  else if(token == '=')
	  {
             if(*src == '=')
             {
                 src ++;
		    token = Eq;
		}
		else
		{
                 token = Assign;
		}
		return;
	  }
	  else if(token == '+')
	  {
             if(*src == '+')
             {
                 src++;
		    token = Inc;
		}
		else
		{
                 token = Add;
		}
		return;
	  }
	  else if(token == '-')
	  {
             if(*src == '-')
             {
                 src++;
		    token = Dec;
		}
		else
		{
                 token = Sub;
		}
		return;
	  }
	  else if(token == '!')
	  {
             if(*src == '=')
             {
                 src++;
		    token = Ne;
		}//else ? Nor? why ...
		return;
	  }
	  else if(token == '<')
	  {
             if(*src == '=')
             {
                 src++;
		    token = Le;
		}
		else if(*src == '<')
		{
                 src++;
		    token = Shl;
		}
		else 
		{
                 token = Lt;
		}

		return;
	  }
	  else if(token == '>')
	  {
             if(*src == '=')
             {
                 src++;
		    token = Ge;
		}
		else if(*src == '>')
		{
                 src++;
		    token = Shr;
		}
	      else
	      	{
                 token = Gt;
		}

		return;
	  }
	  else if(token == '|')
	  {
             if(*src == '|')
             {
                 src++;
		    token = Lor;
		}
		else
		{
                 token = Or;
		}
		return;
	  }
	  else if(token == '&')
	  {
	      if(*src == '&')
	      	{
                 src++;
		    token = Lan;
		}
		else
		{
                 token = And;
		}

		return;

	  }
	  else if(token == '^')
	  {
             token = Xor;
		return;
	  }
	  else if(token == '%')
	  {
             token = Mod;
		return;
	  }
	  else if(token == '*')
	  {
            token = Mul;
	      return;
	  }
	  else if(token == ']')
	  {
             token = Brak;
		return;
	  }
	  else if(token == '[')
	  {
             token = Cond;
		return;
	  }
	  else if(token == '~'||token == ';'||token =='{'||token == '}'||token == '('||token ==')'||token == ']'||token == ','||token == ':')
	  {
            return;
	  }
	  
    }
    
    return;
}

void expression(int level)
{
   // do nothing now
}

void program(void)
{
    next();
    while(token > 0)
    {
        printf("token is: %c\n",token);
	  next();
    }
}

int eval(void)
{
    int op, *tmp;
    while(1)
    {
        op = *pc++;
        switch(op)
        {
            case(IMM):
	      {
                 ax = *pc++;//get reference first then increase
		    break;
	      }
		case(LC):
		{
                 ax = *(char *)ax;//ax stores address, cast it to char type
		    break;
		}
		case(LI):
		{
                 ax = *(int *)ax;
		    break;
		}
		case(SC):
		{
                 ax = *(char *)*sp++ = ax;
		    break;
	       }
		case(SI):
		{
                 *(int *)*sp++ = ax;
		    break;
		}
		case(PUSH):
		{
                 *--sp = ax;
		     break;
		}
		case(JMP):
		{
                 pc = (int *)*pc;// pc stores the next instruction
		    break;
	      }
		case(JZ):
		{
                 pc = ax?pc+1:(int *)*pc;
		    break;
		}
		case(JNZ):
		{
                 pc = ax?(int *)*pc:pc+1;
		    break;
		}
		case(CALL):
		{
                 *--sp = (int)(pc+1);
		    pc = (int *)*pc;
		    break;
		}
		case(ENT):
		{
                 *--sp = (int)bp;
		    bp = sp;
		    sp = sp - *pc++;
		    break;
		}
		case(ADJ):
		{
                 sp = sp + *pc++;
		    break;
		}
		case(LEV):
		{
                 sp = bp;
		    bp = (int *)*sp++;
		    pc = (int *)*sp++;		
		    break;
		}
		case(LEA):
		{
                 ax = (int )(bp+*pc++);
		    break;
	      }
		case(OR):
		{
                ax = *sp++ |ax;
		   break;
		}
		case(XOR):
		{
                ax = *sp++ ^ax;
		   break;
		}
		case(AND):
		{
                 ax = *sp++ &ax;
                 break;
		}
		case(EQ):
		{
                 ax = *sp++ ==ax;
		    break;
		}
		case(NE):
		{
		    ax = *sp++ !=ax;
                 break;
		}
		case(LT):
		{
                 ax = *sp++ < ax;
		    break;
		}
		case(LE):
		{
                 ax = *sp++ <=ax;
		    break;
		}
		case(GT):
		{
                ax = *sp++ >ax;
		   break;
		}
		case(GE):
		{
                ax = *sp++ >=ax;
		   break;
		}
		case(SHL):
		{
                ax = *sp++ <<ax;
		   break;
		}		
		case(SHR):
		{
                ax = *sp++ >>ax;
		   break;
		}		
		case(ADD):
		{
                ax = *sp++ +ax;
		   break;
		}		
		case(SUB):
		{
                ax = *sp++ -ax;
		   break;
		}		
		case(MUL):
		{
                ax = *sp++ *ax;
		   break;
		}
		case(DIV):
		{
                ax = *sp++ /ax;
		   break;
		}
		case(MOD):
		{
                ax = *sp++ %ax;
		   break;
		}
		case(EXIT):
		{
                printf("exit(%d)\n",*sp);
		   return *sp;
		}
		case(OPEN):
		{
                ax = open((char *)sp[1],sp[0]);
		   break;
		}
		case(CLOS):
		{
                ax = close(*sp);
		   break;
		}
		case(READ):
		{
                ax = read(sp[2], (char *)sp[1], *sp);
		   break;
		}
		case(PRTF):
		{
                tmp = sp + pc[1];
		    ax = printf((char *)tmp[-1],tmp[-2],tmp[-3],tmp[-4],tmp[-5],tmp[-6]);
		   break;
		}
		case(MALC):
		{
                ax = (int)malloc(*sp);
		   break;
		}
		case(MSET):
		{
                ax = (int)memset((char *)sp[2], sp[1], *sp);
		   break;
		}
		case(MCMP):
		{
                ax = memcmp((char *)sp[2],(char *)sp[1],*sp);
		   break;
		}
		default:
		{
                printf("unknown instruction:%d",op);
		   return -1; 
		}
		
	  }
    }
    return 0;// do nothing yet
}

int main(int argc, char **argv)
{
    int i, fd;
    argc--;
    argv++;

    poolsize = 256*1024;//why is this?
    line = 1;
    if((fd = open(*argv,0))<0)
    {
        printf("could not open(%s)\n",*argv);
	  return -1;
    }

    if(!(src = old_src = malloc(poolsize)))
    {
        printf("could not malloc(%d) for source area\n", poolsize);
    }

   //read the source file
    if((i = read(fd, src, poolsize-1)) <=0)
    {
        printf("read() returned %d\n", i);
    }

   src[i] = 0;//add EOF character
   close(fd);

   //allocate memory for virtual machine
   if(!(text = old_text = malloc(poolsize)))
   {
        printf("could not malloc(%d) for text area\n",poolsize);
        return -1;

   }

   if(!(data = malloc(poolsize)))
   {
        printf("could not malloc(%d) for data area\n",poolsize);
        return -1;

   }

   if(!(stack = malloc(poolsize)))
   {
        printf("could not malloc(%d) for stack area\n",poolsize);
        return -1;

   }

   if(!(symbols = malloc(poolsize)))
   {
        printf("could not malloc(%d) for stack area\n",poolsize);
        return -1;

   }
   
   memset(text, 0, poolsize);
   memset(data, 0, poolsize);
   memset(stack, 0, poolsize);
   memset(symbols, 0, poolsize);

   bp = sp = (int *)((int)stack+poolsize);
   ax = 0;

   #if 0//test code
   i = 0;
   text[i++] = IMM;
   text[i++] = 10;
   text[i++] = PUSH;
   text[i++] = IMM;
   text[i++] = 20;
   text[i++] = ADD;
   text[i++] = PUSH;
   text[i++] = EXIT;
   pc = text;
   #endif

   src = "char else enum if int return sizeof while"
           " open read close printf malloc memset memcmp exit void main";


   i = Char;
   while(i <= While)
   {
        next();
	  current_id[Token] = i++;
   }

   i = OPEN;
   while(i <= EXIT)
   {
        next();
	  current_id[Class] = Sys;
	  current_id[Type] = Int;
	  current_id[Value] = i++;
   }

   next();
   current_id[Token] = Char;
   next();
   idmain = current_id;
   
   program();

   return eval();

}

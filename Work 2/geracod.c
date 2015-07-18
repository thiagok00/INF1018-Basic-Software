/* Thiago Klein de Angelis 2014 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
     
typedef int (*funcp)(int p0,int p1 ,int p2, int p3, int p4 );

static unsigned char entrada[]={0x55,0x89,0xe5,0x83,0xec,0x14};
static unsigned char saida[]={0x89,0xEC,0x5d,0xc3};

int Constante (char *s)  {       
	
	int flag=0;int num=0;     
	s++;

	if ( s[0] == '-') {    
		flag=1;                       
		s++;                  
	}
                   
	for (;*s; s++)                
		num= num*10 + (*s - '0');

	return flag ?  -num : num;
}
     
static void error (const char *msg, int line) {
     
	fprintf(stderr, "erro %s na linha %d\n", msg, line);
	exit(EXIT_FAILURE);    
}
    
/*  Faz "move var1, %reg" se eax = 1 -> reg = eax, se eax = 0 -> reg = ecx
	Retorna o j atualizado.
*/

int move_reg(unsigned char *codigo,char *var1,int j,int eax,int line) {
	
	
	 if (var1[0] == '$') {    
		 int c=Constante(var1);
		
		 if (eax)       
			 codigo[j]= 0xb8;		
		 else
			 codigo[j]= 0xb9;

		 *( (int *) &codigo[j+1] ) = c;       
		 j+=5;
	 }
	 else {               
		 codigo[j]=0x8b;                  
		 codigo[j+1]=0x45;                   
		 j+=2;    
                 
		 if (var1[0] == 'p' && var1[1] >= '0' && var1[1] < '5')                  
			 codigo[j] = (var1[1]-'0')*4 +8;     
                
		 else if (var1[0] == 'v' && var1[1] >= '0' && var1[1] < '5')                 
			 codigo[j] = 252 - (var1[1]-'0')*4;
                 
		 else                   
			 error("operando invalido", line); 

		 j++;   
	 }	
	 return j;		
}

/*	Faz "op var1, %eax". Retorna j atualizado*/
int op_reg(unsigned char *codigo, char *var1, char op, int j,int line) {
	 
	if (var1[0] == '$') {

		int c=Constante(var1);       
		if (op=='+')      
			codigo[j]= 0x05 ;
     
		else if (op =='-')     
			codigo[j]= 0x2d;
     
		else if (op =='*')      
			codigo[j]= 0x69;
      
		else       
			error("operando invalido", line); 
     
		*( (int *) &codigo[j+1] ) = c;     
		j+=5;

	
	}

	else { 
                        
		if (op == '+') {
			codigo[j]=0x03;
			codigo[j+1]=0x45;
			j+=2;		
		}
                  
		else if (op =='-') {
			codigo[j]=0x2b;
			codigo[j+1]=0x45;
			j+=2;
		}
                    
		if (op=='*') {
			codigo[j]=0x0f;
			codigo[j+1]=0xaf;
			codigo[j+2]=0x45;
			j+=3;
		}   
                     
		if (var1[0] == 'p' && var1[1] >= '0' && var1[1] < '5')                          
			codigo[j] = (var1[1]-'0')*4 +8;     
                    
		else if (var1[0] == 'v' && var1[1] >= '0' && var1[1] < '5')                            
			codigo[j] = 252 - (var1[1]-'0')*4;
                   
		else                     
			error("operando invalido", line);

		j++;
	}	
	return j;
}

/*Concatena vetor 'entrada' e 'saida', se 'entrada' = 0 -> concetena o vetor 'saida'*/
int entrada_saida(unsigned char *codigo, unsigned char *v,int j,int ent) {
	
	int i;

	if (ent)	
		for (;j<6;j++)		
			codigo[j]=v[j];	
	else	
		for (i=0; i<4;i++,j++)		
			codigo[j]=v[i];

	return j;
}
void printa (unsigned char *codigo,int j) {
	int i;
	
	for(i=0;i<j;i++)	
		printf("j:%d - %X\n",i,	codigo[i]);
}



funcp geracod (FILE *myfp) {
            
	int line = 1;            
	char c;           
	unsigned char *codigo;           
	int j=0;		
	int linha[50],jumps[50][2],qtdJump=0;

			
	codigo = (unsigned char*) malloc (4096 * sizeof(unsigned char));
			
	if (codigo == NULL)				
		exit(1);		
			
	j= entrada_saida(codigo,entrada,j,1);			
	linha[0]=-1;
			
	while ((fscanf(myfp," %c",&c)) != EOF) {
			
		linha[line] = j;			
		printf("C: %c\n",c);          
		switch (c) {          
		
		case 'r': {                   
			char var1[100];               
                 
			if (fscanf(myfp,"et %s",var1)!=1)                         
				error("comando invalido i",line);                  
					
			j = move_reg(codigo,var1,j,1,line);					
			j=  entrada_saida(codigo,saida,j,0);  					
			break;				  
				  }
     
    /************************** COMPARAÇAO *****************************/
          
		case 'i': { 
                     
			char var1[100],var2[100];                     
			int posJump;    
                      
			if ( fscanf(myfp,"feq %s %s %d",var1,var2,&posJump) != 3)                    
				error("comando invalido i",line);   

			/***** Movendo var1 para o %eax *******/                  
			j= move_reg(codigo,var1,j,1,line); 
 
            /********** Movendo var2 para o %ecx ************/					   
			j= move_reg(codigo,var2,j,0,line);
                       
            /********* Comparando %eax com %ecx E JE(jump equal) ****************/               
			codigo[j]=0x39;            
			codigo[j+1]=0xc1;            
			j+=2;

			/*	Guarda na matriz jumps a linha para onde deseja se desviar o controle, e aonde começa o jump (JE) */         								
			
			jumps[qtdJump][0]= posJump;			
			jumps[qtdJump][1]= j;			
			qtdJump++;			
			codigo[j]=0x0f;			
			codigo[j+1]=0x84;			
			j+=6;          
			break;
				  }
    /*************************** ATRIBUIÇÃO **************************/  
		case 'v': case 'p': case '$': {   
                                    
			char var1[100],var2[100],op;            
			int ind

			if (fscanf(myfp,"%d := %s %c %s",&ind,var1,&op,var2)!=4)            
				error("comando invalido i",line);  
                   
            /* Movendo var1 para o eax */               
			j = move_reg(codigo,var1,j,1,line);
                                    
            /* Fazendo OP var2, %eax */                   			
			j = op_reg(codigo,var2,op,j,line);

            /* MOVENDO PARA VAR */        
			codigo[j]=0x89;            
			codigo[j+1]=0x45;            
			j+=2;    
            
			if (c == 'p' && ind >= 0 && ind < 5)            
				codigo[j] = ind*4 +8;    
                
			else if (c == 'v' && ind >= 0 && ind < 5)            
				codigo[j] = 252 - ind*4;                
			else            
				error("operando invalido", line);  
                
			j++;                                  
			break;          
				  }         
		default: error("desconhecido", line);   
		}       
		line ++; 
	}
  
	printa(codigo,j); 	

	for (j=0; j<qtdJump ; j++) {	
		*((int *) &codigo[ jumps[j][1]+2 ]) = (int) &codigo [ linha[ jumps[j][0]  ] ] - (int) &codigo[ jumps[j][1]+6 ]   ;
		 /*
			>>jumps[j][0] tem a linha que se deve desviar o controle
			>>linha[..] tem o indice de onde o comando em Minima na linha .. começa no vetor codigo

			logo, &codigo [ linha[ jumps[j][0]  ] ] é o endereço da instrução para onde deve se desviar o controle

			&codigo[ jumps[j][1]+5 ] é o endereço da proxima instrução

		   &codigo[ jumps[j][1]+1 ] é o endereço de onde deve ser posto a diferença dos dois */
	}
	printa(codigo,j);
	return (funcp) codigo;   
}

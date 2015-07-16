#include <stdio.h>
#include <stdlib.h>
#include "utfconv.h"

/***************************************************************************
*
*  Function: processadorL
*
*  $Description
*		Return 1 if processor is little endian or 0 for big endian
*
*  ****/

int processadorL(){
	int i=1;
	if(	*(int *)&i == 1)
		return 1;	

	return 0;		
}

/***************************************************************************
*
*  Function: trocaOrdem
*
*  $Description
*		Invert one variable of 4 bytes
*
*  $Example
*		1 byte 2 byte 3 byte 4 byte
*		result:
*		4 byte 3 byte 2 byte 1 byte
*
*  ****/

int trocaOrdem (unsigned int x) {
	return (x>>24) | ((x>>8) & 0xFF00) | ((x<<8) & 0xFF0000) | (x<<24);
}
/***************************************************************************
*
*  Function: trocaEscrita
*
*  $Description
*		Invert one variable of 'numB' of bytes
*
*  ****/
int trocaEscrita (unsigned int x, int numB) {
	if (numB==4)
		return (x>>24) | ((x>>8) & 0xFF00) | ((x<<8) & 0xFF0000) | (x<<24);
	if (numB==3)
		return (x & 0x00FF0000)>>16 | (x & 0x000000FF)<<16 | (x & 0xFF00);
	if (numB==2)
		return ((x & 0xFF00)>>8) | ((x & 0xFF)<<8);

return 0;
}
/***************************************************************************
*
*  Function: erro_leitura
*
*  $Description
*		Returns a read error on the file at 'posErro' line.
*
*  ****/
int erro_leitura (int posErro) {
	fprintf(stderr,"Erro arquivo de Leitura\nNumero do byte com erro: %d\n",posErro);
	return -1;
}
/***************************************************************************
*
*  Function: erro_gravacao
*
*  $Description
*		Returns a write error on the file at 'posErro' line.
*
*  ****/
int erro_gravacao (int posErro) {
	fprintf(stderr,"Erro arquivo de Gravacao\nNumero do byte com erro: %d\n",posErro);
	return -1;
}

/***************************************************************************
*
*  Function: verifica32_8
*
*  $Description
*		Verify the quantity of bytes that will be need to convert
*
*  ****/
int verifica32_8 (unsigned int x) {
	if (x < 0x80)
		return 1;
	if  (x < 0x800)
		return 2;
	if (x  < 0x10000)
		return 3;
	if (x <= 0x1FFFFF)
		 return 4;

	return -1;
}
/***************************************************************************
*
*  Function: verifica32_8
*
*  $Description
*		Verify the quantity of bytes that will be need to convert
*
*  ****/
int verifica8_32 (unsigned int x) {
    if (x>>7 == 0)
        return 1;
    if (x>>5 == 0x6)
        return 2;
    if (x>>4 == 0xE)
        return 3;
    if (x>>3 == 0x1E)
        return 4;

    return -1;
}
/***************************************************************************
*  Function: conv32_8
*  ****/
  int conv32_8(FILE *arq_entrada, FILE *arq_saida)	{   
	int numB=0, bytePos=0,flag; 
	unsigned int temp=0;
	char ordem;

	flag= processadorL();

	if (fread(&temp,4,1,arq_entrada)==1) {	
		
		bytePos+=4;
		if(flag)
			temp=trocaOrdem(temp);

		if (temp == 0xFEFF)
			ordem='B';
		else if (temp == 0xFFFE0000)
			ordem = 'L';
		else
			return erro_leitura(bytePos); //BOM error
		bytePos+=4;
  }
	else
		return erro_leitura(bytePos); //cant read BOM

	for ( temp = 0; fread(&temp,4,1,arq_entrada)==1 ; temp = 0) {
		bytePos+=4;
		//garantees that im working with big endian
		if((flag && ordem == 'B') || (!flag && ordem == 'L' )) 	
			temp = trocaOrdem(temp);
		
		numB = verifica32_8(temp);
		if (numB==-1)
			return erro_leitura(bytePos);

		if(numB==2) {		
			temp = 0xc080 | (temp & 0x7c0)<<2 | (temp & 0x03f);
		}
		
		else if(numB==3) {
			temp = 0xe08080 | ( temp & 0xf000)<<4 | (temp & 0x0fc0)<<2 | (temp & 0x003f);
		}
		
		else if(numB==4) {
			temp = 0xf0808080 | (temp & 0x1C0000)<<6 | (temp & 0x03f000)<<4 | (temp & 0x000fc0)<<2 | (temp & 0x00003f);
		}			
		if (flag && numB != 1) 
			temp=trocaEscrita(temp,numB);
	
		if(fwrite(&temp,numB,1,arq_saida)==1);
		else	
			return erro_gravacao (bytePos);
	}
	return 0;
  }

/***************************************************************************
*  Function: conv8_32
*  ****/
  int conv8_32(FILE *arq_entrada, FILE *arq_saida, char ordem) {	
	  
	  unsigned int numB,temp=0;		
	  int bytePos=0,flag;		
	  flag = processadorL();
		
	  /* Writing the BOM */
	  
	  if( ordem == 'L')	  
		  temp=0xFFFE0000;
	  
	  else if (ordem == 'B')	  
		  temp=0x0000FEFF;

	
	  if((flag && ordem == 'B') || (flag && ordem == 'L' ))		
		  temp = trocaOrdem(temp);
	  
	  if(fwrite (&temp,sizeof(temp),1,arq_saida)==1);	  
	  else		
		  return erro_gravacao(bytePos);

	  temp=0;
	
	  while(fread(&temp,1,1,arq_entrada)==1) {

		  numB=verifica8_32 (temp);

		  if (numB == -1 )
			  return erro_leitura(bytePos);
		  else if (numB == 2) {

			  unsigned int aux=0;
			  bytePos++;				
			  if(fread(&aux,1,1,arq_entrada)==1 && (aux>>6 ==2))				
				  temp=(temp & 0x1F)<<6 | (aux & 0x3F);
				
			  else				
				  erro_leitura(bytePos);
		  }
			
		  else if (numB == 3) {

			  unsigned int aux=0;
			  bytePos++;				
			  temp = (temp & 0x0F)<<12;
				
			  if(fread(&aux,1,1,arq_entrada)==1 && (aux>>6 ==2)){
				  
				  bytePos++;		
				  temp = temp | (aux & 0x3F)<<6; 
				  aux=0;

				  if(fread(&aux,1,1,arq_entrada)==1 && (aux>>6 ==2))
					  temp = temp | (aux & 0x3F);
				  else
					  return erro_leitura(bytePos);			
			  }
			  else
				  return erro_leitura(bytePos);
			}		
		
		  else if (numB == 4) {
				
			  unsigned int aux=0;	
			  bytePos++;
				
			  if(fread(&aux,1,1,arq_entrada)==1 && (aux>>6 ==2)) {
				  bytePos++;	
				  temp = (temp & 0x07)<<18 | (aux & 0x3F)<<12;
				  aux=0;
					
				  if(fread(&aux,1,1,arq_entrada)==1 && (aux>>6 ==2)) {
					  
					  bytePos++;	
					  temp= temp | (aux & 0x3F)<<6;
					  aux=0;
					  if(fread(&aux,1,1,arq_entrada)==1 && (aux>>6 ==2)){
						  bytePos;
						  temp= temp | (aux & 0x3F);		
					  }
					  else
						  return erro_leitura(bytePos);			
				  }
				  else
					  return erro_leitura(bytePos);
				}
		}
		  if((flag && ordem == 'B') || (!flag && ordem == 'L' )) 	
			  temp = trocaOrdem(temp);

		  if(fwrite (&temp,sizeof(temp),1,arq_saida)==1);
		  else
			  return erro_gravacao(bytePos);
			
		  bytePos++;
			
		  temp=0;
 }
return 0;
  }
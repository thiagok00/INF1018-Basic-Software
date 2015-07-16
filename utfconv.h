/*	Thiago Klein de Angelis	2014	*/


/***********************************************************************
*
*  $FC Function: conv32_8
*
*  $ED Description
*     Convert File from utf32 to utf8
*
*  $EP Parameters
*     *arq_entrada / input file
*	  *arq_saida / output file
*
*
***********************************************************************/

int conv32_8(FILE *arq_entrada, FILE *arq_saida);

/***********************************************************************
*
*  $FC Function: conv8_32
*
*  $ED Description
*     Convert File from utf8 to utf32
*
*  $EP Parameters
*     *arq_entrada / input file
*	  *arq_saida / output file
*	  ordem - the order of file 'L' for little-endian / 'B' for big-endian
*
***********************************************************************/
int conv8_32(FILE *arq_entrada, FILE *arq_saida, char ordem);
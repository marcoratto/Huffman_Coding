#include "huffman.h"

void usage() {
	fprintf(stderr, "Uso: huffman [-e/-d] infile outfile\n");
}

int main(int argc, char *argv[])
{
int E_flag;

if ((argc != 4) || (argv[1][0] != '-')) {
	usage();
	exit(1);
}

switch( tolower( argv[1][1])) {
case 'e':
	E_flag = TRUE;
	break;
case 'd':
	E_flag = FALSE;
	break;
default : /* parametro sconosciuto */
	usage();
	exit(1);
break;
}

if (E_flag) 
	encode(argv[2], argv[3]);
else
	decode(argv[2], argv[3]);
	
return(0);
}



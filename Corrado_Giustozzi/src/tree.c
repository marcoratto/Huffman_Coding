#include "huffman.h"

/*
Routine di gestione dell'albero binario
(C) Copyright 1988,89 Corrado Giustozzi
Uso e riproduzione di questo codice sono concessi
per utilizzo personale ed a fini non conmerciali.
*/

/*
Scrive l'albero su outfile in modo ricorsivo
*/
void writetree(NODO *p, FILE *outfile)
{
	if (( p->sin == NULL ) && ( p->des == NULL) ) {
		putbit(1, outfile );
		putbyte(p->car, outfile );
	} else {
		putbit( 0, outfile );
		writetree( p->sin, outfile );
		writetree( p->des, outfile );
	}
}

/*
  Legge 1'albero da infile in modo ricorsivo
*/
NODO *readtree(FILE *infile)
{
	NODO *p;
	if (getbit( infile ) ) {
		if ((p = (NODO *)malloc( sizeof( NODO ) ) ) == NULL) {
			fprintf(stderr, "Impossibile allocare nodo.\n" );
			exit(1);
		}
		p->car = getbyte( infile );
		p->sin = p->des = NULL;
	} else {
		if ((p = (NODO *)malloc( sizeof( NODO ) ) ) == NULL) {
			fprintf(stderr, "Impossibile allocare nodo.\n" );
			exit(1);
		}
		p->sin = readtree( infile );
		p->des = readtree( infile );
		p->sin->su = p->des->su = p;
	}
	return( p );
}

/*
Dealloca l'albero binario dalla memoria in modo ricorsivo
*/
void killtree(NODO *p) 
{
	if ( p != NULL ) {       
		killtree( p->sin );
		killtree( p->des );
		// p->sin = NULL;
		// p->des = NULL;
		free(p);
	}
}

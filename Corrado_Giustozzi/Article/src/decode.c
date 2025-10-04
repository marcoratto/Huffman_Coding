#include "huffman.h"

void decode(char *infilename, char *outfilename) 
{
	int c;
	word newcrc;
	HEADER hdr;
	NODO *albero;
	FILE *infile, *outfile;
	struct utimbuf t;

#if defined( DEBUG )
	printf("*\nDecodifica: %s --> %s\n\n", infilename, outfilename);
#endif

	/* apertura dei file */
	if ((infile = fopen( infilename, READ)) == NULL ) {
		fprintf( stderr, "Impossible aprire infile %s\n", infilename );
		exit( 1);
	}
	if (( outfile = fopen( outfilename, WRITE)) == NULL) {
		fprintf(stderr, "Impossibile aprire outfile %s\n", outfilename );
		exit( 1);
	}
	/* lettura header © albero di decodifica da infile */
	albero = readheader(&hdr, infile);
	
	/* deconpressione */
	initcrc();
	while ( hdr.lunghezza-- ) {
		c = readhuf(albero, infile);
#if defined( DEBUG )
	printf( "Scrivo %d\n", c);
#endif        
		newcrc = chrcrc( (byte) c );
		putc(c, outfile );
	}
	if (newcrc != hdr.crc ) 
		fprintf( stderr, "Attenzione: CRC errato!\n" );
	
	fflush(outfile);

	/* chiusura dei file */
	if ( fclose( infile ) == EOF )
		fprintf(stderr, "Errore chiudendo file %s\n", infilename );
	
	if ( fclose(outfile ) == EOF) {
		fprintf( stderr, "Errore chiudendo file %s\n", outfilename );
		exit (1);
	}

	/* ripristino della data originale del file */
	t.actime = t.modtime = hdr.timestamp;
	if ( utime( outfilename, &t) == -1)
		fprintf(stderr, "Inpossibile modificare data del file %s\n", outfilename);

	/* deallocazione dell'albero di decodifiea */
	killtree( albero );
}

/*
 	Legge da infile lo header con le infornazioni
	sul file originale e I'albero di decodifica
*/
NODO *readheader(HEADER *hdr, FILE *infile)
{
	byte c[12], sum=0;
	register i;

	/* Lettura dello header byte per byte e controllo checksum */
	for (i = 0; i<= 11; i++)
		sum += ( c[i] = (byte) getc(infile));

	if (sum != 255) {
		fprintf(stderr, "Header corrotto.\n" );
		exit(1);
	}

	/* Ripristino valori dello header */
	hdr->algoritmo = c[0];
	hdr->checksum = c[1];
	hdr->crc = (word) c[2];
	hdr->crc <<= 8;
	hdr->crc &= 0xFF00;
	hdr->crc |= ( (word) c[3] & 0x00FF );

	hdr->lunghezza = (long) c[4];
	for (i = 5; i<=7; i++) {
		hdr->lunghezza <<= 8;
		hdr->lunghezza &= 0xFFFFFF00L;
		hdr->lunghezza |= (long) (c[i] & 0x000000FFL);
	}

	hdr->timestamp = (long)c[8];
	for (i = 9; i <= 11; i++) {
		hdr->timestamp <<= 8;
		hdr->timestamp &= 0xFFFFFF00L;
		hdr->timestamp |= (long) (c[i] & 0x000000FFL);
	}

	/* lettura albero di decodifica */
	return(readtree(infile));
}

/*
 Legge da infile il codice di Huffman e
 Ritorna il carattere corrispondente
 */
int readhuf(NODO *p, FILE *infile) 
{
	do
		p = getbit( infile ) ? p->des : p->sin;
	while ((p->sin != NULL) || (p->des != NULL));
	return ((int) p->car );
}


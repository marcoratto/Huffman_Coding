#include "huffman.h"

static int buffer=0, pos=0;

void putbyte(byte outbyte, FILE *outfile) 
{
	word i, c = 0;
	c = (word) outbyte;
	for (i = 8; i; i--) {
		putbit( (c & 128) >> 7, outfile);
		c <<= 1;
	}
}

byte getbyte(FILE *infile)
{
	word i, c = 0;
	for (i= 8; i; i--) {
		c <<= 1;
		c |= getbit( infile );
}
	return( (byte) c);
}

void putbit(int outbit, FILE *outfile)
{
	buffer <<= 1;
	buffer |= outbit;
	if ( ++pos == 8) {
		if ( putc( buffer, outfile ) == EOF) {
			fprintf( stderr, "Errore di scrittura su outfile\n" );
			exit(1);
		}
		buffer = pos = 0;
	}
}

bit getbit(FILE *infile)
{
	int res;
	bit c;
	if ( pos-- == 0) {
		if ( ( res = getc(infile ) ) == EOF ) {
			fprintf( stderr, "EOF inatteso\n" );
			exit(1);
		}
		buffer = (word)res;
		pos = 7;
	}
	c = ( buffer & 128 ) >> 7;
	buffer <<= 1;
	return (c);
}

void flushbit(FILE *outfile) {
	buffer <<= ( 8 - pos);
	putc( buffer, outfile );
	buffer = pos = 0;
}


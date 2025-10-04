#include "huffman.h"

#define POLY 0x8408
#define MASK 0x0001

static word crcres;

/*
 Inizializzazione CRC
*/
void initcrc() {
	crcres = 0;
}

/*
Conteggia il carattere c
*/
word chrcrc(byte c)
{
	static word buf, q;
	register i;
	buf = (unsigned) c;

	for (i= 8; i; i--) {
		q = (crcres & MASK) ^ ( buf & MASK );
		crcres >>= 1;
		if (q == MASK )
			crcres ^= POLY;
		
		buf >>= 1;
	}

	return(crcres);
}

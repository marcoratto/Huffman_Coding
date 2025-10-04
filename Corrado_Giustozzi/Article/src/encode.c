#include "huffman.h"

static ELEM tab[256];
static ENTAB enc[256];

void encode(char *infilename, char *outfilename)  {
	int c;
	HEADER hdr;
	NODO *albero;
	FILE *infile, *outfile;
	struct stat f;

#if defined(DEBUG)
	printf("\nCodifica: %s > %s\n", infilename, outfilename);
#endif

	/* apertura dei file */

	if ((infile = fopen(infilename, READ)) == NULL ) {
		fprintf( stderr, "Inpossibile aprire infile %s\n", infilename );
		exit(1);
	}

	if ((outfile = fopen( outfilename, WRITE)) == NULL ) {
		fprintf(stderr, "Impossibile aprire outfile %s\n", outfilename );
		exit( 1);
	}
	/* inizializzazione dello header */
	fstat(fileno(infile), &f);

	hdr.algoritmo = 1;
	hdr.lunghezza = f.st_size;
	hdr.timestamp = f.st_ctime;

#if defined(DEBUG)
	printf("Lunghezza del file: %08llx\n", f.st_size);
    printf("Timestamp del file: %08lx\n", f.st_ctime);
    printf("\nI fase: creazione codice di Huffman\n\n" );
#endif

	albero = maketree( infile, &hdr );
	writeheader( &hdr, albero, outfile );

#if defined (DEBUG)
	printf( "\nII fase: codifica\n\n" );
#endif

	rewind( infile );

	while ((c = getc(infile )) != EOF) 
		writehuf(c, outfile);
		
	flushbit(outfile);
    
    fflush(outfile);

	if (fclose(infile) == EOF)
		fprintf(stderr, "Errore chiudendo file %s\n", infilename );

	if (fclose(outfile) == EOF) {
		fprintf(stderr, "Errore chiudendo file %s\n", outfilename );
		exit (1);
	}

	killtree(albero);
}

/* 
 Costruisce l'albero di Huffman e la tabella di codifica
*/
NODO *maketree(FILE *in, HEADER *hdr)
{

	long tot = 0, origbit, hufbit = 1, totbit;
	word crc = 0;
	int max, hdrbit, treebit, p1, p2, c[MAXHUFBIT], len;
	NODO *t, *radice;
	LISTA *lista;
	register i;

	/* inizializzazione tabella caratteri */
	for (i = 0; i < 256; i++) {
		tab[i].num = 0;
		tab[i].foglia = NULL;
	}

	/* creazione tabella frequenze dal file e calcoto crc */
#if defined(DEBUG )
	printf( "\nAnalisi del file di input\n" );
#endif

	initcrc();

	while((i= getc(in)) != EOF ) {
		tab[i].num++;
		crc = chrcrc((byte) i);
	}
#if defined(DEBUG )
	printf( "\ncrc: %04x\n", crc);
#endif
	hdr->crc = crc;

	/* conteggio caratteri distinti */
	max = -1;

	for (i=0; i < 256; i++)
		if (tab[i].num) {
			++max;
			tot += tab[i].num;
		}

	if (max == -1) {
		fprintf(stderr, "Letti 0 caratteri.\n" );
		exit(1);
	}

#if defined(DEBUG )
	printf( "\tSize: (file) %ld, (huffman) %ld\n", hdr->lunghezza, tot );
#endif

	if (hdr->lunghezza != tot) {
		fprintf( stderr, "Incoerenza nella lunghezza del file.\n" );
		exit(1);
	}
		
	origbit = tot * 81;
	hdrbit = 8 * sizeof( HEADER );
	treebit = (10 * max) + 9;

	#if defined(DEBUG)
		printf( "\tLetti %ld caratteri, %d diversi.\n", tot, max+1);
	#endif

	/* allocazione lista nodi esterni della foresta */
	if ( ( lista = (LISTA *)calloc( max+1, sizeof( LISTA ) ) ) == NULL ) {
		fprintf( stderr, "Impossibile allocare lista.\n" );
		exit( 1);
	}

	/* creazione foresta per i soli caratteri presenti */
#if defined(DEBUG )
	printf("\nCreazione foresta\n");
#endif

	max = -1;
	for (i= 0; i < 256; i++)
		if (tab[i].num) {
			++max;
#if defined(DEBUG)
			printf( "\t%3d (%3d) %c %5ld\n", max, i, (isprint(i) ? i : ' '), tab[i].num);
#endif

			lista[max].peso = tab[i].num;
			if ((t= (NODO *)malloc( sizeof( NODO ) ) ) == NULL) {
				fprintf(stderr, "Impossibile allocare nodo.\n" );
				exit(1);
			}
			tab[i].foglia = lista[max].testa = t;
			t->su = t->sin = t->des = NULL;
			t->car = (char) i;
		}
		
	/* fusione della foresta in un unico albero binario */

#if defined(DEBUG )
	printf( "\nCreazione albero binario\n" );
#endif

	while (max > 0) {
		p1 = selectmin(lista, max--);
		p2 = selectmin(lista, max);

	#if defined(DEBUG)
		printf( "\t%3d: %3d (%5ld) + %3d (%5ld) ", max, p1, lista[p1].peso, p2, lista[p2].peso);
	#endif

		if ((t = (NODO *) malloc (sizeof(NODO))) == NULL) {
			fprintf( stderr, "Inpossibile allocare nodo.\n" );
			exit(1);
		}

		t->car = 0;
		t->su = NULL;
		t->sin = lista[p1].testa;
		t->des = lista[p2].testa;
		t->sin->su = t->des->su = t;

		lista[p2].peso += lista[p1].peso;
		lista[p2].testa = t;

#if defined (DEBUG )
		printf( "= %3d (%5ld)\n", p2, lista[p2].peso);
#endif
	}

	radice = lista[0].testa;

	free(lista);

/* costruzione tabella codifica caratteri */
#if defined(DEBUG )
		printf( "\nCreazione tabella di codifica\n" );
#endif

	for ( i = 0; i < 256; i++)
		if ((t= tab[i].foglia) != NULL ) {
			len = -1;
			while (t->su != NULL) {
				if (++len >= MAXHUFBIT ) {
					fprintf ( stderr, "Albero degenerato (%d bit).\n", len+1);
					exit(1);
				}
				c[len] = (t->su->sin != t);
				t = t->su;
			}
			enc[i].car = (char) i;
			enc[i].len = (byte)(len + 1);
			enc[i].cod = gethufcode(c, len);
			hufbit += (long)tab[i].num * (long )enc[i].len;
			
#if defined(DEBUG)
			printf("\t%c (%3d) : %2d %8lX :", (isprint(i) ? i :'.'), i, enc[i].len, enc[i].cod);
			for ( ; len >=0; len--)
				printf( "%01d", c[len]);
				
			printf("\n");
#endif
	} else
		enc[i].cod = enc[i].car = enc[i].len = 0;

	totbit = (long) hdrbit + (long)treebit + hufbit;

#if defined( DEBUG )
	printf( "\nStatistiche sulla compressione\n" );
	printf( "\tBit nel file originario: %ld\n", origbit );
	printf( "\tBit nell'header: %d\n", hdrbit );
	printf( "\tBit nell'albero: %d\n", treebit );
	printf( "\tBit nel file codificato: %ld\n", hufbit );
	printf( "\tBit complessivi utilizzati:  %ld\n", totbit );
	printf( "\tDelta di compressione: %ld\n", 100*(totbit-origbit)/origbit);
#endif

	return(radice);
}

/*
seleziona l'elemento avente peso minore
e lo posiziona in coda alla lista
*/
int selectmin(LISTA l[], int maxel)
{
	int p; 
	long min = LONG_MAX;
	register i, j;
	LISTA t;

	for(j=0, i = maxel; j<maxel; j++, i--) {
		if (l[i].peso < min ) {
			min = l[i].peso;
			p = i;
		}
    }

	t = l[maxel];
	l[maxel] = l[p];
	l[p] = t;
	return( maxel );
}

/*
ritorna una dword i cui bit sono alti o bassi
secondo i corrispondenti elementi di c
*/
dword gethufcode(int c[MAXHUFBIT+1], int l)
{
	dword buf = 0;
	register i;
	for (i = 0; i<= l; i++) {
		buf <<= 1;
		buf |= c[i];
	}

	return(buf);
}

/*
Scrive su outfile lo header con le informazioni
sul file originale l'albero di decodifica
*/
void writeheader(HEADER *hdr, NODO *albero, FILE *outfile)
{
	byte c[12], sum = 0;
	register i;
	
/* Formazione header come array di char per portabilita’ */
c[0] = hdr->algoritmo;
c[1] = 0;
c[2] = (byte) (( hdr->crc >> 8 ) & 0xFF);
c[3] = (byte)  ( hdr->crc & 0XFF);
c[4] = (byte) (( hdr->lunghezza >> 24 ) & 0XFFL );
c[5] = (byte) (( hdr->lunghezza >> 16 ) & 0XFFL );
c[6] = (byte) (( hdr->lunghezza >> 8 ) & 0xFFL );
c[7] = (byte)  ( hdr->lunghezza & 0xFFL );
c[8] = (byte) (( hdr->timestamp >> 24 ) & 0XFFL);
c[9] = (byte) (( hdr->timestamp >> 16 ) & 0XFFL);
c[10] = (byte)(( hdr->timestamp >> 8 ) & 0XFFL );
c[11] = (byte) ( hdr->timestamp & 0XFFL );

/* calcolo del checksum dello header */
for (i = 0; i <= 11; i++)
	sum += c[i];

c[1] = hdr->checksum = (byte)( 255 - sum);

#if defined(DEBUG)
    printf("checksum header: %02x", (int) c[1]);
#endif

/* Scrittura dello header byte per byte (per portabilita') */

for (i = 0; i <= 11; i++)
	putc( c[i], outfile);

/* Scrittura dell'albero di decodifica */
writetree( albero, outfile );

}

/*
Scrive su outfile 11 codice di Huffman
corrispondente al carattere c
*/
void writehuf(int c, FILE *outfile) 
{
    register i;
	dword j;
	i = enc[c].len;
	j = enc[c].cod;

#if defined( DEBUG )
	printf("Scrivo '%c' (%3d) : %2d %8lX\n", (isprint(c) ? c : '.'), c, i, j);
#endif

	do {
		putbit((bit)( j & 1), outfile );
		j >>=1;
	} while (--i);

}


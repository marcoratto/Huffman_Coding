#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <limits.h> 

#include "stuff.h"

#define MAXHUFBIT 8*sizeof(dword)

struct elem {
	long num;
	struct nodo *foglia;
};

struct lista {
		long peso;
		struct nodo *testa;
};

struct nodo {
	char car;
	struct nodo *su;
	struct nodo *sin;
	struct nodo *des;
};

struct entab {
	char car;
	byte len;
	dword cod;
};

struct header {
	byte algoritmo;
	byte checksum;
	word crc;
	long lunghezza;
	long timestamp;
};

typedef struct elem ELEM;
typedef struct lista LISTA;
typedef struct nodo NODO;
typedef struct entab ENTAB;
typedef struct header HEADER;

void usage();

NODO *maketree(FILE *in, HEADER *hdr);
dword gethufcode(int c[MAXHUFBIT+1], int l);
// int selectmin(LISTA l[], int maxel);
int selectmin(LISTA *l, int maxel);

void encode(char *infilename, char *outfilename);

void decode(char *infilename, char *outfilenane);
int readhuf(NODO *p, FILE *infile);

NODO *readheader(HEADER *hdr, FILE *infile);
void writeheader(HEADER *hdr, NODO *albero, FILE *outfile);

NODO *readtree(FILE *infile);
void writetree(NODO *p, FILE *outfile);
void killtree(NODO *p);

void putbit(int outbit, FILE *outfile);
bit getbit(FILE *infile);
void writehuf(int c, FILE *outfile);

void flushbit(FILE *outfile);
void putbyte(byte outbyte, FILE *outfile);
byte getbyte(FILE *infile);

void initcrc();
word chrcrc(byte c);

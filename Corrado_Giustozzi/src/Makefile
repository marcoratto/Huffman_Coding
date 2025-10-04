CFLAGS = -Wall

OBJ = bitstrm.o \
	chrcrc.o \
	decode.o \
	encode.o \
	huffman.o \
	tree.o 
 
SRCS = $(OBJ:%.o=%.c)

huffman: $(SRCS)
	gcc -g -ggdb -o $@ $(CFLAGS) $(LIBS) $(SRCS)

.PHONY: clean
clean:
	rm huffman

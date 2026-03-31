prog: main.o huffman.o
	gcc -o prog main.o huffman.o
main.o: main.c huffman.h
	gcc -c main.c
huffman.o: huffman.c huffman.h 
	gcc -c huffman.c
clean:
	del prog.exe main.o huffman.o
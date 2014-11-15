.PHONY: clean

main:main.c typingDiff.o
	gcc main.c typingDiff.o -o main

typingDiff.o:typingDiff.c
	gcc -c typingDiff.c -o typingDiff.o

clean:
	rm -rf *.o
	rm -rf main
	rm -rf main.exe

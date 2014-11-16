.PHONY: clean

sample:sample.c typingDiff.o
	gcc sample.c typingDiff.o -o sample

typingDiff.o:typingDiff.c
	gcc -c typingDiff.c -o typingDiff.o

clean:
	rm -rf *.o
	rm -rf sample
	rm -rf sample.exe

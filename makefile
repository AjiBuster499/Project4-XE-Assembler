project4: main.o symbols.o
	gcc -o project4 -Wall -O0 main.o symbols.o -lm

main.o: main.c
	gcc -c -g -Wall -O0 main.c

symbols.o: symbols.c
	gcc -c -g -Wall -O0 symbols.c

clean:
	rm *.o -f
	touch *.c
	rm project4 -f

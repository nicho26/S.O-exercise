
main: main.o fast_trigo.o
	
	gcc -o main main.o fast_trigo.o -lm
	
main.o: main.c fast_trigo.h

	gcc -c  main.c -lm
	
fast_trigo.o: fast_trigo.c fast_trigo.h

	gcc -c fast_trigo.c -lm 

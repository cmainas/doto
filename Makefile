doto: doto.o
	gcc doto.o -o doto

doto.o: doto.c
	gcc -c doto.c

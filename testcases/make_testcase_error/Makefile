all: make4061_test
	echo 'make4061_test'

make4061_test: util.a main.o 
	gcc -o make4061_test main.o util.a

main.o: main.c
	gcc -c main.c

util.a: parse.o cal.o 
	ar rcs util.a parse.o cal.o

cal.o: cal.c
	gcc -c cal.c

parse.o: parse.c
	gcc -c parse.c

clean:
	rm -rf main.o cal.o parse.o util.a make4061_test

mysh : prog.c
	gcc prog.c -o prog

clean : prog
	rm -rf prog

all : clean mysh
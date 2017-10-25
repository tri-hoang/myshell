.PHONY: all clean

all: myshell

myshell: myshell.c
	gcc myshell.c -lreadline; mv a.out myshell

clean:
	-rm myshell

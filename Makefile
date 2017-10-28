.PHONY: all clean

all: myshell

myshell: myshell.c
	gcc myshell.c -lreadline -o myshell

clean:
	-rm myshell

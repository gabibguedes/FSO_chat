all:
	gcc -o chat -lrt -lpthread  chat.c

run:
	./chat
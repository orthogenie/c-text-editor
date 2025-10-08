EXEC = kilo

rebuild: clean kilo

kilo: kilo.c
	$(CC) kilo.c -o kilo -Wall -Wextra -pedantic -std=c99

clean:
	rm -f *.o $(EXEC)
	clear

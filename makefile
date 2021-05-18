CC = gcc
FLAGS = -Wall -g

all:ipv4 ipv6

ipv4: ipv4.o
	$(CC) $(FLAGS) ipv4.o -o ipv4_flood

ipv6: ipv6.o
	$(CC) $(FLAGS) ipv6.o -o ipv6_flood

ipv4.o: ipv4.c
	$(CC) $(FLAGS) -c ipv4.c -o ipv4.o

ipv6.o: ipv6.c
	$(CC) $(FLAGS) -c ipv6.c -o ipv6.o	
	
.PHONY:clean all

clean:
	rm -f *.o ipv4_flood ipv6_flood

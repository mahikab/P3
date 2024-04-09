# CC=gcc
# CFLAGS= -Wall -Werror -std=gnu99 -g -O0

# all: mysh

# run.o: run.c $(wildcard *.h)
# 	$(CC) $(CFLAGS) -c run.c
# parse.o: parse.c $(wildcard *.h)
# 	$(CC) $(CFLAGS) -c parse.c
# main.o: main.c $(wildcard *.h)
# 	$(CC) $(CFLAGS) -c main.c

# mysh: run.o parse.o main.o $(wildcard *.h)
# 	$(CC) $(CFLAGS) -o $@ main.o run.o parse.o 

# clean:
# 	rm -f mysh $(wildcard *.h)

# test: all
# 	./runtests.sh
CC=gcc
CFLAGS= -Wall -Werror -std=gnu99 -g -O0

SRCS = main.c run.c parse.c glob.c
OBJS = $(SRCS:.c=.o)

all: mysh
%.o: %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c -o $@ $<

mysh: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) 

clean:
	rm -f mysh $(OBJS)

test: all
	./runtests.sh

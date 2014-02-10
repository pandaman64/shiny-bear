
TARGET=tweet

CFLAGS= -ansi -std=c99 -c -W -Wall -Werror -DDEBUG -g
LDFLAGS= -lssl -loauth

all: $(TARGET)

OBJS = \
	webclient.o \
	tweet.o \
	main.o

$(TARGET): $(OBJS)
	gcc -o $(TARGET) $(OBJS) $(LDFLAGS)

.c.o :
	gcc $(CFLAGS) $<
clean:
	-rm $(TARGET)
	-rm *.o
	
webclient.o : webclient.c
webclient.c : webclient.h
tweet.o : tweet.c
tweet.c : tweet.h
main.o : main.c

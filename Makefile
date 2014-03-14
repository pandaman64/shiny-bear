
TARGET=tweet
CC=gcc
CFLAGS= -ansi -std=c99 -c -W -Wall -Werror
LDFLAGS= -lcurl -loauth -lcrypto
OBJS = \
	main.o
LIBS = tweet.a

all: $(TARGET)

lib: $(LIBS)

$(TARGET): $(OBJS) $(LIBS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)

.o.a:
	ar rv $*.a $<

.c.o :
	gcc $(CFLAGS) -O2 $<

clean:
	-rm $(TARGET)
	-rm *.o
	-rm *.a


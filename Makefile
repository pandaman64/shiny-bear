
TARGET=tweet
CC=gcc
CFLAGS= -ansi -std=c99 -c -W -Wall -Werror
DCFLAGS=-g
LDFLAGS= -lcurl -loauth -lcrypto -lphobos2 -lpthread
OBJS = \
	dmain.o
LIBS = tweet.a

all: $(TARGET)

lib: $(LIBS)

$(TARGET): $(OBJS) $(LIBS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)

.SUFFIXES: .d

.o.a:
	ar rv $*.a $<

.c.o :
	gcc $(CFLAGS) -O2 $<

.d.o:
	dmd $(DCFLAGS) -c $<
	
clean:
	-rm $(TARGET)
	-rm *.o
	-rm *.a


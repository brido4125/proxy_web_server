CC = gcc
TARGET = server
OBJS = server.o csapp.o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

csapp.o :
	$(CC) -c csapp.c

server.o :
	$(CC) -c server.c

clean :
	rm -f $(OBJS) $(TARGET)

CC = gcc
TARGET = client
OBJS = echoclient.o csapp.o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

csapp.o :
	$(CC) -c csapp.c

echoclient.o :
	$(CC) -c echoclient.c

clean :
	rm -f $(OBJS) $(TARGET)

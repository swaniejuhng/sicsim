OBJECTS = 20150959.o first_project.o second_project.o third_project.o
SRC = 20150959.c first_project.c second_project.c third_project.c

CC = gcc
CFLAGS = -g -Wall
TARGET = 20150959.out

all : $(TARGET)

$(TARGET) : $(OBJECTS) 20150959.h
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm


clean :
	rm -rf $(OBJECTS) $(TARGET)
$(OBJECTS) : $(SRC) 20150959.h

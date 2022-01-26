#Make file that is probably bad
#Compiler Program
CC = g++

#Compiler Flags:
#-Wall turn on all warnings
#-std=c++17 use c++ 17 standard
CFLAGS = -Wall -std=c++17

#Linker options
#link sqlite3 lib
#link pthread
#link argon2 from ../libs
LFLAGS = -lsqlite3 -lpthread -L../libs -largon2 

#Build Target
TARGET = listener

#Output Location
BIN_DIR = ../bin

#Source Location
SRC_DIR = ../src

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$(TARGET) $(SRC_DIR)/* $(LFLAGS)

clean:
	rm $(BIN_DIR)/*

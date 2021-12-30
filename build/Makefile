#Make file that is probably bad
#Compiler Program
CC = g++

#Compiler Flags:
#-Wall turn on all warnings
#-Werror turn all warnings into errors - write some clean code lads
#-std=c++17 use c++ 17 standard
CFLAGS = -Wall -Werror -std=c++17

#Linker options
LFLAGS = -lsqlite3 -lpthread #Link sqlite3 and pthread libraries

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

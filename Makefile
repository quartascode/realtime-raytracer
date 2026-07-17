CC = g++

CFLAGS = -Wall -Wextra -Iinclude

LDFLAGS = -lraylib

SRC = $(wildcard src/*.cpp)

TARGET = raytracer

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

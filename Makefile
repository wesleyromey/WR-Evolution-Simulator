all:
	g++ -I src/include -L src/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
#	g++ -I src/include -L src/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image --debug


# A simple Makefile for compiling small SDL projects:
#	See https://www.geeksforgeeks.org/sdl-library-in-c-c-with-examples/
#	NOTE: This is NOT implemented in the actual program!
#CC := clang
#CFLAGS := `sdl2-config --libs --cflags` --ggdb3 -O0 --std=c99 -Wall -lSDL2_image -lm
#HDRS := 
#SRCS := main.cpp
#OBJS := $(SRCS:.cpp=.o)
#EXEC := main.exe
#all: $(EXEC)
#showfont: showfont.c Makefile
#	$(CC) -o $@ $@.c $(CFLAGS) $(LIBS)
#glfont: glfont.c Makefile
#	$(CC) -o $@ $@.c $(CFLAGS) $(LIBS)
#$(EXEC): $(OBJS) $(HDRS) Makefile
#	$(CC) -o $@ $(@:.o=.c) -c #(CFLAGS)
#clean:
#	rm -f #(EXEC) $(OBJS)
#.PHONY: all clean
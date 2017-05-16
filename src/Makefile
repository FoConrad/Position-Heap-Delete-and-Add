EXE = driver
CPP_FLAGS = -Wall -Wextra -g 
.SUFFIXES:
.SUFFIXES: .o .cpp

.cpp.o:
	g++ $(CPP_FLAGS) -c $*.cpp

all: driver.o downNode.o heap.o file.o generic.o mylist.o
	g++ driver.o downNode.o heap.o file.o generic.o mylist.o -o $(EXE)

clean:
	rm -vf *.o $(EXE)

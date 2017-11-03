CPPFLAGS=-g -W -Wall -Wpedantic -std=c++14 -I./include/
LDFLAGS=-g
LDLIBS=-lstdc++


main: main.o 
	g++ $(LDFLAGS) -o main main.o $(LDLIBS) 

main.o:
	g++ $(CPPFLAGS) -c main.cpp

clean:
	rm main *.o

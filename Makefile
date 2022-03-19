COMPILE=g++
STD=-std=c++17
CFLAGS=-Wall -pedantic 


make: barriers.cc
	$(COMPILE) $(STD) $(CFLAGS) -o barriers barriers.cc

clean:
	rm barriers
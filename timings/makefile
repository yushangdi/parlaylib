ifeq (, $(shell which jemalloc-config))
JEMALLOC =
else
JEMALLOCLD = $(shell jemalloc-config --libdir)
JEMALLOC = -L$(JEMALLOCLD) -ljemalloc 
endif

INCLUDE = -I../include/parlay
CFLAGS = -mcx16 -O3 -std=c++17 -march=native -Wall -Wextra $(INCLUDE)

OMPFLAGS = -DOPENMP -fopenmp
CILKFLAGS = -DCILK -fcilkplus
HGFLAGS = -DHOMEGROWN -pthread

ifdef CLANG
CC = clang++-5.0
PFLAGS = $(CILKFLAGS)
else ifdef CILK
CC = g++
PFLAGS = $(CILKFLAGS)
else ifdef OPENMP
CC = g++
PFLAGS = $(OMPFLAGS)
else ifdef HOMEGROWN
CC = g++
PFLAGS = $(HGFLAGS)
else ifdef SERIAL
CC = g++
PFLAGS =
else # default is homegrown
CC = g++
PFLAGS = $(HGFLAGS)
endif

time_tests:	$(AllFiles) time_tests.cpp time_operations.h
	$(CC) $(CFLAGS) $(PFLAGS) time_tests.cpp -o time_tests $(JEMALLOC)

all:	time_tests

clean:
	rm -f time_tests

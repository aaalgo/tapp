CC = g++

LDLIBS += -lboost_date_time -lta_lib

all:	example example2

example.o:	example.cpp ta++.h ta++-plot.h

example2.o:	example2.cpp ta++.h ta++-plot.h


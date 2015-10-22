CC = g++

LDLIBS += -lboost_date_time -lta_lib

all:	example

example.o:	example.cpp ta++.h ta++-plot.h


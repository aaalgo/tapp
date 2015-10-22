## Introduction

TA++ is a C++ wrapper of TA-lib. I design this library for those who (mainly me) want to do quick and dirty technical analysis in C++. My goal is simplicity rather than exporting all the functionality of TA-lib. The typical use case is when you have a TA indicator in mind and want to screen stock data downloaded from Yahoo.

## Prerequisites

- Boost
- TA-lib
- Gnuplot (for rendering images.)

The library generates Gnuplot scripts as figures, though it doesn't depend on Gnuplot.

## Installation

There are only two files in the library: ta++.h and ta++-plot.h. Just drop these files in somewhere your C++ compiler is aware of and you are done.

## Compile and Link

TA++ depends on two libraries: TA-lib and boost date & time. If you use g++ and the latest version of TA-lib, you need to add "-lboost_date_time -lta_lib" to your g++ commandline.

To build the example program, type "make" in the project directory.  After running the example program, a file named "c.gp" will be generated.  Run "gnuplot c.gp" to produce the image file "c.png".

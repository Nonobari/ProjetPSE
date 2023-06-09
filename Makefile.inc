# Makefile.inc: definitions communes

PSE_DIR = ..
PSE_BIN = ${PSE_DIR}/bin
PSE_LIB = ${PSE_DIR}/lib
PSE_INCL_DIR = ${PSE_DIR}/include

CFLAGS = -g -I. -I${PSE_INCL_DIR}

LDLIBS = -L${PSE_LIB} -lpse -lm -pthread  -W -Wall -ansi -pedantic 

CC = gcc

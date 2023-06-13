include Makefile.inc
# Makefile.inc: definitions communes

EXE = serveur client

all: ${EXE}

${EXE): ${PSE_LIB}

clean:
	rm -f *.o *~ ${EXE} journal.log
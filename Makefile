# TP6 : Fichier Makefile
#
include PSE/Makefile.inc

EXE = serveur client

${EXE): ${PSE}

all: ${EXE}

clean:
	rm -f *.o *~ ${EXE} journal.log



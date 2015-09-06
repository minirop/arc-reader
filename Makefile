FILES=main.c arc.c dsc.c cbg.c bse.c decrypt.c write.c
OPTS=-Wall -ansi -pedantic -Wno-unused-result -O2
LIBS=-lpng12

all:
	gcc -o ethornell $(OPTS) $(FILES) $(LIBS)

debug:
	gcc -g -o ethornelld $(OPTS) $(FILES) $(LIBS)

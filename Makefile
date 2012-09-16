all:
	gcc -o ethornell -Wall -ansi -pedantic -Wno-unused-result -O2 main.c arc.c dsc.c cbg.c decrypt.c write.c -lpng12


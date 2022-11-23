CFLAGS += -g
APP = main

${APP}: translate.o main.o commands.o

translate.o: translate.c

.PHONY: clean
clean:
	rm -f *.o
	rm -f ${APP}
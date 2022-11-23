CFLAGS += -g
APP = main

${APP}: translate.o main.o util.o

translate.o: translate.c

.PHONY: clean
clean:
	rm *.o
	rm ${APP}
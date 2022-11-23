CFLAGS += -g
APP = browser

SRCS = engine.c main.c commands.c external.c
OBJS=$(SRCS:.c=.o )

${APP}: ${OBJS}

.PHONY: clean
clean:
	rm -f *.o
	rm -f ${APP}
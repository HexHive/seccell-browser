CFLAGS += -g
APP = browser

SRCS = engine.c main.c commands.c external.c
OBJS=$(SRCS:.c=.o )

${APP}: ${OBJS}
	$(CC) -o ${APP} $(LDFLAGS) ${OBJS} $(LOADLIBES) $(LDLIBS)

.PHONY: clean
clean:
	rm -f *.o
	rm -f ${APP}
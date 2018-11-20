CC = gcc
CFLAGS = -D_REENTRANT
LDFLAGS = -lpthread -pthread

web_server: server.c
	${CC} -o web_server server.c util.o ${LDFLAGS}

clean:
	rm web_server

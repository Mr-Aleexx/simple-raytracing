CC = gcc
CFLAGS = -Wall -Wextra 
DEBUG_CFLAGS = -DDEBUG -g -O0 -fsanitize=address

LFLAGS = -lSDL2 -lm

all: run

run: clock
	./raytracing

clock: 
	$(CC) -o raytracing raytracing.c $(CFLAGS) $(LFLAGS)

debug:
	$(CC) -o raytracing raytracing.c $(CFLAGS) $(LFLAGS) $(DEBUG_FLAGS)

clean: 
	rm -rf raytracing


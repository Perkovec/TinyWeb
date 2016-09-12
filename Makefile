ifeq ($(OS),Windows_NT)
	CLEAN = del /S *.o & del main
else
	CLEAN = find . -name "*.o" -type f -delete && rm main
endif

all: main.o mimeTypes.o logger.o
	gcc -o main main.o mimeTypes.o logger.o

main.o: main.c
	gcc -c -o main.o main.c

mimeTypes.o: mimeTypes.c
	gcc -c -o mimeTypes.o mimeTypes.c

logger.o: logger.c
	gcc -c -o logger.o logger.c

clean:
	$(CLEAN)
CC=gcc
CFLAGS=-I.
DEPS = 
OBJ = BMP180.o tvdemo.o
EXTRA_LIBS=-lwiringPi -lwiringPiDev -lm

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tvdemo: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(EXTRA_LIBS)

.PHONY: clean

clean:
	rm -f tvdemo $(OBJ) 

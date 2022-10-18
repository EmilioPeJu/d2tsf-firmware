TARGETS = test_gps
all: $(TARGETS)
CFLAGS = -g -I. -I../Src -DTEST

test: test_gps
	./test_gps

test_gps: test_gps.c ../Src/gps.c

clean:
	rm $(TARGETS)

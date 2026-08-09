CPPFLAGS += -D_FILE_OFFSET_BITS=64 -D_POSIX_C_SOURCE=200809L
CFLAGS ?= -std=c11 -Wall
all: hfdisk

test: tests/test_wire tests/test_large_file
	./tests/test_wire
	./tests/test_large_file

hfdisk: hfdisk.o dump.o partition_map.o convert.o io.o errors.o bitfield.o

tests/test_wire: tests/test_wire.c convert.o bitfield.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. $^ -o $@

tests/test_large_file: tests/test_large_file.c io.o errors.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. $^ -o $@

clean:
	rm -f *.o hfdisk tests/test_wire tests/test_large_file

convert.o: convert.c partition_map.h convert.h
dump.o: dump.c io.h errors.h partition_map.h convert.h
errors.o: errors.c errors.h
io.o: io.c hfdisk.h io.h errors.h
partition_map.o: partition_map.c partition_map.h hfdisk.h convert.h io.h errors.h
hfdisk.o: hfdisk.c hfdisk.h io.h errors.h partition_map.h version.h

partition_map.h: dpme.h
dpme.h: bitfield.h

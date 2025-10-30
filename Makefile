CC = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra
TEST_TARGET = test_app
BENCHMARK_TARGET = benchmark
OBJS = mew.o mew2.o

.PHONY: all test bench clean

all: test bench

mew.o: mew.c mew.h
	$(CC) $(CFLAGS) -c mew.c -o mew.o

mew2.o: mew2.c mew.h
	$(CC) $(CFLAGS) -c mew2.c -o mew2.o

test_app: $(OBJS) test.o
	$(CC) $(OBJS) test.o -o $(TEST_TARGET)

benchmark: $(OBJS) nyashka.o
	$(CC) $(OBJS) nyashka.o -o $(BENCHMARK_TARGET)

test.o: test.c mew.h
	$(CC) $(CFLAGS) -c test.c -o test.o

nyashka.o: nyashka.c mew.h
	$(CC) $(CFLAGS) -c nyashka.c -o nyashka.o

test: test_app
	@echo "unit tests go"
	@./$(TEST_TARGET)

bench: benchmark
	@echo "benchmarks go"
	@./$(BENCHMARK_TARGET)

clean:
	rm -f *.o $(TEST_TARGET) $(BENCHMARK_TARGET)

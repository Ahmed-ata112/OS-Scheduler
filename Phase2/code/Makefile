CFLAGS ?= -g -Wall

build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c hashmap.c queue.c -lm priority_queue.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

debug:
	gcc $(CFLAGS) process_generator.c -o process_generator.out
	gcc $(CFLAGS) clk.c -o clk.out
	gcc $(CFLAGS) scheduler.c hashmap.c queue.c -lm priority_queue.c -o scheduler.out
	gcc $(CFLAGS) process.c -o process.out
	gcc $(CFLAGS) test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt


run:
	./process_generator.out 2 1
all: run

generate:
	gcc test_generator.c -o test_generator.out
	./test_generator.out

test: build
	./process_generator.out 2 1


buddy:
	gcc -o hey -g  buddy_core.c -lm
	./hey


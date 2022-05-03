build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out

	gcc scheduler.c  hashmap.c -lm priority_queue.c -o scheduler.out

	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt


run:
	./process_generator.out

generate:
	gcc test_generator.c -o test_generator.out
	./test_generator.out

test: build
	./process_generator.out 2 1


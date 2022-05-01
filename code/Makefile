build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c  hashmap.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out


test: build
	./process_generator.out


buddy:
	gcc -o hey -g  buddy_core.c -lm
	./hey
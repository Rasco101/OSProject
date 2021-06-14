build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out

test_generator:
	gcc test_generator.c -o test_generator.out

test_generator_run:
	./test_generator.out

clock:
	gcc clk.c -o clk.out

scheduler:
	gcc scheduler.c -o scheduler.out -lpthread

process:
	gcc process.c -o process.out -lpthread

process_generator:
	gcc process_generator.c -o process_generator.out
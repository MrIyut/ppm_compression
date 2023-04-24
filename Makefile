all: run

build:
	gcc header.h image.c main.c qtee.c queue.c -o quadtree

clean:
	rm quadtree
	rm *.out

run:
	make build	
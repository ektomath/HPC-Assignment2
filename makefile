all: cell_distances

cell_distances: cell_distances.c
	gcc -o cell_distances cell_distances.c -lm -O2 -march=native -fopenmp

clean:
	rm cell_distances
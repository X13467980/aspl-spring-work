CC = gcc
CFLAGS = -Wall -O2 -std=c99

tsp_gen: tsp_gen.c
	$(CC) $(CFLAGS) -o tsp_gen tsp_gen.c -lm

white_noise: white_noise.c
	$(CC) $(CFLAGS) -o white_noise white_noise.c

tsp_to_ir: tsp_to_ir.c
	$(CC) $(CFLAGS) -o tsp_to_ir tsp_to_ir.c -lm

clean:
	rm -f tsp_gen white_noise tsp_to_ir

.PHONY: clean

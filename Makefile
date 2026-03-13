CC = gcc
CFLAGS = -Wall -O2 -std=c99

tsp_gen: tsp_gen.c
	$(CC) $(CFLAGS) -o tsp_gen tsp_gen.c -lm

white_noise: white_noise.c
	$(CC) $(CFLAGS) -o white_noise white_noise.c

tsp_to_ir: tsp_to_ir.c
	$(CC) $(CFLAGS) -o tsp_to_ir tsp_to_ir.c -lm

# tsp_1～10 の全録音に対してインパルス応答を算出
tsp_to_ir_all: tsp_to_ir
	@for i in 1 2 3 4 5 6 7 8 9 10; do \
		echo "Processing tsp_$$i..."; \
		./tsp_to_ir recordings/tsp_signal.wav recordings/tsp_$$i.wav impulse_response_tsp_$$i.wav; \
	done
	@echo "Done: impulse_response_tsp_1.wav ～ impulse_response_tsp_10.wav"

clean:
	rm -f tsp_gen white_noise tsp_to_ir

.PHONY: clean tsp_to_ir_all

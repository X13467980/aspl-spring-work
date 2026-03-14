CC = gcc
CFLAGS = -Wall -O2 -std=c99

tsp_gen: tsp_gen.c
	$(CC) $(CFLAGS) -o tsp_gen tsp_gen.c -lm

white_noise: white_noise.c
	$(CC) $(CFLAGS) -o white_noise white_noise.c

tsp_to_ir: tsp_to_ir.c
	$(CC) $(CFLAGS) -o tsp_to_ir tsp_to_ir.c -lm

adaptive_filter: adaptive_filter.c
	$(CC) $(CFLAGS) -o adaptive_filter adaptive_filter.c -lm

ir_analyze: ir_analyze.c
	$(CC) $(CFLAGS) -o ir_analyze ir_analyze.c -lm

ir_to_inverse: ir_to_inverse.c
	$(CC) $(CFLAGS) -o ir_to_inverse ir_to_inverse.c -lm

ir_to_txt: ir_to_txt.c
	$(CC) $(CFLAGS) -o ir_to_txt ir_to_txt.c

# 残響曲線を gnuplot でプロット（decay_curve.txt が必要。ir_analyze の第2引数で出力）
plot_decay:
	gnuplot plot_decay_curve.gp
	@echo "Done: decay_curve.png"

# インパルス応答を gnuplot でプロット
plot_ir: ir_to_txt
	./ir_to_txt impulse_response_tsp.wav ir_data.txt
	gnuplot plot_ir.gp
	@echo "Done: impulse_response.png"

plot_ir_white: ir_to_txt
	./ir_to_txt impulse_response_white.wav ir_data.txt
	gnuplot -e "outfile='impulse_response_white.png'" plot_ir.gp
	@echo "Done: impulse_response_white.png"

# tsp_1～10 を時間領域で平均してからインパルス応答を算出（友達のやり方）
tsp_to_ir_all: tsp_to_ir
	./tsp_to_ir recordings/tsp_signal.wav impulse_response_tsp.wav \
		recordings/tsp_1.wav recordings/tsp_2.wav recordings/tsp_3.wav recordings/tsp_4.wav recordings/tsp_5.wav \
		recordings/tsp_6.wav recordings/tsp_7.wav recordings/tsp_8.wav recordings/tsp_9.wav recordings/tsp_10.wav
	@echo "Done: impulse_response_tsp.wav (10回平均)"

clean:
	rm -f tsp_gen white_noise tsp_to_ir adaptive_filter ir_analyze ir_to_inverse ir_to_txt

.PHONY: clean tsp_to_ir_all plot_decay plot_ir plot_ir_white

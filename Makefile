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

ir_add_noise: ir_add_noise.c
	$(CC) $(CFLAGS) -o ir_add_noise ir_add_noise.c -lm

# 残響曲線を gnuplot でプロット（decay_curve.txt が必要。ir_analyze の第2引数で出力）
plot_decay:
	gnuplot plot_decay_curve.gp
	@echo "Done: decay_curve.png"

# 白色信号の残響曲線
plot_decay_white:
	./ir_analyze impulse_response_white.wav decay_curve_white.txt
	gnuplot -e "datafile='decay_curve_white.txt'; outfile='decay_curve_white.png'" plot_decay_curve.gp
	@echo "Done: decay_curve_white.png"

# 白色信号 SN20 の残響曲線
plot_decay_white_sn20: ir_add_noise
	./ir_add_noise impulse_response_white.wav ir_white_sn20.wav 20
	./ir_analyze ir_white_sn20.wav decay_curve_white_sn20.txt
	gnuplot -e "datafile='decay_curve_white_sn20.txt'; outfile='decay_curve_white_sn20.png'" plot_decay_curve.gp
	@echo "Done: decay_curve_white_sn20.png"

# 白色信号 SN50 の残響曲線
plot_decay_white_sn50: ir_add_noise
	./ir_add_noise impulse_response_white.wav ir_white_sn50.wav 50
	./ir_analyze ir_white_sn50.wav decay_curve_white_sn50.txt
	gnuplot -e "datafile='decay_curve_white_sn50.txt'; outfile='decay_curve_white_sn50.png'" plot_decay_curve.gp
	@echo "Done: decay_curve_white_sn50.png"

# 白色信号の残響曲線（T10/T20/RT60 付き）
plot_decay_white_t10t20:
	./ir_analyze impulse_response_white.wav decay_curve_white.txt
	gnuplot -e "datafile='decay_curve_white.txt'; outfile='decay_curve_white_t10t20.png'; t10=0.012; t20=0.025; rt60_t10=0.072; rt60_t20=0.075" plot_decay_curve_t10t20.gp
	@echo "Done: decay_curve_white_t10t20.png"

# 残響曲線（T10/T20/RT60 の基準線・数値付き）
plot_decay_t10t20:
	gnuplot plot_decay_curve_t10t20.gp
	@echo "Done: decay_curve_t10t20.png"

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
	rm -f tsp_gen white_noise tsp_to_ir adaptive_filter ir_analyze ir_to_inverse ir_to_txt ir_add_noise

.PHONY: clean tsp_to_ir_all plot_decay plot_decay_white plot_decay_white_sn20 plot_decay_white_sn50 plot_decay_white_t10t20 plot_decay_t10t20 plot_ir plot_ir_white

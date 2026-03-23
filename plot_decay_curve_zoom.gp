#!/usr/bin/env gnuplot
# 残響曲線の減衰部分を拡大表示（T10/T20 フィット線付き）
# 使い方: ./ir_analyze impulse_response.wav decay_curve.txt
#        gnuplot plot_decay_curve_zoom.gp
# または: gnuplot -e "datafile='decay_curve.txt'; outfile='decay_curve_zoom.png'; xmin=4.1; xmax=4.3" plot_decay_curve_zoom.gp

if (!exists("datafile")) datafile = "decay_curve.txt"
if (!exists("outfile")) outfile = "decay_curve_zoom.png"

# 拡大する時間範囲（減衰が起きる部分）
if (!exists("xmin")) xmin = 4.15
if (!exists("xmax")) xmax = 4.30

# フィット線パラメータ（decay_curve_fit.gp から自動ロード）
# datafile が "decay_curve.txt" の場合、fitfile = "decay_curve_fit.gp"
# 事前に ./ir_analyze impulse_response.wav decay_curve.txt を実行すること
fitfile = (strstrt(datafile, ".txt") > 0) ? substr(datafile, 1, strstrt(datafile, ".txt") - 1) . "_fit.gp" : "decay_curve_fit.gp"
load fitfile

set terminal pngcairo size 800, 500 font "Helvetica,12"
set output outfile

set xlabel "Time [s]"
set ylabel "Energy [dB]"
set grid
set key top right

set xrange [xmin:xmax]
set yrange [-50:5]

# 残響曲線＋T10/T20 フィット線（参考画像に倣い、凡例付き）
plot datafile using 1:2 with lines lw 2 lc rgb "#e41a1c" title "Schroeder decay curve", \
     (slope_t10*x + intercept_t10) with lines dt 2 lw 1.5 lc rgb "#377eb8" title "T_{10} fit", \
     (slope_t20*x + intercept_t20) with lines dt 2 lw 1.5 lc rgb "#4daf4a" title "T_{20} fit"

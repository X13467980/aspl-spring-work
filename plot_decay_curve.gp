#!/usr/bin/env gnuplot
# 残響曲線をプロット
# 使い方: gnuplot plot_decay_curve.gp
# または: gnuplot -e "datafile='decay_curve.txt'; outfile='decay_curve.png'" plot_decay_curve.gp

# デフォルトの入力・出力（コマンドラインで上書き可能）
if (!exists("datafile")) datafile = "decay_curve.txt"
if (!exists("outfile")) outfile = "decay_curve.png"

set terminal pngcairo size 800, 500 font "Helvetica,12"
set output outfile

set xlabel "Time [s]"
set ylabel "Energy [dB]"
set grid
set key off

set xrange [*:*]
set yrange [-50:5]

plot datafile using 1:2 with lines lw 2 lc rgb "#e41a1c"

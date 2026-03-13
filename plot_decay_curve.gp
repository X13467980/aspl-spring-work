#!/usr/bin/env gnuplot
# 残響曲線をプロット
# 使い方: gnuplot plot_decay_curve.gp
# または: gnuplot -e "datafile='decay_curve.txt'; outfile='decay_curve.png'" plot_decay_curve.gp

# デフォルトの入力・出力（コマンドラインで上書き可能）
if (!exists("datafile")) datafile = "decay_curve.txt"
if (!exists("outfile")) outfile = "decay_curve.png"

set terminal pngcairo size 800, 500 font "Helvetica,12"
set output outfile

set xlabel "時間 (秒)"
set ylabel "エネルギー (dB)"
set title "残響曲線 (Schroeder積分)"
set grid
set key top right

set xrange [*:*]
set yrange [-40:5]

# T10/T20 の区間を示す水平線
set arrow from graph 0, first -5 to graph 1, first -5 nohead lt 2 lw 1
set arrow from graph 0, first -15 to graph 1, first -15 nohead lt 3 lw 1
set arrow from graph 0, first -25 to graph 1, first -25 nohead lt 4 lw 1

plot datafile using 1:2 with lines lw 2 title "残響曲線"

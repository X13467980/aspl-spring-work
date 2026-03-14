#!/usr/bin/env gnuplot
# 残響曲線を T10/T20/RT60 の基準線付きでプロット
# 使い方: gnuplot plot_decay_curve_t10t20.gp

if (!exists("datafile")) datafile = "decay_curve.txt"
if (!exists("outfile")) outfile = "decay_curve_t10t20.png"

# T10, T20, RT60 の値（オプション、ラベル用）
if (!exists("t10")) t10 = 0.012
if (!exists("t20")) t20 = 0.024
if (!exists("rt60_t10")) rt60_t10 = 0.070
if (!exists("rt60_t20")) rt60_t20 = 0.071

set terminal pngcairo size 800, 500 font "Helvetica,12"
set output outfile

set xlabel "Time [s]"
set ylabel "Energy [dB]"
set grid
set key off

set xrange [*:*]
set yrange [-50:5]

# T10/T20 の区間を示す水平線 (-5, -15, -25 dB)
set arrow from graph 0, first -5 to graph 1, first -5 nohead lt 2 lw 1 lc rgb "#377eb8"
set arrow from graph 0, first -15 to graph 1, first -15 nohead lt 3 lw 1 lc rgb "#4daf4a"
set arrow from graph 0, first -25 to graph 1, first -25 nohead lt 4 lw 1 lc rgb "#984ea3"

# T10, T20, RT60 の値を表示
set label sprintf("T_{10} = %.3f s", t10) at graph 0.02, 0.95 left
set label sprintf("T_{20} = %.3f s", t20) at graph 0.02, 0.88 left
set label sprintf("RT_{60} (from T_{10}) = %.3f s", rt60_t10) at graph 0.02, 0.81 left
set label sprintf("RT_{60} (from T_{20}) = %.3f s", rt60_t20) at graph 0.02, 0.74 left

plot datafile using 1:2 with lines lw 2 lc rgb "#e41a1c"

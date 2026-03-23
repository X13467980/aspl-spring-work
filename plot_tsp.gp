#!/usr/bin/env gnuplot
# TSP信号の波形を描画

set terminal pngcairo size 1000, 400 font "Helvetica,12"
set output "tsp_waveform.png"

set xlabel "Time [s]"
set ylabel "Amplitude"
set title ""
set grid
set key off

# ラベル・目盛り用にマージンを十分確保
set lmargin 8
set bmargin 4
set tmargin 2
set rmargin 2

# 軸範囲（参考画像に倣う）
set xrange [0:5]
set yrange [-1:1]

plot "tsp_waveform.txt" using 1:2 with lines lc rgb "red" lw 0.5

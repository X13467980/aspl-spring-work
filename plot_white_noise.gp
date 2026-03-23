#!/usr/bin/env gnuplot
# 白色信号（180秒）の波形を描画

set terminal pngcairo size 1000, 400 font "Helvetica,12"
set output "white_noise_waveform.png"

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

# 軸範囲（180秒の先頭1秒を表示）
set xrange [0:1]
set yrange [-1:1]

plot "white_noise_waveform.txt" using 1:2 with lines lc rgb "red" lw 0.3

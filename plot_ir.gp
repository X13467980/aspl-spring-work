#!/usr/bin/env gnuplot
# インパルス応答をプロット
# 使い方: ./ir_to_txt impulse_response.wav ir_data.txt
#        gnuplot plot_ir.gp
# または: gnuplot -e "datafile='ir_data.txt'; outfile='impulse_response.png'" plot_ir.gp

if (!exists("datafile")) datafile = "ir_data.txt"
if (!exists("outfile")) outfile = "impulse_response.png"

set terminal pngcairo size 800, 400 font "Helvetica,12"
set output outfile

set xlabel "時間 (秒)"
set ylabel "振幅"
set title "インパルス応答"
set grid
set key off

plot datafile using 1:2 with lines lw 1 lc rgb "#e41a1c"

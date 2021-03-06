#!/usr/bin/gnuplot -c

# Output format .eps
set terminal postscript eps enhanced size 3in,3in

# Key (legend) of data
set key outside above  # Set key (legend) places at the outside of the plot

# x: Time
set xlabel "Time"
set format x "%.1t s"
set autoscale x

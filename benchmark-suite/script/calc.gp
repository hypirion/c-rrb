set terminal pdf mono font 'Bitstream Charter'
set termoption dash

set output 'out.pdf'
array='array.dat'
dummy='dummy.dat'
rrb='rrb.dat'

set title 'Box-and-whisker plot of array calculation steps'
#set key inside left top box linetype -1 linewidth 1.000
set key inside right bottom box linetype -1 linewidth 1.000 width 6
set bars 2.0

set boxwidth 0.15

set xrange [0.5:3.5]

# set xlabel 'xlabel'
set ylabel 'Nanoseconds'
set format y '%.0f'
S = 1

plot dummy u (((int($0))%2==0) ? ($0)/2 + 0.80 : 1/0):2:1:5:4\
     with candlesticks whiskerbars lt 1 lc rgb 'blue' t "Dummy",\
     '' u (((int($0))%2==0) ? ($0)/2 + 0.80 : 1/0):3:3:3:3\
     with candlesticks lt -1 notitle,\
     array u (((int($0))%2==0) ? ($0)/2 + 1 : 1/0):2:1:5:4:(0.15):xticlabels(6)\
     with candlesticks whiskerbars lt 1 lc rgb 'web-green' t "Array",\
     '' u (((int($0))%2==0) ? ($0)/2 + 1 : 1/0):3:3:3:3\
     with candlesticks lt -1 notitle,\
     rrb u (((int($0))%2==0) ? ($0)/2 + 1.20 : 1/0):2:1:5:4\
     with candlesticks whiskerbars lt 1 lc rgb 'dark-red' t "RRB",\
     '' u (((int($0))%2==0) ? ($0)/2 + 1.20 : 1/0):3:3:3:3\
     with candlesticks lt -1 notitle,\
     

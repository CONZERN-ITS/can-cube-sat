set datafile separator comma

set xlabel "Время от старта (минуты)" font ",16"
set xtics 10 nomirror font ",16"
set mxtics

set ylabel "Концентрация кислорода (%)" font ",16"
set ytics 5 nomirror font ",16"
set mytics


set yzeroaxis
set xzeroaxis

set border 1+2
set grid xtics ytics 


set key off

set terminal png size 1920,1080
set output 'PLD_ME2O2_DATA-13-0-o2.png'

plot 'PLD_ME2O2_DATA-13-0.csv' using 'time_from_start_min':'o2_conc'

pause mouse close

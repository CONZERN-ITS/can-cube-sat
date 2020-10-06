set datafile separator comma

set xlabel "Время от старта (минуты)" font ",16"
set xtics 10 nomirror font ",16"
set mxtics

set ylabel "Концентрация газовых примесей (ppm)" font ",16"
set ytics 50 nomirror font ",16"
set mytics
set yrange [0:30]

set yzeroaxis
set xzeroaxis

set border 1+2
set grid xtics ytics

set key samplen 10 right top font ",16"

set terminal png size 1920,1080
set output 'PLD_MICS_6814_DATA-13-0-no2.png'


plot 'PLD_MICS_6814_DATA-13-0.csv' using 'time_from_start_min':'nh3_conc' title "Концентрация NH3 (ppm)"


pause mouse close

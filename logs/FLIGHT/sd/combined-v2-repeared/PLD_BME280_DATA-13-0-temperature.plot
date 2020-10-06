set datafile separator comma

set xlabel "Время от старта (минуты)" font ",16"
set xtics 10 nomirror font ",16"
set mxtics

set ylabel "Температура по BME280 (°C)" font ",16"
set ytics nomirror font ",16"
set mytics

set yzeroaxis
set xzeroaxis

set border 1+2
set grid xtics ytics


set key off

set terminal png size 1920,1080
set output 'PLD_BME280_DATA-13-0-temperature.png'

plot 'PLD_BME280_DATA-13-0.csv' using 'time_from_start_min':'temperature' axis x1y1 

pause mouse close

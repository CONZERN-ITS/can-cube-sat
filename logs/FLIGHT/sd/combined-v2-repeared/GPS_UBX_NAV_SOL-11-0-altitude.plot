set datafile separator comma

set xlabel "Время от старта (минуты)" font ",16"
set xtics 10 nomirror font ",16"
set mxtics

set ylabel "Высота от опорного эллипсоида WGS84 по GPS (м)" font ",16"
set ytics 1000 nomirror font ",16"
set mytics

set yzeroaxis
set xzeroaxis

set border 1+2
set grid xtics ytics 

set key off

set terminal png size 1920,1080
set output 'GPS_UBX_NAV_SOL-11-0-altitude.png'

plot 'GPS_UBX_NAV_SOL-11-0.csv' using 'minutes':'h'
pause mouse close

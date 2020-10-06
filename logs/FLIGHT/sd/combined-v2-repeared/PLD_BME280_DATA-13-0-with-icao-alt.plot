set datafile separator comma

set xlabel "Время от старта (минуты)" font ",16"
set xtics 10 nomirror font ",16"
set mxtics

set ylabel "высота (м)" font ",16"
set ytics nomirror font ",16"
set mytics


set yzeroaxis
set xzeroaxis

set border 1+2+8
set grid xtics ytics

set key samplen 10 right bottom font ",16"

set terminal png size 1920,1080
set output 'PLD_BME280_DATA-13-0-with-icao-alt.png'

plot 'PLD_BME280_DATA-13-0-with-icao-alt.csv' using 'time_from_start_min':'icao_alt' title "Барометрическая высота",\
	'GPS_UBX_NAV_SOL-11-0.csv' using 'minutes':'h' title "Высота по GPS"

pause mouse close

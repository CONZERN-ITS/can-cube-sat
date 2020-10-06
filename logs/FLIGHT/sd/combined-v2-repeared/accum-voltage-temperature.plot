set datafile separator comma

set xlabel "Время от старта (минуты)" font ",16"
set xtics 10 nomirror font ",16"
set mxtics

set ylabel "Температура аккумуляторов (°C)" font ",16"
set ytics 5 nomirror font ",16"
set yrange [-40:40]
set mytics

set y2label "Напряжение аккумуляторов (V)" font ",16"
set y2tics 0.5 nomirror font ",16"
set y2range [3:9]
set my2tics

set yzeroaxis
set xzeroaxis

set border 1+2+8
set grid xtics ytics 

set key samplen 10 right bottom font ",16"

set terminal png size 1920,1080
set output 'accum-voltage-temperature.png'

plot 'accum-temperature.csv' using 'time_from_start_min':'temperature' axis x1y1 title "Температура аккамуляторов",\
	'Electrical_state.csv' using 'minutes':'voltage' axis x1y2 title "Напряжение на аккумуляторах"

pause mouse close

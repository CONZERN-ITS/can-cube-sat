set datafile separator comma

set xlabel "Высота от опорного эллипсоида WGS84 (м)" font ",16"
set xtics 1000 nomirror font ",16"
set mxtics

set ylabel "Концентрация O2 (%)" font ",16"
set ytics nomirror font ",16"
set mytics

set yzeroaxis
set xzeroaxis

set border 1+2
set grid xtics ytics

set key off

set terminal png size 1920,1080
set output 'GPS_UBX_NAV_SOL-11-0-with-o2.png'

plot 'GPS_UBX_NAV_SOL-11-0-with-o2.csv' using 'h':'o2_conc',\
	'GPS_UBX_NAV_SOL-11-0-with-o2.csv' using 'h':'o2_conc':(sprintf('%0.0f', column('minutes'))) every 100 with labels offset 0,3 font ",16"
pause mouse close

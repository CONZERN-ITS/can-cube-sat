set datafile separator comma

set xlabel "Высота от опорного эллипсоида WGS84 (м)" font ",16"
set xtics 1000 nomirror font ",16"
set mxtics

set ylabel "Концентрация NH3 (ppm)" font ",16"
set ytics nomirror font ",16"
set mytics

set yzeroaxis
set xzeroaxis

set border 1+2
set grid xtics ytics 

set key off

set terminal png size 1920,1080
set output 'GPS_UBX_NAV_SOL-11-0-with-mics-nh3.png'

plot 'GPS_UBX_NAV_SOL-11-0-with-o2-and-bme-and-mics.csv' using 'h':'mics_nh3',\
	'GPS_UBX_NAV_SOL-11-0-with-o2-and-bme-and-mics.csv' using 'h':'mics_nh3':(sprintf('%0.0f', column('minutes'))) every 100 with labels offset 0,1.5 font ",16"
pause mouse close

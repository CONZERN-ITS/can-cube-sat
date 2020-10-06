set datafile separator comma

set xlabel "Высота от опорного эллипсоида WGS84 (м)" font ",16"
set xtics 2000 nomirror font ",16"
set mxtics

set ylabel "Давление BME280 (Па)" font ",16"
set ytics 10000 nomirror font ",16"
set yrange [25000:130000]
set mytics

set yzeroaxis
set xzeroaxis

set border 1+2+8
set grid xtics ytics 

set key samplen 10 right top font ",16"

set terminal png size 1920,1080
set output 'GPS_UBX_NAV_SOL-11-0-with-bme-pressure.png'

plot 'icao.csv' using 'alt':'pressure' every 100 with lines lw 3 title "Давление по  ICAO standard atmosphere 1993", \
	'GPS_UBX_NAV_SOL-11-0-with-o2-and-bme.csv' using 'h':'bme_pressure' title "Давление по BME280", \
	'GPS_UBX_NAV_SOL-11-0-with-o2-and-bme.csv' using 'h':'bme_pressure':(sprintf('%0.0f', column('minutes'))) every 100 with labels offset 0,1.5 font ",20"


pause mouse close

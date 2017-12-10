#draw fy3b ndvi 
#2017-11-23 wf
reset
#heatmap
set terminal png noenhanced size 2000,1390 font "/QHZX_DATA/extras/libsans-reg.ttf,16"
set output '{{{OUTFILE}}}'
set encoding utf8
set multiplot

#plot1
set origin 0.03,0.12
set size  0.95,0.85
unset key 
unset xtics 
unset ytics 
unset x2tics

set format x "%.0f"
set format y "%.0f"

set xrange[-3000000:3000000]
set yrange[1800000:6000000]
set cbrange[-0.2:0.2]

set cbtics ( -0.2 , -0.15 , -0.1 , -0.05 , 0.0 , 0.05 , 0.1 , 0.15 , 0.2 )


#set pal maxcolors 14
set palette defined (  \
0 '#2B83BA',   \
1 '#5EA7B1' , \
2 '#91CBA9',   \
3 '#BCE4AA',   \
4 '#FFFFBF',   \
5 '#FFFFBF',   \
6 '#FFFFBF',   \
7 '#FEBE74',   \
8 '#F69053',   \
9 '#E75437',   \
10 '#D7191C'    \
)

set label 1 "NOAA/AVHRR NDVI {{{TITLE}}} 0.05\260 Daily Anomaly" at screen 0.5, screen 0.1 font '/QHZX_DATA/extras/libsans-reg.ttf,32' center 

set label 2 "Produced:{{{PTIME}}}" at screen 0.05, screen 0.05 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left 


set object 1 rect from graph 0.86,0.0 to graph 1.0,0.3
set object 1 rect fc rgb 'white' fillstyle solid  0.0 noborder front
plot \
'{{{INFILE}}}' using 1:2:3 with image , \
'/QHZX_DATA/extras/world_110m-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/river-albers-final.txt' u 1:2 with lines lt 1 lc '#0E35A6',\
'/QHZX_DATA/extras/grid-albers-final.txt' u 1:2 with lines lt 1 lc '#cccccc'



#nan hai
unset key 
unset xtics 
unset ytics 
set size 0.14,0.28
set origin 0.73,0.122
set xrange[40000:1800000]
set yrange[100000:2500000]
unset colorbox
unset object 1
plot \
'{{{INFILE}}}' using 1:2:3 with image , \
'/QHZX_DATA/extras/world_110m-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/river-albers-final.txt' u 1:2 with lines lt 1 lc '#0E35A6' ,\
'/QHZX_DATA/extras/grid-albers-final.txt' u 1:2 with lines lt 1 lc '#cccccc'


#logos
set origin 0.75,-0.00
set size square 0.24,0.24
set xrange[0:128]
set yrange[0:128]
unset key 
unset xtics
unset x2tics
unset ytics
unset border
unset cbrange
unset pm3d
plot '/QHZX_DATA/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage , '/QHZX_DATA/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0.5)  with rgbimage 
unset multiplot
unset output
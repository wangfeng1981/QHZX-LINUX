#draw ndvi 
#2017-10-13 wf
reset
#heatmap
set terminal png noenhanced size 2000,1390 font '/QHZX_DATA/extras/libsans-reg.ttf,24'
set output "{{{OUTFILE}}}"
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

set xrange[-1800000:3500000]
set yrange[1960000:6600000]
set cbrange[0:10]

set cbtics ( \
'Filled' 0.5,\
'Bad Value' 1.5,\
'Night' 2.5,\
'Saturation' 3.5,\
'Uncertain' 4.5,\
'Cloud' 5.5,\
'Water' 6.5,\
'Land' 7.5,\
'Ice' 8.5,\
'Snow' 9.5 )

set cbtics scale 0

set palette defined (  \
0 '#666666', 1 '#666666',  \
1 '#777777', 2 '#777777',  \
2 '#444444', 3 '#444444',  \
3 '#eaeafa', 4 '#eaeafa',  \
4 '#990099', 5 '#990099',  \
5 '#eaeaea', 6 '#eaeaea',  \
6 '#2B579A', 7 '#2B579A',  \
7 '#9a9a9a', 8 '#9a9a9a',  \
8 '#aaaaff', 9 '#aaaaff',  \
9 '#ff0000', 10 '#ff0000' \
)

set label 1 "{{{PNAME}}} {{{YMD}}} Daily China" at screen 0.05, screen 0.1 font '/QHZX_DATA/extras/libsans-reg.ttf,28' left 


set object 1 rect from graph 0.86,0.0 to graph 1.0,0.3
set object 1 rect fc rgb 'white' fillstyle solid  0.0 noborder front
plot \
'{{{INFILE}}}' using 1:2:3 with image , \
'/QHZX_DATA/extras/world_50m-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china2-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/cn-river-asia-albers.txt' u 1:2 with lines lt 1 lc '#0E35A6',\
'/QHZX_DATA/extras/grid-asia-albers.txt' u 1:2 with lines lt 1 lc '#cccccc'



#nan hai
unset key 
unset xtics 
unset ytics 
set size 0.14,0.28
set origin 0.73,0.122
set xrange[1100000:2700000]
set yrange[100000:2500000]
unset colorbox
unset object 1
plot \
'{{{INFILE}}}' using 1:2:3 with image , \
'/QHZX_DATA/extras/world_50m-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china2-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/cn-river-asia-albers.txt' u 1:2 with lines lt 1 lc '#0E35A6',\
'/QHZX_DATA/extras/grid-asia-albers.txt' u 1:2 with lines lt 1 lc '#cccccc'


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
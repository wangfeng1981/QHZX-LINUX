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

set xrange[-5250000:5250000]
set yrange[  683201:8459207]
#set yrange[1800000:6000000]
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

set label 1 "{{{PNAME}}} {{{YMD}}} Daily Euro-Asia" at screen 0.05, screen 0.1 font '/QHZX_DATA/extras/libsans-reg.ttf,28' left 

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
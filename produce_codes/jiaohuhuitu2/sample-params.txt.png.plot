#draw ndvi -0.1 0.9936 
#2017-12-05 wf
reset
#heatmap
set terminal png noenhanced size 833,1000 font '/QHZX_DATA/extras/libsans-reg.ttf'
set output "sample-params.txt.png"
set encoding utf8
set multiplot
#title lables
set label 1 "NOAA NDVI 0.05 degree MAX COMB." at screen 0.02, screen 0.96 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left  
set label 2 "Date from 2011-01-01 to 2011-02-01; Geo:[70,170,60,-60]." at screen 0.02, screen 0.93 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left
set label 3 "Produced:2017-12-05 10:09:57; CFC:3."  at screen 0.02, screen 0.90 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left

#plot1
set origin 0,0
set size  0.9,0.9
unset key 
set xtics 
set ytics 
set x2tics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set cbrange[-0.1:0.8]
set xrange[70:170]
set yrange[-60:60]
set x2range[70:170]

set palette defined (  \
0 '#713E3A',   \
1.11 '#83533C' , \
2.22 '#AC8327',   \
3.33 '#D1AF18',   \
4.44 '#EFE900',   \
5.55 '#84E200',   \
6.66 '#05B514',   \
7.77 '#00811C',   \
8.88 '#015020',   \
10 '#001F18'    \
)
set colorbox user origin 0.9,0.05 size 0.05,0.8
plot  "sample-params.txt.png.tmp" using 1:2:3 with image \
, '/QHZX_DATA/extras/world_110m.txt' u 1:2 with lines lt 1 lc 'grey' \
, '/QHZX_DATA/extras/china2.txt' u 1:2 with lines lt 1 lc '#ccbbdd' 



#logos
set origin 0.75,0.88
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

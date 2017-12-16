#draw sst 17.6223 29.4015 
#2017-12-07 wf
reset
#heatmap
set terminal png noenhanced size 1000,1000 font '/QHZX_DATA/extras/libsans-reg.ttf'
set output "sample-fy4sstparams.txt.png"
set encoding utf8
set multiplot
#title lables
set label 1 "FY4A SST 4Km AVER COMB." at screen 0.02, screen 0.96 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left  
set label 2 "Date from 2017-08-18 to 2017-08-21; Geo:[100,120,10,-10]." at screen 0.02, screen 0.93 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left
set label 3 "Produced:2017-12-07 12:45:52; CFC:4."  at screen 0.02, screen 0.90 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left

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
set xrange[100:120]
set yrange[-10:10]
set x2range[100:120]


set cbrange[11:35]
set cbtics ("Land" 11,\
"<14\260C" 14,\
"20\260C" 20 ,\
"26\260C" 26 ,\
"30.5\260C" 30.5,\
">33.5\260C" 35 \
)

set hidden3d front
set palette defined (\
 0.00 '#f1f1f1', 6.25 '#f1f1f1',\
 6.25 '#0854c2', 12.50 '#0854c2',\
12.50 '#1464d2', 18.75 '#1464d2',\
18.75 '#1e6eeb', 25.00 '#1e6eeb',\
25.00 '#2882f0' ,31.25 '#2882f0' ,\
31.25 '#3c96f5', 37.50 '#3c96f5',\
37.50 '#50a5f5', 43.75 '#50a5f5',\
43.75 '#78b9fa', 50.00 '#78b9fa',\
50.00 '#96d2fa' ,56.25 '#96d2fa',\
56.25 '#b4f0fa', 62.25 '#b4f0fa',\
62.50 '#e1ffff', 68.75 '#e1ffff',\
68.75 '#fffaaa', 75.00 '#fffaaa',\
75.00 '#ffc03c' ,81.25 '#ffc03c',\
81.25 '#ff6000', 87.50 '#ff6000',\
87.50 '#e11400', 93.75 '#e11400',\
93.75 '#a50000', 100.00 '#a50000'\
)



set colorbox user origin 0.9,0.05 size 0.05,0.8
plot   "sample-fy4sstparams.txt.png.tmp" using 1:2:( ($3<13)*13 + ($3>=13)*$3 ) with image \
,'/QHZX_DATA/extras/world_50m.txt' u 1:2 with filledcurves lt 1 lc 'grey' \
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

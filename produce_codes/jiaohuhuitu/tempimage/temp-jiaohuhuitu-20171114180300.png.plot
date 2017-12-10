#fy4海表温度交互绘图sst plot
reset
set terminal png noenhanced size 1000,1000 font '/root/ncc-fy4-project/extras/libsans-reg.ttf,16' 
set output '/root/ncc-fy4-project/produce_codes/jiaohuhuitu/tempimage/temp-jiaohuhuitu-20171114180300.png'
set multiplot
set encoding utf8

set bmargin 8

set xtics
set ytics
unset ztics
unset border
unset key 

set palette defined (  0.00 '#0854c2', 50.00 '#efef00' , 100.00 '#a50000' )

####################################
set xrange[105:120] 
set yrange[15:30] 
set cbrange[15:35]

plot  "/root/ncc-fy4-project/produce_codes/jiaohuhuitu/tempimage/temp-jiaohuhuitu-20171114180300.png.xyz.txt" using 1:2:3 with image , '/root/ncc-fy4-project/extras/world_50m.txt' u 1:2 with filledcurves lt 1 lc 'grey'   , '/root/ncc-fy4-project/extras/china2.txt' u 1:2  with lines lt 1 lc '#ccbbdd'  


#title lables
set label 1 '{{{TITLE1}}}' at screen 0.5,0.08 center font '/root/ncc-fy4-project/extras/libsans-reg.ttf,16'  
set label 2 "Produced 2017-11-15 20:34:57" at screen 0.5,0.01 center font '/root/ncc-fy4-project/extras/libsans-reg.ttf,13' 

#logos
unset xtics
unset ytics
unset cbrange
set xrange [0:128] 
set yrange [0:128] 
set margin 0,0,0,0
unset title
set origin 0,0
set size 0.2,0.2
plot '/root/ncc-fy4-project/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage  ,'/root/ncc-fy4-project/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0)  with rgbimage 


unset multiplot
unset output

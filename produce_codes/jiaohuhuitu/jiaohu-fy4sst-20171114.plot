#fy4海表温度交互绘图sst plot
reset
set terminal png noenhanced size 1000,1000 font '/root/ncc-fy4-project/extras/libsans-reg.ttf,16' 
set output '{{{OUTFILE}}}'
set multiplot
set encoding utf8

set bmargin 8

set xtics
set ytics
unset ztics
unset border
unset key 

set palette defined (\
 0.00 '#0654a2',  \
 6.25 '#0854c2', \
12.50 '#1464d2',  \
18.75 '#1e6eeb', \
25.00 '#2882f0' , \
31.25 '#3c96f5',  \
37.50 '#50a5f5',  \
43.75 '#78b9fa',  \
50.00 '#96d2fa' , \
56.25 '#b4f0fa',  \
62.50 '#e1ffff',  \
68.75 '#fffaaa',  \
75.00 '#ffc03c' , \
81.25 '#ff6000',  \
87.50 '#e11400',  \
93.75 '#a50000', \
100.00 '#950000'\
)
####################################
set xrange[{{{X0}}}:{{{X1}}}] 
set yrange[{{{Y0}}}:{{{Y1}}}] 
set cbrange[{{{VAL0}}}:{{{VAL1}}}]

plot  "{{{INFILE}}}" using 1:2:3 with image , '/root/ncc-fy4-project/extras/world_50m.txt' u 1:2 with lines lt 1 lc 'grey'   , '/root/ncc-fy4-project/extras/china2.txt' u 1:2  with lines lt 1 lc '#ccbbdd'  


#title lables
set label 1 '' at screen 0.5,0.08 center font '/root/ncc-fy4-project/extras/libsans-reg.ttf,16'  
set label 2 "Produced {{{MAKETIME}}}" at screen 0.5,0.01 center font '/root/ncc-fy4-project/extras/libsans-reg.ttf,13' 

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

#draw jhht snow days count 0.5 5.5 
#2017-12-16 wf
reset
#heatmap
set terminal png noenhanced size 1000,797 font '/QHZX_DATA/extras/libsans-reg.ttf'
set output "sample-params.txt.png"
set encoding utf8
set multiplot
#title lables
set label 1 "FY4A SNC" at screen 0.02, screen 0.96 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left  
set label 2 "Date from 2017-09-01 to 2017-09-05; Geo:[50,130,50,0]." at screen 0.02, screen 0.93  font '/QHZX_DATA/extras/libsans-reg.ttf,12' left
set label 3 "Produced:2017-12-17 15:11:28; CFC:5."  at screen 0.02, screen 0.90  font '/QHZX_DATA/extras/libsans-reg.ttf,12' left

#plot1
set origin 0,0
set size  0.9,0.9
unset key 
unset xtics
unset ytics
set cbtics scale 0

set cbrange[0.5:5.5]
set xrange[-2.83489e+06:4.23609e+06]
set yrange[762531:6.39652e+06]
set x2range[-2.83489e+06:4.23609e+06]

set palette defined (  \
0 '#2C7BB6', 1 '#2C7BB6',\
1 '#64A5CD', 2 '#64A5CD',\
2 '#9DCFE4', 3 '#9DCFE4',\
3 '#C7E6DB', 4 '#C7E6DB',\
4 '#EDF7C9', 5 '#EDF7C9',\
5 '#FFEDAA', 6 '#FFEDAA',\
6 '#FEC980', 7 '#FEC980',\
7 '#F99E59', 8 '#F99E59',\
8 '#E85B3A', 9 '#E85B3A',\
9 '#D7191C',10 '#D7191C' \
)
set colorbox user origin 0.9,0.05 size 0.05,0.8
plot  "sample-params.txt.png.txt" using 1:2:3 with image, \
'/QHZX_DATA/extras/world_50m-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china2-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-asia-albers.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/cn-river-asia-albers.txt' u 1:2 with lines lt 1 lc '#0E35A6',\
'/QHZX_DATA/extras/grid-asia-albers.txt' u 1:2 with lines lt 1 lc '#cccccc' ,\
'/QHZX_DATA/extras/grid-oyalbers-labels20.txt' u 1:2:3 with labels textcolor '#888888' 



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

#draw ndvi anomalies juping
#2017-12-18 wf
reset
#heatmap
set terminal png noenhanced size 2000,1390 font '/QHZX_DATA/extras/libsans-reg.ttf,20' #new
set output "{{{OUTFILE}}}"
set encoding utf8
set multiplot

#plot1
set origin 0.03,0.05 #new
set size  0.87,0.85
unset key 
unset xtics 
unset ytics 
unset x2tics

#longitude labels
set label 11 "80\260E" at screen 0.08,screen 0.05 center textcolor "#888888"
set label 12 "90\260E" at screen 0.23,screen 0.05 center textcolor "#888888"
set label 13 "100\260E" at screen 0.39,screen 0.05 center textcolor "#888888"
set label 14 "110\260E" at screen 0.55,screen 0.05 center textcolor "#888888"
set label 15 "120\260E" at screen 0.70,screen 0.05 center textcolor "#888888"
set label 16 "130\260E" at screen 0.87,screen 0.05 center textcolor "#888888"

#latitude labels
set label 30 "40\260N" at screen 0.03,screen 0.65 center textcolor "#888888"
set label 31 "30\260N" at screen 0.03,screen 0.42 center textcolor "#888888"
set label 32 "20\260N" at screen 0.03,screen 0.2 center textcolor "#888888"

set format x "%.0f"
set format y "%.0f"

set xrange[-3000000:3000000]
set yrange[1800000:6000000]
set cbrange[-0.5:0.5]

set cbtics (  -0.5,-0.4,-0.3,-0.2,-0.1,0,0.1,0.2,0.3,0.4,0.5 )#new

set colorbox user origin screen 0.90,screen 0.1 size screen 0.05,screen 0.75 #new

set palette defined ( \
0 '#713e3a' , 0.05 '#713e3a' , \
0.05 '#835632' , 0.1 '#835632' , \
0.1 '#956e29' , 0.15 '#956e29' , \
0.15 '#a78721' , 0.2 '#a78721' , \
0.2 '#b99f19' , 0.25 '#b99f19' , \
0.25 '#cbb711' , 0.3 '#cbb711' , \
0.3 '#ddbf09' , 0.35 '#ddbf09' , \
0.35 '#ddcc0e' , 0.4 '#ddcc0e' , \
0.4 '#eedd0e' , 0.45 '#eedd0e' , \
0.45 '#eeee0e' , 0.5 '#eeee0e' , \
0.5 '#98E300' , 0.55 '#98E300' , \
0.55 '#3EC90B' , 0.6 '#3EC90B' , \
0.6 '#05B514' , 0.65 '#05B514' , \
0.65 '#03A017' , 0.7 '#03A017' , \
0.7 '#018B1A' , 0.75 '#018B1A' , \
0.75 '#00751D' , 0.8 '#00751D' , \
0.8 '#01611F' , 0.85 '#01611F' , \
0.85 '#015120' , 0.9 '#015120' , \
0.9 '#013D1D' , 0.95 '#013D1D' , \
0.95 '#001d14' , 1 '#001d14'  \
)

set label 1 "{{{PNAME}}}" at screen 0.05, screen 0.94 font '/QHZX_DATA/extras/libsans-reg.ttf,30' left #new
set label 2 "From {{{YMD}}} to {{{YMD1}}}" at screen 0.05, screen 0.90 font '/QHZX_DATA/extras/libsans-reg.ttf,30' left    #new


set object 1 rect from graph 0.86,0.0 to graph 1.0,0.3
set object 1 rect fc rgb 'white' fillstyle solid  0.0 noborder front

#new empty box.
set object 2 rect from screen 0.90 ,screen 0.05 to screen 0.95,screen 0.08 fc rgb 'white' fillstyle solid  0.0  front
set label 3 "Empty" at screen 0.955,screen 0.06 left 


plot "{{{INFILE}}}" using 1:2:3 with image ,\
'/QHZX_DATA/extras/world_110m-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-albers-final.txt' u 1:2 with lines lt 1 lc '#222222' lw 1 ,\
'/QHZX_DATA/extras/river-albers-final.txt' u 1:2 with lines lt 1 lc '#0E35BB' lw 2,\
'/QHZX_DATA/extras/grid-albers-final.txt' u 1:2 with lines lt 1 lc '#cccccc'


#nan hai
unset key 
unset xtics 
unset ytics 
set size 0.165,0.28 #new
set origin 0.735,0.05 #new
set xrange[40000:1800000]
set yrange[100000:2500000]
unset colorbox
unset object 1
unset object 2
plot \
'/QHZX_DATA/extras/world_110m-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/china-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/sheng-albers-final.txt' u 1:2 with lines lt 1 lc '#ccbbdd' ,\
'/QHZX_DATA/extras/river-albers-final.txt' u 1:2 with lines lt 1 lc '#0E35A6' ,\
'/QHZX_DATA/extras/grid-albers-final.txt' u 1:2 with lines lt 1 lc '#cccccc'


#logos
set origin 0.75, 0.86 #new
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

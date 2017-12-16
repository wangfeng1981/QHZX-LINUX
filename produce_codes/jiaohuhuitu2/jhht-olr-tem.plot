#draw sst {{{VMIN}}} {{{VMAX}}} 
#2017-12-07 wf
reset
#heatmap
set terminal png noenhanced size {{{OUTWID}}},{{{OUTHEI}}} font '/QHZX_DATA/extras/libsans-reg.ttf'
set output "{{{OUTFILE}}}"
set encoding utf8
set multiplot
#title lables
set label 1 "{{{DISPLAYNAME}}}" at screen 0.02, screen 0.96 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left  
set label 2 "Date from {{{DATE0}}} to {{{DATE1}}}; Geo:[{{{LEFT}}},{{{RIGHT}}},{{{TOP}}},{{{BOTTOM}}}]." at screen 0.02, screen 0.93 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left
set label 3 "Produced:{{{PTIME}}}; CFC:{{{CFC}}}."  at screen 0.02, screen 0.90 font '/QHZX_DATA/extras/libsans-reg.ttf,12' left

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
set xrange[{{{X0}}}:{{{X1}}}]
set yrange[{{{Y0}}}:{{{Y1}}}]
set x2range[{{{X0}}}:{{{X1}}}]


set cbrange[50:400]
#set cbtics ( "<50" 50, "125" 125 ,"200" 200 ,"275" 275,"350" 350 ,">400 (W/m^2)" 400 )
set hidden3d front
set palette defined (\
 0 '#0854c2', 1 '#0854c2',\
1 '#1e6eeb', 2 '#1e6eeb',\
2 '#2882f0' ,3 '#2882f0' ,\
3 '#3c96f5', 4 '#3c96f5',\
4 '#50a5f5', 5 '#50a5f5',\
5 '#78b9fa', 6 '#78b9fa',\
6 '#96d2fa' ,7 '#96d2fa',\
7 '#b4f0fa', 8 '#b4f0fa',\
8 '#e1ffff', 9 '#e1ffff',\
9 '#fffaaa', 10 '#fffaaa',\
10 '#ffc03c' ,11 '#ffc03c',\
11 '#ff6000', 12 '#ff6000',\
12 '#e11400', 13 '#e11400',\
13 '#a50000', 14 '#a50000'\
)



set colorbox user origin 0.9,0.05 size 0.05,0.8
plot   "{{{INFILE}}}" using 1:2:3 with image \
,'/QHZX_DATA/extras/world_50m.txt' u 1:2 with lines lt 1 lc 'grey' \
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
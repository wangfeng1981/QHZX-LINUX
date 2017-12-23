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


set cbrange[{{{VMIN}}}:{{{VMAX}}}]

set hidden3d front
set palette defined (  \
 6.25 '#4d96b488',  \
12.50 '#6fb3ae88',   \
18.75 '#91cba9',   \
25.00 '#b1e09f',  \
31.25 '#c7e98b',   \
37.50 '#def277',  \
43.75 '#f4fb63',  \
50.00 '#fff55a',   \
56.25 '#ffdf5c',   \
62.50 '#fec95e',   \
68.75 '#feb460',  \
75.00 '#f69053',   \
81.25 '#ec6841',  \
87.50 '#e2402e',   \
93.75 '#d7191c' \
)


set colorbox user origin 0.85,0.05 size 0.05,0.8
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
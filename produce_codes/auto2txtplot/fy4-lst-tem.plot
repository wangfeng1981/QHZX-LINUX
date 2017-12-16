#fy4 长波辐射圆盘绘图sst plot 20171206
reset
set terminal png noenhanced size 800,1000 font '/QHZX_DATA/extras/libsans-reg.ttf'
set output '{{{OUTFILE}}}'
set multiplot
set encoding utf8
set mapping spherical
set angles degrees

set parametric
set view 90,195
set isosamples 30
set xyplane at -1
set origin -0.30,-0.2
set size 1.6,1.4
unset xtics
unset ytics
unset ztics
unset border
unset key 



set colorbox horizontal user origin 0.1 , 0.055 size 0.8,.05
set urange[0:360]
set vrange[-90:90]
set cbrange[256:320]
set format cb "%.0fK"
set pal maxcolors 16
set palette defined (  \
 0.00 '#e8e8e888', 6.25 '#e8e8e888', \
 6.25 '#4d96b488', 12.5 '#4d96b488', \
12.50 '#6fb3ae88', 18.75 '#6fb3ae88', \
18.75 '#91cba9', 25 '#91cba9', \
25.00 '#b1e09f', 31.25 '#b1e09f', \
31.25 '#c7e98b', 37.5 '#c7e98b', \
37.50 '#def277', 43.75 '#def277', \
43.75 '#f4fb63', 50 '#f4fb63', \
50.00 '#fff55a', 56.25 '#fff55a', \
56.25 '#ffdf5c', 62.5 '#ffdf5c', \
62.50 '#fec95e', 68.75 '#fec95e', \
68.75 '#feb460', 75 '#feb460', \
75.00 '#f69053', 81.25 '#f69053', \
81.25 '#ec6841', 87.5 '#ec6841', \
87.50 '#e2402e', 93.75 '#e2402e', \
93.75 '#d7191c', 100 '#d7191c'\
)
set cbtics (  'Land' 256 , '<260K' 260, '272K' 272 , '284K' 284,'296K' 296, '308K' 308,'>320K' 320 )

set hidden3d front

r = 1.01
set pm3d interpolate 2,2
####################################
splot '/QHZX_DATA/extras/land_0125half.llv.txt' using 1:2:(1):3 with pm3d , "{{{INFILE}}}" using 1:2:(1):( ($3<260)*260 + ($3>=260)*$3 ) with pm3d, r*cos(v)*cos(u),r*cos(v)*sin(u),r*sin(v) with lines lt 0 lc 'grey' ,'/QHZX_DATA/extras/world_110m.txt' u 1:2:(1.01) with lines lt 1 lc 'grey' , '/QHZX_DATA/extras/china.dat' u 1:2:(1.01) with lines lt 1 lc '#ccbbdd'  



#title lables font should change.
set label 1 "FY4A AGRI LST {{{DATE}}} {{{TTYPE}}} 4Km" at screen 0.5,0.95 center font '/QHZX_DATA/extras/libsans-reg.ttf,16' 
set label 2 "Produced:{{{PTIME}}}" at screen 0.5,0.92 center font '/QHZX_DATA/extras/libsans-reg.ttf,12' 

#logos
unset cbrange
set xrange [-1:1] ;
set yrange [-1:1] ;
unset title
plot '/QHZX_DATA/extras/cmalogo.png' binary filetype=png dx=0.00125 dy=0.001 origin=(0.18,0.48)  with rgbimage  ,'/QHZX_DATA/extras/ncclogo.png' binary filetype=png dx=0.001 dy=0.001 origin=(0.36,0.48)  with rgbimage 


unset multiplot
unset output
#fy4 长波辐射圆盘绘图sst plot 20171206
reset
set terminal png noenhanced size 1200,640 font '/QHZX_DATA/extras/libsans-reg.ttf'
set output "{{{OUTFILE}}}"
set multiplot
set encoding utf8
set pm3d map

set xtics
set ytics
unset ztics
set border
unset key 
set grid
set colorbox vertical user origin 0.88 , 0.1 size 0.03,0.7
set xrange[30:180]
set x2range[30:180]
set yrange[-20:60]
set cbrange[25:400]
set cbtics ("Land" 25,\
"<50" 52,\
"125" 125 ,\
"200" 200 ,\
"275" 275,\
"350" 350 ,\
">400(W/m^2)" 400 \
)


set margins 0,0,0,0
set origin -0.03,0
set size 1.0,1.0

set xtics ("20\260E" 20, "40\260E" 40, "60\260E" 60, "80\260E" 80, "100\260E" 100,"120\260E" 120, "140\260E" 140,"160\260E" 160,"180\260" 180 )
set ytics ("20\260S" -20, "0\260" 0, "20\260N" 20,"40\260N" 40, "60\260N" 60,)


set pal maxcolors 15
set palette defined (\
 0.00 '#f1f1f1', 6.67 '#f1f1f1',\
 6.67 '#0854c2', 13.33 '#0854c2',\
13.33 '#1e6eeb', 20.00 '#1e6eeb',\
20.00 '#2882f0' ,26.67 '#2882f0' ,\
26.67 '#3c96f5', 33.33 '#3c96f5',\
33.33 '#50a5f5', 40.00 '#50a5f5',\
40.00 '#78b9fa', 46.67 '#78b9fa',\
46.67 '#96d2fa' ,53.33 '#96d2fa',\
53.33 '#b4f0fa', 60.00 '#b4f0fa',\
60.00 '#e1ffff', 66.67 '#e1ffff',\
66.67 '#fffaaa', 73.33 '#fffaaa',\
73.33 '#ffc03c' ,80.00 '#ffc03c',\
80.00 '#ff6000', 86.67 '#ff6000',\
86.67 '#e11400', 93.33 '#e11400',\
93.33 '#a50000', 100.00 '#a50000'\
)

set pm3d interpolate 2,2
####################################
splot '/QHZX_DATA/extras/land_0125half.llv.txt' using 1:2:(1):3 with pm3d ,"{{{INFILE}}}" using 1:2:(1):( ($3<50)*50 + ($3>=50)*$3 ) with pm3d, '/QHZX_DATA/extras/world_110m.txt' u 1:2:(2) with lines lt 1 lc 'grey' , '/QHZX_DATA/extras/china.dat' u 1:2:(1.1) with lines lt 1 lc '#ccbbdd'  



#title lables font should change.
set label 1 "FY4A AGRI OLR {{{DATE}}} {{{TTYPE}}} 4Km Asian Region" at screen 0.5,0.95 center  font '/QHZX_DATA/extras/libsans-reg.ttf,16' 
set label 2 "Produced:{{{PTIME}}}" at screen 0.5,0.92 center font '/QHZX_DATA/extras/libsans-reg.ttf,10' 

#logos
unset border
unset cbrange
unset xtics
unset ytics
set origin 0.84,0.88
set size 0.12,0.12
set xrange [0:128] ;
set yrange [0:64] ;
unset title
plot '/QHZX_DATA/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage  ,'/QHZX_DATA/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0)  with rgbimage 


unset multiplot
unset output


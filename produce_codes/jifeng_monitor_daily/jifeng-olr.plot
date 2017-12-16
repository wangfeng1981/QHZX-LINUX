# 20171208 季风OLR和风场叠加绘图
reset
set terminal png noenhanced size 1500,700 font "/QHZX_DATA/extras/libsans-reg.ttf,16"
set output '{{{OUTFILE}}}'
set encoding utf8
set multiplot
#title lables
set label 1 'FY4A OLR With NCEP 850hPa Wind {{{YMD}}} {{{SJCD}}}'  at screen 0.5, screen 0.96 font '/QHZX_DATA/extras/libsans-reg.ttf,22' center 


unset key 
set xtics 
set ytics 

unset ztics
set format x "%.0f\260"
set format y "%.0f\260"
set format x2 "%.0f\260"
show grid
set grid front 
set xrange[30:180]
set x2range[30:180]
set yrange[-20:50]
set cbrange[25:400]


set pal maxcolors 15
set palette defined (\
 0.00 '#0834a2', 6.67 '#0834a2',\
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

set zrange[0:2] ;
set view 0,0
set origin 0.05,0.1
set size 0.85,0.85
set colorbox user origin 0.90,0.24 size 0.03,0.5

################################ plot legend
set style arrow 1 filled lc 'black' lw 2  
set label 3 'NCEP 850hPa Wind' at graph 0.1 , graph -0.1 left font '/QHZX_DATA/extras/libsans-reg.ttf,16'
set arrow 1 from graph 0.28, graph -0.1 to graph 0.3135,  graph -0.1 arrowstyle 1
set label 4 '10.0m/s' at graph 0.34 , graph -0.1 left font '/QHZX_DATA/extras/libsans-reg.ttf,16'

set ytics ( "40\260S" -40, "20\260S" -20, "0\260" 0, "20\260N" 20, "40\260N" 40, "60\260N" 60 )
set xtics ( "20\260E" 20, "40\260E" 40, "60\260E" 60, "80\260E" 80, "100\260E" 100, "120\260E" 120 , "140\140E" 140 , "160\260E" 160 , "180\260" 180 )

plot  '{{{INFILE1}}}' using 1:2:3 with image ,'/QHZX_DATA/extras/world_110m.txt' u 1:2 with lines  lt 1 lc '#eaea55' ,  '/QHZX_DATA/extras/china2.txt' u 1:2  with lines lt 1 lc '#55ff55' ,'{{{INFILE2}}}' u 1:2:($3/2):($4/2) w vec arrowstyle 1


#logos
set origin 0.72,-0.02
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

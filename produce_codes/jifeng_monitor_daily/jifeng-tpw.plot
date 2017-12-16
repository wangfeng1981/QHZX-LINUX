# 2017-11-10 季风水汽和风场叠加绘图 linux
reset
set terminal png noenhanced size 1500,700 font "/QHZX_DATA/extras/libsans-reg.ttf,16"
set output '{{{OUTFILE}}}'
set encoding utf8
set multiplot
#title lables
set label 1 'FY4A TPW With NCEP 850hPa Wind {{{YMD}}} {{{SJCD}}}'  at screen 0.5, screen 0.96 font '/QHZX_DATA/extras/libsans-reg.ttf,22' center 


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
set cbrange[0:10]
#set format cb "%.01fcm"
set pal maxcolors 10
set palette defined (  \
 0 '#5C004B',   1 '#5C004B', \
 1 '#B6369A',   2 '#B6369A', \
 2 '#BE7B9F',   3 '#BE7B9F', \
 3 '#4BD336',   4 '#4BD336', \
 4 '#1AE948',   5 '#1AE948', \
 5 '#2ABCD9',   6 '#2ABCD9', \
 6 '#2796E2',   7 '#2796E2', \
 7 '#1D74BB',   8 '#1D74BB', \
 8 '#125293',   9 '#125293' ,\
 9 '#08306B',   10 '#08306B' \
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


set cbtics ( 0, 2 , 4 , 6 , 8 , 10 )
set ytics ( "40\260S" -40, "20\260S" -20, "0\260" 0, "20\260N" 20, "40\260N" 40, "60\260N" 60 )
set xtics ( "20\260E" 20, "40\260E" 40, "60\260E" 60, "80\260E" 80, "100\260E" 100, "120\260E" 120 , "140\140E" 140 , "160\260E" 160 , "180\260" 180 )


plot  '{{{INFILE1}}}' using 1:2:3 with image ,'/root/ncc-fy4-project/extras/world_110m.txt' u 1:2 with lines  lt 1 lc '#eaea55' ,  '/root/ncc-fy4-project/extras/china2.txt' u 1:2  with lines lt 1 lc '#55ff55' ,'{{{INFILE2}}}' u 1:2:($3/2):($4/2) w vec arrowstyle 1


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
plot '/root/ncc-fy4-project/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage , '/root/ncc-fy4-project/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0.5)  with rgbimage 
unset multiplot
unset output

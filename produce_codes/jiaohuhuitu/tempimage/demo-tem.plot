#fy4海表温度交互绘图sst plot
reset
set terminal png noenhanced size 1000,1000 font '/root/ncc-fy4-project/extras/libsans-reg.ttf,16' 
set output "{{{OUTFILE}}}"
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

plot  "{{{INFILE}}}" using 1:2:3 with image , '/root/ncc-fy4-project/extras/world_50m.txt' u 1:2 with filledcurves lt 1 lc 'grey'   , '/root/ncc-fy4-project/extras/china2.txt' u 1:2  with lines lt 1 lc '#ccbbdd'  





unset multiplot
unset output

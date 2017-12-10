#孟加拉湾海温剖面图 x轴为时间，y轴为90E的海温和风场 2017-11-13

set term png size 1200,900 font "/root/ncc-fy4-project/extras/libsans-reg.ttf,16"
set output '{{{OUTFILE}}}'

set multiplot 

#set xdata time
#set timefmt "%Y-%m-%d"
#set format x "%m"

set view map
set origin -0.05,0.47
set size 1.05,0.55
set cbrange[22:30]

# palette
set palette defined ( 0 '#D73027',1 '#D73027',\
    	    	      1 '#F46D43',2 '#F46D43',\
		      2 '#FDAE61',3 '#FDAE61',\
		      3 '#FEE090',4 '#FEE090',\
		      4 '#E0F3F8',5 '#E0F3F8',\
		      5 '#ABD9E9',6 '#ABD9E9',\
		      6 '#74ADD1',7 '#74ADD1',\
		      7 '#4575B4',8 '#4575B4' )

set palette negative

set contour both
set cntrparam levels discrete 22,23,24,25,26,27,28,29,30,31,32
set style textbox opaque margins  0.5,0.5  noborder 
set cntrlabel  format '%8.1f' font ',7' start 5 interval 2000
set hidden3d back offset 1 trianglepattern 3 undefined 1 altdiagonal bentover
set zlabel  offset character 1, 0, 0 font "" textcolor lt -1 norotate

set linetype 1  
set linetype 2  
set linetype 3 lc rgb "black"
set linetype 4 lc rgb "black"
set linetype 5 lc rgb "black"
set linetype 6 lc rgb "black"
set linetype 7 lc rgb "black"
set linetype 8 lc rgb "black"
set linetype 9 lc rgb "black"
set linetype 10 lc rgb "black"
set linetype 11 lc rgb "black"
set linetype 12 lc rgb "black"
set linetype 13 lc rgb "black"
set linetype 14 lc rgb "black"
set linetype 15 lc rgb "black"
set linetype 16 lc rgb "black"
set linetype 17 lc rgb "black"
set linetype 18 lc rgb "black"

unset key

set xdata time
set timefmt "%Y-%m-%d"
set format x "%m"

set xrange["{{{DATE0}}}":"{{{DATE1}}}"]
#set xrange["2016-11-30":"2017-11-30"]
set yrange[-5:20]

splot '{{{INFILE1}}}' using 1:2:(0):3 with pm3d , '' u 1:2:3 w l  ,'' u 1:2:3  w labels boxed 


###################### plot downside

set xdata time
set timefmt "%Y-%m-%d"
set format x "%m"

set xrange["{{{DATE0}}}":"{{{DATE1}}}"]
set yrange[-5:20]

set origin 0.023,0.1
set size 0.884,0.43
plot '{{{INFILE2}}}' using 1:2:($3*100000):($4/8) with vec

###################### logos

reset
set label 1 'Bay of Bengal SST Monitor from {{{TITLE1}}} to {{{TITLE2}}}'  at screen 0.5, screen 0.96 font '/root/ncc-fy4-project/extras/libsans-reg.ttf,22' center

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
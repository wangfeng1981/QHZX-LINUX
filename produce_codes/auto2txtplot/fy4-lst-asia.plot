#20E-160E 0-70N
reset
set terminal png noenhanced size 1200,600 font '/QHZX_DATA/extras/libsans-reg.ttf'
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
set xrange[20:160]
set x2range[20:160]
set yrange[0:70]
set cbrange[256:320]
set cbtics (  'Land' 256 , '<260K' 260, '272K' 272 , '284K' 284,'296K' 296, '308K' 308,'>320K' 320 )



set margins 0,0,0,0
set origin -0.03,0
set size 1.0,1.0

set xtics ("20\260E" 20, "40\260E" 40, "60\260E" 60, "80\260E" 80, "100\260E" 100,"120\260E" 120, "140\260E" 140,"160\260E" 160,"180\260" 180 )
set ytics ("20\260S" -20, "0\260" 0, "20\260N" 20,"40\260N" 40, "60\260N" 60,)


set pal maxcolors 15
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

set pm3d interpolate 2,2
####################################
splot '/QHZX_DATA/extras/land_0125half.llv.txt' using 1:2:(1):3 with pm3d ,"{{{INFILE}}}" using 1:2:(1):( ($3<260)*260 + ($3>=260)*$3 ) with pm3d, '/QHZX_DATA/extras/world_110m.txt' u 1:2:(2) with lines lt 1 lc 'grey' , '/QHZX_DATA/extras/china2.txt' u 1:2:(1.1) with lines lt 1 lc '#ccbbdd'  



#title lables font should change.
set label 1 "FY4A AGRI LST {{{DATE}}} {{{TTYPE}}} 4Km Asian Region" at screen 0.5,0.95 center  font '/QHZX_DATA/extras/libsans-reg.ttf,16' 
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


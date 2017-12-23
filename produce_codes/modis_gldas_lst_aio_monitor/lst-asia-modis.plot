#lst plot global 20171218
reset
set terminal png noenhanced size 1600,1800 font "/QHZX_DATA/extras/libsans-reg.ttf"
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

set xrange[20:160]
set x2range[20:160]
set yrange[0:70]
set cbrange[255:335]
set cbtics (  "Land" 255 , "<265K" 265,"270K" 270,"280K" 280, "290K" 290 , "300K" 300,"310K" 310, "320K" 320,">330K" 330 ) #new

set margins 0,0,0,0
set origin -0.03,0
set size 1,0.5

set xtics ("180\260" -180 , "160\260W" -160 , "140\260W" -140 , "120\260W" -120 , "100\260W" -100 , "80\260W" -80 , "60\260W" -60 , "40\260W" -40 , "20\260W" -20 , "0\260" 0 , "20\260E" 20 , "40\260E" 40 , "60\260E" 60 , "80\260E" 80 , "100\260E" 100 , "120\260E" 120 , "140\260E" 140 , "160\260E" 160 , "180\260" 180)


set ytics (  "80\260S" -80 , "60\260S" -60 , "40\260S" -40 , "20\260S" -20 , "0\260" 0 , "20\260N" 20 , "40\260N" 40 , "60\260N" 60 , "80\260N" 80   ) #new


set palette defined (  \
 0.00 "#e8e8e888", 6.25 "#e8e8e888", \
 6.25 "#4d96b488", 12.5 "#4d96b488", \
12.50 "#6fb3ae88", 18.75 "#6fb3ae88", \
18.75 "#91cba9", 25 "#91cba9", \
25.00 "#b1e09f", 31.25 "#b1e09f", \
31.25 "#c7e98b", 37.5 "#c7e98b", \
37.50 "#def277", 43.75 "#def277", \
43.75 "#f4fb63", 50 "#f4fb63", \
50.00 "#fff55a", 56.25 "#fff55a", \
56.25 "#ffdf5c", 62.5 "#ffdf5c", \
62.50 "#fec95e", 68.75 "#fec95e", \
68.75 "#feb460", 75 "#feb460", \
75.00 "#f69053", 81.25 "#f69053", \
81.25 "#ec6841", 87.5 "#ec6841", \
87.50 "#e2402e", 93.75 "#e2402e", \
93.75 "#d7191c", 100 "#d7191c"\
)


############ Night
set colorbox vertical user origin 0.91 , 0.07 size 0.03,0.35
splot  "/QHZX_DATA/extras/global_land_raster_025.txt" using 1:2:(1):3 with pm3d,"{{{INFILE2}}}" using 1:2:(1.1):( ($3<260)*260 + ($3>=260)*$3 ) with pm3d  ,"/QHZX_DATA/extras/world_110m.txt" u 1:2:(1)  with lines lt 1 lc "grey", "/QHZX_DATA/extras/china2.txt" u 1:2:(1.2)  with lines lt 1 lc "#ccbbdd"  


######## Daytime
set origin -0.03,0.48
set size 1,0.5
set colorbox vertical user origin 0.91 , 0.55 size 0.03,0.35
splot  "/QHZX_DATA/extras/global_land_raster_025.txt" using 1:2:(1):3 with pm3d,"{{{INFILE}}}" using 1:2:(1.1):( ($3<260)*260 + ($3>=260)*$3 ) with pm3d  ,"/QHZX_DATA/extras/world_110m.txt" u 1:2:(1)  with lines lt 1 lc "grey", "/QHZX_DATA/extras/china2.txt" u 1:2:(1.2)  with lines lt 1 lc "#ccbbdd" 



#title lables font should change.
set label 1 "{{{PNAME}}}" at screen 0.05,0.95 left  font "/QHZX_DATA/extras/libsans-reg.ttf,24" 
set label 2 "{{{YMD}}}" at screen 0.05,0.93 left font "/QHZX_DATA/extras/libsans-reg.ttf,24" 
set label 3 "{{{PNAME2}}}" at screen 0.05,0.47 left  font "/QHZX_DATA/extras/libsans-reg.ttf,24" 
set label 4 "{{{YMD2}}}" at screen 0.05,0.45 left font "/QHZX_DATA/extras/libsans-reg.ttf,24" 




#logos
unset border
unset cbrange
unset xtics
unset ytics
set origin 0.79,0.92
set size 0.14,0.06
set xrange [0:128] ;
set yrange [0:64] ;
unset title
plot '/QHZX_DATA/extras/cmalogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(0,0)  with rgbimage  ,'/QHZX_DATA/extras/ncclogo.png' binary filetype=png dx=0.5 dy=0.5 origin=(64,0)  with rgbimage 


unset multiplot
unset output

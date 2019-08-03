set term x11 0
if(i==150)i=0;exit
set title "Waldanordnung: ->  conf".i.".dat  <-"
set xrange [0:101]
set yrange [0:101] 
set xlabel "X-Position"
set ylabel "Y-Position"
set point 2

plot [][] "conf".i.".dat"  u ($1+1):($2+1):($3+1) pt 5  lc variable t "" 

i=i+1

#reset
#set title "Waldanordnung"
#set term x11 1
#set xrange [0:i]
#plot "Baumanzahl.dat" using 1:2

pause 0.5

reread

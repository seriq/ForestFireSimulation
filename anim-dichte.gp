set term x11 1

reset
set title "zeitliche Dichteentwicklung"
set xlabel "Schritt"
set ylabel "Baumdichte"
#set xrange [-0.1:9]
#set yrange [-0.1:10] 
f(x)=a*x+b
#fit f(x) "dichte.dat" using ($1<4?log($1):0/0):($2==0?0/0:(log($2))) via a,b
#plot "dichte.dat" using (log($1)):($2==0?0/0:(log($2))) notitle,f(x) notitle
plot "dichte.dat" using 1:2 notitle
pause 0



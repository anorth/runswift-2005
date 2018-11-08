p(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 25
fit p(x) "rawGrabTurnCW.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawGrabTurnCW.dat" using 1:($2*pg/62.5) via m,d
#set xrange [-40:0]
#set yrange [-45:0]
plot "rawGrabTurnCW.dat" using 1:($2*pg/62.5) with linespoints, p(x), l(x)
pause -1

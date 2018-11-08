p(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 25
fit p(x) "rawGrabTurnCCW.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawGrabTurnCCW.dat" using 1:($2*pg/62.5) via m,d
#set xrange [0:40]
#set yrange [0:60]
plot "rawGrabTurnCCW.dat" using 1:($2*pg/62.5) with linespoints, p(x), l(x)
pause -1

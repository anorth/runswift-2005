p(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 23
set xrange[0:55]
set yrange[0:90]
fit p(x) "rawTurnCCW.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawTurnCCW.dat" using 1:($2*pg/62.5) via m,d
plot "rawTurnCCW.dat" using 1:($2*pg/62.5) with linespoints, p(x), l(x)
pause -1

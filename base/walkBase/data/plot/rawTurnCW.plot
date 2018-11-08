p(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 22
fit p(x) "rawTurnCW.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawTurnCW.dat" using 1:($2*pg/62.5) via m,d
#set xrange [-55:0]
#set yrange [-70:0]
plot "rawTurnCW.dat" using 1:($2*pg/62.5) with linespoints, p(x), l(x)
pause -1

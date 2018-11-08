p(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 22
fit p(x) "rawLeft.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawLeft.dat" using 1:($2*pg/62.5) via m,d
plot "rawLeft.dat" using 1:($2*pg/62.5) with linespoints, p(x), l(x)
pause -1

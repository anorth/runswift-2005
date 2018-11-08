q(x) = a*x*x + b*x + c
l(x) = m*x + d
fit q(x) "rawLeft.dat" using 1:2 via a,b,c
fit l(x) "rawLeft.dat" using 1:2 via m,d
set xrange [0:6]
set yrange [0:10]
plot "rawLeft.dat" with linespoints, q(x), l(x)
pause -1

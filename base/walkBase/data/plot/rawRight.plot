q(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 22
fit q(x) "rawRight.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawRight.dat" using 1:($2*pg/62.5) via m,d
#set xrange [-6:0]
#set yrange [-8:0]
plot "rawRight.dat" using 1:($2*pg/62.5) with linespoints, q(x), l(x)
pause -1

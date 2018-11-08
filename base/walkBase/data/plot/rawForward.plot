p(x) = a*x*x + b*x + c
l(x) = m*x + d
pg = 22
fit p(x) "rawForward.dat" using 1:($2*pg/62.5) via a,b,c
fit l(x) "rawForward.dat" using 1:($2*pg/62.5) via m,d
#set xrange [0:9]
#set yrange [0:18]
plot "rawForward.dat" using 1:($2*pg/62.5) with linespoints, p(x), l(x)
pause -1

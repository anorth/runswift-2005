set xlabel "Left Command"
set ylabel "Turn Command"
set zlabel "Turn Actual"
set xrange [0:14]
set yrange [0:90]
set zrange [0:90]
l(x) = x
# lower plane equation
# obtained from points (12 10 10), (12 88, 43.5), (3, 88, 71)
p(x,y) =  (1430*x  - 201*y - 19830) / -468
splot "lftTurn_lftCmd12.dat" using 2:3:6 with linespoints, \
      "lftTurn_lftCmd9.dat" using 2:3:6 with linespoints, \
      "lftTurn_lftCmd6.dat" using 2:3:6 with linespoints, \
      "lftTurn_lftCmd3.dat" using 2:3:6 with linespoints, l(y), p(x,y)
pause -1

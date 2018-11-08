set xlabel "Left Command"
set ylabel "Turn Command"
set zlabel "Left Actual"
set xrange [0:14]
set yrange [0:90]
set zrange [0:14]
l(x) = x
# lower plane equation
# obtained from points (12 30 12), (12, 88, 6.9), (3, 88, 1.5)
p(x,y) = (319*x - 45*y + 3786) / 522
splot "lftTurn_lftCmd12.dat" using 2:3:5 with linespoints, \
      "lftTurn_lftCmd9.dat" using 2:3:5 with linespoints, \
      "lftTurn_lftCmd6.dat" using 2:3:5 with linespoints, \
      "lftTurn_lftCmd3.dat" using 2:3:5 with linespoints, l(x), p(x,y)
pause -1

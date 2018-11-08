set xlabel "Forward Command"
set ylabel "Turn Command"
set zlabel "Turn Actual"
set xrange [0:25]
set yrange [0:90]
set zrange [0:90]
l(y) = y

p(x,y) = (145*x - 44*y - 4480) / -100.0
splot "fwdTurn_fwdCmd20.dat" using 1:3:6 with linespoints, \
      "fwdTurn_fwdCmd17.5.dat" using 1:3:6 with linespoints, \
      "fwdTurn_fwdCmd15.dat" using 1:3:6 with linespoints, \
      "fwdTurn_fwdCmd12.5.dat" using 1:3:6 with linespoints, \
      "fwdTurn_fwdCmd10.dat" using 1:3:6 with linespoints, \
      "fwdTurn_trnCmd80.dat" using 1:3:6 with linespoints, \
      "fwdTurn_trnCmd70.dat" using 1:3:6 with linespoints, \
      "fwdTurn_trnCmd60.dat" using 1:3:6 with linespoints, \
      "fwdTurn_trnMatch.dat" using 1:3:6 with linespoints, \
      "fwdTurn_trnCmd50.dat" using 1:3:6 with linespoints, l(y), p(x,y)
pause -1

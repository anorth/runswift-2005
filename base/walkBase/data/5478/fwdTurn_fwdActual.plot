set xlabel "Forward Command"
set ylabel "Turn Command"
set zlabel "Forward Actual"
set xrange [0:25]
set yrange [0:90]
set zrange [0:25]
l(x) = x
# lower plane equation
#using points (6, 80, 6.31) & (20, 30, 19.7) & (20, 80, 13.7)
p(x, y) = (739*x  - 168*y  + 17840)/1400
# fringe line
splot "fwdTurn_fwdCmd20.dat" using 1:3:4 with linespoints, \
      "fwdTurn_fwdCmd17.5.dat" using 1:3:4 with linespoints, \
      "fwdTurn_fwdCmd15.dat" using 1:3:4 with linespoints, \
      "fwdTurn_fwdCmd12.5.dat" using 1:3:4 with linespoints, \
      "fwdTurn_fwdCmd10.dat" using 1:3:4 with linespoints, \
      "fwdTurn_trnCmd80.dat" using 1:3:4 with linespoints, \
      "fwdTurn_trnCmd70.dat" using 1:3:4 with linespoints, \
      "fwdTurn_trnCmd60.dat" using 1:3:4 with linespoints, \
      "fwdTurn_fwdMatch.dat" using 1:3:4 with linespoints, \
      "fwdTurn_trnCmd50.dat" using 1:3:4 with linespoints, l(x), p(x,y)
pause -1

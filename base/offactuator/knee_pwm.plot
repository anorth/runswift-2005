set xlabel "x:frame"
set ylabel "y:knee"
plot "pwm.dat" using 1:4 title "front left" with linespoints, \
     "pwm.dat" using 1:7 title "front right" with linespoints, \
     "pwm.dat" using 1:10 title "rear left" with linespoints, \
     "pwm.dat" using 1:13 title "rear right" with linespoints
pause -1

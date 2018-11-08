set xlabel "x:frame"
set ylabel "y:joint"
plot "pwm.dat" using 1:2 title "front left" with linespoints, \
     "pwm.dat" using 1:5 title "front right" with linespoints, \
     "pwm.dat" using 1:8 title "rear left" with linespoints, \
     "pwm.dat" using 1:11 title "rear right" with linespoints
pause -1

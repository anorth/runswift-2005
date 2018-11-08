set xlabel "x:frame"
set ylabel "y:shoulder"
#set multiplot
plot "pwm.dat" using 1:3 title "front left" with linespoints, \
     "pwm.dat" using 1:6 title "front right" with linespoints, \
     "pwm.dat" using 1:9 title "rear left" with linespoints, \
     "pwm.dat" using 1:12 title "rear right" with linespoints
#set parametric
#plot 224,t
#unset parametric
#unset multiplot
pause -1
